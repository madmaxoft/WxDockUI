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

		mNotebook->Bind(
			wxEVT_NOTEBOOK_PAGE_CHANGED,
			&TabContainerWindow::onNotebookChanged,
			this
		);
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





}  // namespace WxDockUI::Internal
