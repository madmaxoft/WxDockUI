#include <WxDockUI/Internal/PaneDragController.hpp>
#include <WxDockUI/Internal/PaneContainer.hpp>
#include <WxDockUI/Internal/TabContainer.hpp>
#include <WxDockUI/Internal/FrameDockManager.hpp>





namespace WxDockUI::Internal
{





	PaneDragController::PaneDragController(DockSystem & aDockSystem):
		mDockSystem(aDockSystem)
	{
	}





	bool PaneDragController::isDragging() const
	{
		return ((mSourceFrame != nullptr) && (mDraggedPane != nullptr));
	}





	void PaneDragController::beginDrag(
		FrameDockManager & aSourceFrame,
		const Layout::PaneNode * aDraggedPane,
		const wxPoint & aScreenPos
	)
	{
		assert(!isDragging());
		if (aDraggedPane == nullptr)
		{
			return;
		}
		mSourceFrame = &aSourceFrame;
		mDraggedPane = aDraggedPane;
		aSourceFrame.dockOverlay().setCurrentDragNode(mDraggedPane);
		mCurrentTarget.reset();
		updateUI(aScreenPos);
	}





	void PaneDragController::updateDrag(const wxPoint & aScreenPos)
	{
		if (!isDragging())
		{
			assert(mDragGhost == nullptr);
			return;
		}

		// Check if targeting the same frame, if not, update mTargetFrame:
		auto targetFrame = mDockSystem.managedWindowAtScreenPos(aScreenPos);
		if (targetFrame == nullptr)
		{
			// TODO: Indicate floating
			mCurrentTarget.reset();
			updateUI(aScreenPos);
			return;
		}
		if ((mTargetFrame == nullptr) || (targetFrame != mTargetFrame))
		{
			if (mTargetFrame != nullptr)
			{
				mTargetFrame->dockOverlay().showOverlay(false);
			}
			mTargetFrame = targetFrame;
			mTargetFrame->dockOverlay().setCurrentDragNode(mDraggedPane);
			mTargetFrame->dockOverlay().showOverlay(true);
			mTargetFrame->frame()->Raise();
		}

		auto target = mTargetFrame->dockOverlay().hitTest(aScreenPos);
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





	void PaneDragController::endDrag(const wxPoint & aScreenPos)
	{
		if (!isDragging())
		{
			assert(mDragGhost == nullptr);
			return;
		}

		// Finalize any remaining mouse movement:
		updateDrag(aScreenPos);

		if (mCurrentTarget.has_value())
		{
			// Hide the overlay early so that it doesn't cover the debugger:
			mTargetFrame->dockOverlay().showOverlay(false);
			mTargetFrame->performDock(*mSourceFrame, *mDraggedPane, mCurrentTarget.value());
		}

		clearState();
	}





	void PaneDragController::cancelDrag()
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
		if (mTargetFrame != nullptr)
		{
			mTargetFrame->dockOverlay().Hide();
			mTargetFrame->dockOverlay().setCurrentDragNode(nullptr);
			mTargetFrame = nullptr;
		}

		updateUI(wxGetMousePosition());
	}





	void PaneDragController::updateUI(const wxPoint & aScreenPos)
	{
		if (isDragging())
		{
			if (mDragGhost == nullptr)
			{
				mDragGhost = new Internal::DragGhost();
				mDragGhost->Show();
				mDragGhost->Raise();
			}
			moveDragGhostToTarget(aScreenPos);
			if (mTargetFrame != nullptr)
			{
				mTargetFrame->dockOverlay().setCurrentDragNode(mDraggedPane);
				mTargetFrame->dockOverlay().updateMousePosition(aScreenPos);
				mTargetFrame->dockOverlay().showOverlay(true);
			}
		}
		else
		{
			if (mDragGhost != nullptr)
			{
				mDragGhost->Hide();
				delete mDragGhost;
				mDragGhost = nullptr;
			}
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
		assert(mTargetFrame != nullptr);
		wxWindow * wnd;
		auto parentNode = mCurrentTarget.value().mNode->parent();
		if (parentNode->type() == WxDockUI::Layout::NodeType::Tab)
		{
			wnd = mTargetFrame->layoutEngine().ensureTabContainer(parentNode->asTabNode());
		}
		else
		{
			switch (mCurrentTarget.value().mNode->type())
			{
				case Layout::NodeType::Pane:
				{
					auto pane = mCurrentTarget.value().mNode->asPaneNode();
					assert(pane != nullptr);
					wnd = mDockSystem.findPaneWindow(pane->paneId());
					break;
				}
				case Layout::NodeType::Tab:
				{
					wnd = mTargetFrame->layoutEngine().maybeTabContainer(mCurrentTarget.value().mNode->asTabNode());
					break;
				}
				default:
				{
					assert(!"Unhandled node type");
					return;
				}
			}
		}
		if (wnd == nullptr)
		{
			wxLogWarning("PaneDragController::moveDragGhostToTargetPane: Invalid window");
			return;
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
		assert(mTargetFrame != nullptr);
		auto frame = mTargetFrame->frame();
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
