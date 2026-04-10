#include <WxDockUI/Internal/FrameDockManager.hpp>
#include <WxDockUI/Internal/LayoutOps.hpp>





namespace WxDockUI::Internal
{





	FrameDockManager::FrameDockManager(wxTopLevelWindow & aFrame, DockSystem & aDockSystem):
		mLayoutEngine(*this),
		mDockOverlay(*this),
		mDockSystem(aDockSystem),
		mFrame(aFrame)
	{
		mRoot.setChild(std::make_unique<Layout::TabNode>());

		aFrame.Bind(wxEVT_SIZE, &FrameDockManager::onFrameSize, this);
	}





	FrameDockManager::~FrameDockManager()
	{
		mLayoutEngine.clear();
	}





	void FrameDockManager::onFrameSize(wxSizeEvent & aEvent)
	{
		aEvent.Skip();
		updateLayout();
	}





	void FrameDockManager::addPane(const PaneId & aPaneId, const PaneInfo & aOptions)
	{
		// Add the pane to the layout:
		auto paneNode = std::make_unique<Layout::PaneNode>(aPaneId);
		if (aOptions.initialDock() == DockPosition::Center)
		{
			Layout::Ops::insertCenterPane(mRoot, std::move(paneNode));
		}
		else
		{
			auto sizePx = ((aOptions.initialDock() == DockPosition::Left) || (aOptions.initialDock() == DockPosition::Right)) ? aOptions.bestWidth() : aOptions.bestHeight();
			Layout::Ops::insertEdgePane(mRoot, std::move(paneNode), aOptions.initialDock(), sizePx);
		}
		Layout::Ops::cleanup(mRoot);
		updateLayout();
	}





	void FrameDockManager::updateLayout()
	{
		mFrame.Freeze();

		wxRect rect = mFrame.GetClientRect();
		mLayoutEngine.applyLayout(mRoot, &mFrame, rect);

		mFrame.Thaw();
	}





	void FrameDockManager::dumpLayout(std::ostream & aOut) const
	{
		mRoot.dump(aOut, 1);
	}





	void FrameDockManager::uncaptureMouse()
	{
		auto captured = mFrame.GetCapture();
		if (captured != nullptr)
		{
			captured->ReleaseMouse();
		}
	}





	void FrameDockManager::beginDrag(const Layout::PaneNode * aPane, const wxPoint & aScreenPos)
	{
		return mDockSystem.paneDragController().beginDrag(*this, aPane, aScreenPos);
	}





	void FrameDockManager::updateDrag(const wxPoint & aScreenPos)
	{
		return mDockSystem.paneDragController().updateDrag(aScreenPos);
	}





	void FrameDockManager::endDrag(const wxPoint & aScreenPos)
	{
		return mDockSystem.paneDragController().endDrag(aScreenPos);
	}





	void FrameDockManager::cancelDrag()
	{
		return mDockSystem.paneDragController().cancelDrag();
	}





	void FrameDockManager::performDock(
		FrameDockManager & aSourceFrame,
		const Layout::PaneNode & aDraggedPane,
		const DockTarget & aTarget
	)
	{
		mDockSystem.performDock(aSourceFrame, aDraggedPane, aTarget);
	}





}  // namespace WxDockUI
