#include <WxDockUI/Internal/TabContainer.h>

#include <unordered_set>

#include <WxDockUI/FrameDockManager.h>
#include <WxDockUI/Internal/PaneContainer.h>




namespace WxDockUI::Internal
{





	TabContainer::TabContainer(
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

		mNotebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &TabContainer::onNotebookChanged,   this);
		mNotebook->Bind(wxEVT_LEFT_DOWN,             &TabContainer::onNotebookLeftDown,  this);
		mNotebook->Bind(wxEVT_MOTION,                &TabContainer::onNotebookMouseMove, this);
		mNotebook->Bind(wxEVT_LEFT_UP,               &TabContainer::onNotebookLeftUp,    this);
		Bind(wxEVT_MOTION,  &TabContainer::onNotebookMouseMove, this);
		Bind(wxEVT_LEFT_UP, &TabContainer::onNotebookLeftUp,    this);
	}





	TabContainer::~TabContainer()
	{
		// Remove all pages from mNotebook, without actually deleting them (they are owned by mPaneTabs):
		mNotebook->Freeze();
		while (mNotebook->GetPageCount() > 0)
		{
			auto * page = mNotebook->GetPage(0);
			mNotebook->RemovePage(0);
			page->Hide();
		}
		mNotebook->Thaw();
	}





	void TabContainer::updateLayout()
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





	void TabContainer::updateTabs()
	{
		// Re-create all tabs in mNotebook:
		mNotebook->Freeze();
		while (mNotebook->GetPageCount() > 0)
		{
			auto * page = mNotebook->GetPage(0);
			mNotebook->RemovePage(0);
			page->Hide();
		}
		std::unordered_set<const Layout::PaneNode *> usedPanes;
		for (const auto & paneNode: mTabNode.panes())
		{
			auto & tab = tabForPane(*paneNode);
			tab.mPanel->Show();
			mNotebook->AddPage(
				tab.mPanel.get(),  // Borrow the pointer, but will NOT delete it
				tab.mCaption,
				false
			);
			usedPanes.insert(paneNode.get());
		}

		// Remove tabs for nodes no longer in the TabNode:
		for (auto itr = mPaneTabs.begin(); itr != mPaneTabs.end();)
		{
			if (usedPanes.find(itr->first) == usedPanes.end())
			{
				// Remove Tab:
				auto paneContents = mFrameDockManager.layoutEngine().maybePaneContainer(*(itr->first));
				if (paneContents != nullptr)
				{
					if (paneContents->GetParent() == itr->second.mPanel.get())
					{
						paneContents->Reparent(mFrameDockManager.frame());
					}
				}
				itr->second.mPanel.reset();
				itr = mPaneTabs.erase(itr);
			}
			else
			{
				++itr;
			}
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




	void TabContainer::onNotebookChanged(wxBookCtrlEvent & aEvent)
	{
		mTabNode.setActiveIndex(mNotebook->GetSelection());
	}





	void TabContainer::onNotebookLeftDown(wxMouseEvent & aEvent)
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





	void TabContainer::onNotebookMouseMove(wxMouseEvent & aEvent)
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





	void TabContainer::onNotebookLeftUp(wxMouseEvent & aEvent)
	{
		if (HasCapture())
		{
			ReleaseMouse();
		}

		mFrameDockManager.paneDragController().endDrag(mDraggedPane, wxGetMousePosition());
		mIsDraggingTab = false;
		mDragTabIndex = wxNOT_FOUND;
		mDraggedPane = nullptr;
		aEvent.Skip();
	}





	void TabContainer::startTabDrag()
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





	TabContainer::Tab & TabContainer::tabForPane(const Layout::PaneNode & aPaneNode)
	{
		auto & tab = mPaneTabs[&aPaneNode];
		if (tab.mPanel != nullptr)
		{
			return tab;
		}

		// First time seeing this pane, create a new panel for it and reparent the pane contents into it:
		tab.mPanel.reset(new wxPanel(mNotebook));
		auto * pc = mFrameDockManager.layoutEngine().ensurePaneContainer(aPaneNode);
		if (pc == nullptr)
		{
			assert(!"Unknown pane container");
			throw std::runtime_error("Unknown pane container");
		}
		const auto * paneInfo = mFrameDockManager.findPaneInfo(aPaneNode.paneId());
		if (paneInfo == nullptr)
		{
			throw std::runtime_error("Unknown paneInfo for a known pane");
		}
		tab.mCaption = paneInfo->caption();
		auto oldSizer = pc->GetContainingSizer();
		if (oldSizer != nullptr)
		{
			oldSizer->Detach(pc);
		}
		if (pc->GetParent() != tab.mPanel.get())
		{
			pc->Reparent(tab.mPanel.get());
		}
		pc->showCaptionBar(false);
		tab.mSizer = new wxBoxSizer(wxVERTICAL);
		tab.mSizer->Add(pc, 1, wxEXPAND);
		tab.mPanel->SetSizer(tab.mSizer);
		return tab;
	}





}  // namespace WxDockUI::Internal
