#include <WxDockUI/FrameDockManager.h>
#include <WxDockUI/DockSystem.h>
#include <WxDockUI/Internal/PaneContainer.h>
#include <WxDockUI/Internal/TabContainerWindow.h>
#include <WxDockUI/Internal/LayoutOps.h>





namespace WxDockUI
{





	///////////////////////////////////////////////////////////////////////////////
	// PaneInfo:

	PaneInfo::PaneInfo(const PaneId & aPaneId):
		mPaneId(aPaneId)
	{
	}





	PaneInfo & PaneInfo::caption(const wxString & aCaption)
	{
		mCaption = aCaption;
		return *this;
	}





	PaneInfo & PaneInfo::left()
	{
		mInitialDock = DockPosition::Left;
		return *this;
	}





	PaneInfo & PaneInfo::right()
	{
		mInitialDock = DockPosition::Right;
		return *this;
	}





	PaneInfo & PaneInfo::bottom()
	{
		mInitialDock = DockPosition::Bottom;
		return *this;
	}





	PaneInfo & PaneInfo::top()
	{
		mInitialDock = DockPosition::Top;
		return *this;
	}





	PaneInfo & PaneInfo::center()
	{
		mInitialDock = DockPosition::Center;
		return *this;
	}





	PaneInfo & PaneInfo::bestSize(int aWidth, int aHeight)
	{
		mBestWidth = aWidth;
		mBestHeight = aHeight;
		return *this;
	}





	///////////////////////////////////////////////////////////////////////////////
	// FrameDockManager:

	FrameDockManager::FrameDockManager(wxFrame & aFrame, DockSystem & aDockSystem):
		mLayoutEngine(*this),
		mPaneDragController(*this),
		mDockOverlay(*this),
		mDockSystem(aDockSystem),
		mFrame(aFrame)
	{
		mDockSystem.registerManager(this);
		mRoot.setChild(std::make_unique<Layout::TabNode>());
	}





	FrameDockManager::~FrameDockManager()
	{
		mDockSystem.unregisterManager(this);
	}





	void FrameDockManager::addPane(wxWindow * aWindow, const PaneInfo & aOptions)
	{
		// Register the pane:
		mPaneWindows[aOptions.paneId()] = aWindow;
		mPaneInfos[aOptions.paneId()] = aOptions;
		// TODO: Check if such a pane ID already exists

		// Add the pane to the layout:
		std::cout << "Before addPane(" << aOptions.caption() << "):" << std::endl;
		dumpLayout(std::cout);
		auto paneNode = std::make_unique<Layout::PaneNode>(aOptions.paneId());
		if (aOptions.initialDock() == DockPosition::Center)
		{
			Layout::Ops::insertCenterPane(mRoot, std::move(paneNode));
		}
		else
		{
			Layout::Ops::insertEdgePane(mRoot, std::move(paneNode), aOptions.initialDock());
		}
		Layout::Ops::cleanup(mRoot);
		updateLayout();
		std::cout << "After addPane(" << aOptions.caption() << "):" << std::endl;
		dumpLayout(std::cout);
		std::cout << std::endl;
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
		aOut << "FrameDockManager layout:\n";

		if (mRoot.child() == nullptr)
		{
			aOut << "\t<empty>\n";
			return;
		}

		mRoot.child()->dump(aOut, 1);
	}





	wxWindow * FrameDockManager::findPaneWindow(const PaneId & aId) const
	{
		auto itr = mPaneWindows.find(aId);
		if (itr == mPaneWindows.end())
		{
			return nullptr;
		}
		return itr->second;
	}





	wxWindow * FrameDockManager::findPaneWindow(const Layout::PaneNode & aPaneNode) const
	{
		return findPaneWindow(aPaneNode.paneId());
	}





	const PaneInfo * FrameDockManager::findPaneInfo(const PaneId & aId)
	{
		auto itr = mPaneInfos.find(aId);
		if (itr == mPaneInfos.end())
		{
			return nullptr;
		}
		return &(itr->second);
	}





	void FrameDockManager::performDock(Layout::PaneNode & aDraggedPane, const Internal::DockTarget & aTarget)
	{
		if (!aTarget.isValid())
		{
			return;
		}

		const std::string & paneId = aDraggedPane.paneId();

		bool didMove = false;

		if (aTarget.isRootSplit())
		{
			auto pos = aTarget.dockPosition();
			auto pane = Layout::Ops::removePane(mRoot, paneId);
			if (pane.get() == nullptr)
			{
				return;
			}
			Layout::Ops::insertEdgePane(mRoot, std::move(pane), pos);
			didMove = true;
		}
		else if (aTarget.isPaneSplit())
		{
			if ((aTarget.mPane == nullptr) || (aTarget.mPane == &aDraggedPane))
			{
				return;
			}
			auto pos = aTarget.dockPosition();
			didMove = Layout::Ops::movePaneToNodeEdge(mRoot, paneId, *aTarget.mPane, pos);
		}
		else if (aTarget.mKind == Internal::DockTarget::Kind::PaneTab)
		{
			if ((aTarget.mPane == nullptr) || (aTarget.mPane == &aDraggedPane))
			{
				return;
			}
			didMove = Layout::Ops::mergePanesIntoTab(mRoot, paneId, aTarget.mPane->paneId(), -1);
		}
		if (!didMove)
		{
			return;
		}

		Layout::Ops::cleanup(mRoot);

		#ifndef NDEBUG
			Layout::Ops::validateLayoutTree(mRoot, &std::cerr);
		#endif

		updateLayout();
	}





}  // namespace WxDockUI
