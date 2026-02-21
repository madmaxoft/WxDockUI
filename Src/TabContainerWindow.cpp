#include <WxDockUI/Internal/TabContainerWindow.h>
#include <WxDockUI/FrameDockManager.h>





namespace WxDockUI::Internal
{





	TabContainerWindow::TabContainerWindow(
		FrameDockManager & aFrameDockManager,
		wxWindow * aParent,
		Layout::TabNode & aTabNode
	):
		Super(aParent, wxID_ANY),
		mFrameDockManager(aFrameDockManager),
		mTabNode(aTabNode)
	{
		mNotebook = new wxNotebook(this, wxID_ANY);

		auto sizer = new wxBoxSizer(wxVERTICAL);
		sizer->Add(mNotebook, 1, wxEXPAND);
		SetSizer(sizer);

		mNotebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &TabContainerWindow::onNotebookChanged,   this);
		mNotebook->Bind(wxEVT_LEFT_DOWN,             &TabContainerWindow::onNotebookLeftDown,  this);
		mNotebook->Bind(wxEVT_MOTION,                &TabContainerWindow::onNotebookMouseMove, this);
		mNotebook->Bind(wxEVT_LEFT_UP,               &TabContainerWindow::onNotebookLeftUp,    this);
		Bind(wxEVT_MOTION,  &TabContainerWindow::onNotebookMouseMove, this);
		Bind(wxEVT_LEFT_UP, &TabContainerWindow::onNotebookLeftUp,    this);
	}





	void TabContainerWindow::updateLayout()
	{
		const int activeIndex = mTabNode.activeIndex();
		const auto & panes = mTabNode.panes();

		if ((activeIndex < 0) || (activeIndex >= static_cast<int>(panes.size())))
		{
			return;
		}

		updateTabs();

		Layout();
	}





	void TabContainerWindow::updateTabs()
	{
		// Re-create all tabs in mNotebook:
		mNotebook->Freeze();
		while (mNotebook->GetPageCount() > 0)
		{
			auto * page = mNotebook->GetPage(0);
			mNotebook->RemovePage(0);
			page->Hide();
		}
		for (const auto & paneNode: mTabNode.panes())
		{
			auto * window = mFrameDockManager.findPaneWindow(paneNode->paneId());
			if (window == nullptr)
			{
				continue;
			}
			const auto * paneInfo = mFrameDockManager.findPaneInfo(paneNode->paneId());
			if (paneInfo == nullptr)
			{
				assert(!"Unknown paneInfo for a known window");
				continue;
			}
			if (window->GetParent() != mNotebook)
			{
				window->Reparent(mNotebook);
			}
			window->Show();
			mNotebook->AddPage(
				window,
				paneInfo->caption(),
				false
			);
		}

		// Update the current page index:
		if (
			(mTabNode.activeIndex() >= 0) &&
			(mTabNode.activeIndex() < static_cast<int>(mNotebook->GetPageCount()))
		)
		{
			mNotebook->SetSelection(mTabNode.activeIndex());
		}

		mNotebook->Thaw();
	}




	void TabContainerWindow::onNotebookChanged(wxBookCtrlEvent & aEvent)
	{
		mTabNode.setActiveIndex(mNotebook->GetSelection());
	}





	void TabContainerWindow::onNotebookLeftDown(wxMouseEvent & aEvent)
	{
		auto pos = aEvent.GetPosition();
		long flags = 0;
		int tabIndex = mNotebook->HitTest(pos, &flags);
		if (tabIndex == wxNOT_FOUND)
		{
			aEvent.Skip();
			return;
		}

		mIsDraggingTab = true;
		mDragTabIndex = tabIndex;
		mDragStartScreenPos = mNotebook->ClientToScreen(pos);

		CaptureMouse();
		aEvent.Skip();
	}





	void TabContainerWindow::onNotebookMouseMove(wxMouseEvent & aEvent)
	{
		if (!aEvent.Dragging())
		{
			aEvent.Skip();
			return;
		}

		if (mDraggedPane != nullptr)
		{
			mFrameDockManager.paneDragController().updateDrag(mDraggedPane, wxGetMousePosition());
		}
		else if (mIsDraggingTab)
		{
			auto currentScreenPos = mNotebook->ClientToScreen(aEvent.GetPosition());
			if (
				(std::abs(currentScreenPos.x - mDragStartScreenPos.x) < DRAG_THRESHOLD_PIXELS) &&
				(std::abs(currentScreenPos.y - mDragStartScreenPos.y) < DRAG_THRESHOLD_PIXELS)
			)
			{
				return;
			}
			startTabDrag();
			mIsDraggingTab = false;
		}

		aEvent.Skip();
	}





	void TabContainerWindow::onNotebookLeftUp(wxMouseEvent & aEvent)
	{
		if (HasCapture())
		{
			ReleaseMouse();
		}

		mFrameDockManager.paneDragController().endDrag(mDraggedPane, wxGetMousePosition());
		mIsDraggingTab = false;
		mDragTabIndex = wxNOT_FOUND;
		aEvent.Skip();
	}





	void TabContainerWindow::startTabDrag()
	{
		if (mDragTabIndex == wxNOT_FOUND)
		{
			return;
		}
		if (mDragTabIndex >= static_cast<int>(mTabNode.panes().size()))
		{
			return;
		}

		mDraggedPane = const_cast<Layout::PaneNode *>(mTabNode.pane(mDragTabIndex));
		if (mDraggedPane == nullptr)
		{
			return;
		}
		mFrameDockManager.paneDragController().beginDrag(mDraggedPane, mDragStartScreenPos);
	}





}  // namespace WxDockUI::Internal
