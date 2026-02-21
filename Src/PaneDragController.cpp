#include <WxDockUI/Internal/PaneDragController.h>
#include <WxDockUI/Internal/PaneContainer.h>
#include <WxDockUI/Internal/TabContainerWindow.h>
#include <WxDockUI/FrameDockManager.h>





namespace WxDockUI::Internal
{





	PaneDragController::PaneDragController(WxDockUI::FrameDockManager & aFrameDockManager):
		mFrameDockManager(aFrameDockManager)
	{
	}





	bool PaneDragController::isDragging() const
	{
		return (mDraggedPane != nullptr);
	}





	void PaneDragController::beginDrag(Layout::PaneNode * aPane, const wxPoint & aScreenPos)
	{
		if (aPane == nullptr)
		{
			return;
		}
		mDraggedPane = aPane;
		mFrameDockManager.dockOverlay().setCurrentDragNode(mDraggedPane);
		mCurrentTarget.reset();
		updateUI(aScreenPos);
	}





	void PaneDragController::updateDrag(Layout::PaneNode * aPane, const wxPoint & aScreenPos)
	{
		if (!isDragging())
		{
			assert(mDragGhost == nullptr);
			return;
		}

		auto target = mFrameDockManager.dockOverlay().hitTest(aScreenPos);
		if (!target.isValid())
		{
			mCurrentTarget.reset();
			updateUI(aScreenPos);
			return;
		}

		// Only update UI if the target has changed:
		if (
			(!mCurrentTarget.has_value()) ||
			(mCurrentTarget.value() != target)
		)
		{
			mCurrentTarget = target;
			updateUI(aScreenPos);
		}
	}





	void PaneDragController::endDrag(Layout::PaneNode * aPane, const wxPoint & aScreenPos)
	{
		if (!isDragging())
		{
			assert(mDragGhost == nullptr);
			return;
		}

		updateDrag(aPane, aScreenPos);

		if (mCurrentTarget.has_value())
		{
			mFrameDockManager.performDock(*mDraggedPane, mCurrentTarget.value());
		}

		clearState();
	}





	void PaneDragController::cancelDrag(Layout::PaneNode * aPane)
	{
		if (!isDragging())
		{
			return;
		}

		clearState();
	}





	void PaneDragController::clearState()
	{
		// Reset the source and target:
		mDraggedPane = nullptr;
		mCurrentTarget.reset();
		mFrameDockManager.dockOverlay().setCurrentDragNode(nullptr);

		updateUI(wxGetMousePosition());
	}





	void PaneDragController::updateUI(const wxPoint & aScreenPos)
	{
		if (isDragging())
		{
			if (mDragGhost == nullptr)
			{
				mDragGhost = new Internal::GhostFrame(mFrameDockManager.frame());
				mDragGhost->Show();
			}
			moveDragGhostToTarget(aScreenPos);
			mFrameDockManager.dockOverlay().updateMousePosition(aScreenPos);
			mFrameDockManager.dockOverlay().showOverlay(true);
		}
		else
		{
			if (mDragGhost != nullptr)
			{
				mDragGhost->Hide();
				delete mDragGhost;
				mDragGhost = nullptr;
			}
			mFrameDockManager.dockOverlay().showOverlay(false);
		}
	}





	void PaneDragController::moveDragGhostToTarget(const wxPoint & aScreenPos)
	{
		if (mDragGhost == nullptr)
		{
			return;
		}
		if (!mCurrentTarget.has_value())
		{
			mDragGhost->SetPosition(aScreenPos);
			return;
		}
		switch (mCurrentTarget.value().mKind)
		{
			case DockTarget::Kind::PaneTab:
			{
				moveDragGhostToTargetPane(10, 10, 90, 90);
				break;
			}
			case DockTarget::Kind::PaneSplitTop:
			{
				moveDragGhostToTargetPane(0, 0, 50, 100);
				break;
			}
			case DockTarget::Kind::PaneSplitLeft:
			{
				moveDragGhostToTargetPane(0, 0, 100, 50);
				break;
			}
			case DockTarget::Kind::PaneSplitBottom:
			{
				moveDragGhostToTargetPane(50, 0, 100, 100);
				break;
			}
			case DockTarget::Kind::PaneSplitRight:
			{
				moveDragGhostToTargetPane(0, 50, 100, 100);
				break;
			}
			case DockTarget::Kind::RootSplitTop:
			{
				moveDragGhostToTargetRoot(0, 0, 20, 100);
				break;
			}
			case DockTarget::Kind::RootSplitLeft:
			{
				moveDragGhostToTargetRoot(0, 0, 100, 20);
				break;
			}
			case DockTarget::Kind::RootSplitBottom:
			{
				moveDragGhostToTargetRoot(80, 0, 100, 100);
				break;
			}
			case DockTarget::Kind::RootSplitRight:
			{
				moveDragGhostToTargetRoot(0, 80, 100, 100);
				break;
			}
			default:
			{
				mDragGhost->SetPosition(aScreenPos);
				break;
			}
		}
	}





	void PaneDragController::moveDragGhostToTargetPane(int aTopPercent, int aLeftPercent, int aBottomPercent, int aRightPercent)
	{
		auto pane = mCurrentTarget.value().mPane;
		wxWindow * wnd;
		if (pane->parent()->type() == WxDockUI::Layout::NodeType::Tab)
		{
			wnd = mFrameDockManager.layoutEngine().tabContainerWindow(pane->parent()->asTabNode());
		}
		else
		{
			wnd = mFrameDockManager.findPaneWindow(*pane);
		}
		auto paneRect = wnd->GetScreenRect();
		int t = paneRect.y + paneRect.height * aTopPercent    / 100;
		int l = paneRect.x + paneRect.width  * aLeftPercent   / 100;
		int b = paneRect.y + paneRect.height * aBottomPercent / 100;
		int r = paneRect.x + paneRect.width  * aRightPercent  / 100;
		mDragGhost->SetSize(wxSize(r - l, b - t));
		mDragGhost->Update();
		mDragGhost->SetSize(wxRect(l, t, r - l, b - t));
	}





	void PaneDragController::moveDragGhostToTargetRoot(int aTopPercent, int aLeftPercent, int aBottomPercent, int aRightPercent)
	{
		auto frame = mFrameDockManager.frame();
		wxRect clientRect = frame->GetClientRect();
		wxPoint clientTopLeftScreen = frame->ClientToScreen(clientRect.GetTopLeft());
		wxRect clientScreenRect(
			clientTopLeftScreen,
			clientRect.GetSize()
		);
		int t = clientScreenRect.y + clientScreenRect.height * aTopPercent    / 100;
		int l = clientScreenRect.x + clientScreenRect.width  * aLeftPercent   / 100;
		int b = clientScreenRect.y + clientScreenRect.height * aBottomPercent / 100;
		int r = clientScreenRect.x + clientScreenRect.width  * aRightPercent  / 100;
		mDragGhost->SetSize(wxSize(r - l, b - t));
		mDragGhost->Update();
		mDragGhost->SetSize(wxRect(l, t, r - l, b - t));
	}





}  // namespace WxDockUI::Internal
