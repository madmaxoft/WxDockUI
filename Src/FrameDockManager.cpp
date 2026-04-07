#include <WxDockUI/Internal/FrameDockManager.hpp>
#include <WxDockUI/Internal/LayoutOps.hpp>





namespace WxDockUI::Internal
{





	FrameDockManager::FrameDockManager(wxTopLevelWindow & aFrame, DockSystem & aDockSystem):
		mLayoutEngine(*this),
		mPaneDragController(*this),
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





	void FrameDockManager::performDock(const Layout::PaneNode & aDraggedPane, const Internal::DockTarget & aTarget)
	{
		if (!aTarget.isValid())
		{
			return;
		}

		const std::string & paneId = aDraggedPane.paneId();
		bool didMove = false;
		#ifdef WXDOCKUI_DUMP_LAYOUT_ON_DOCK
			std::cout << "Performing a dock of pane \"" << paneId << "\" into target " << aTarget.describe() << std::endl;
			std::cout << "Layout before the dock:" << std::endl;
			dumpLayout(std::cout);
		#endif
		if (aTarget.isRootSplit())
		{
			auto pos = aTarget.dockPosition();
			auto pane = Layout::Ops::removePane(mRoot, paneId);
			if (pane.get() == nullptr)
			{
				return;
			}
			auto clientRect = mFrame.GetClientRect();
			auto sizePx = aTarget.isHorizontalSplit() ? (clientRect.width / 5) : (clientRect.height / 5);
			Layout::Ops::insertEdgePane(mRoot, std::move(pane), pos, sizePx);
			didMove = true;
		}
		else if (aTarget.isPaneSplit())
		{
			if (aTarget.mNode == nullptr)
			{
				return;
			}
			if (aTarget.mNode == &aDraggedPane)
			{
				auto tab = aTarget.mNode->parent()->asTabNode();
				if (
					(tab == nullptr) ||         // The target pane is not within a tab
					(tab->panes().size() == 1)  // The source pane is the last pane in a tab
				)
				{
					return;
				}
			}
			auto pos = aTarget.dockPosition();
			didMove = Layout::Ops::movePaneToNodeEdge(mRoot, paneId, const_cast<Layout::BaseNode &>(*aTarget.mNode), pos);
		}
		else if (aTarget.mKind == Internal::DockTarget::Kind::PaneTab)
		{
			if ((aTarget.mNode == nullptr) || (aTarget.mNode == &aDraggedPane))
			{
				return;
			}
			switch (aTarget.mNode->type())
			{
				case Layout::NodeType::Pane:
				{
					didMove = Layout::Ops::mergePanesIntoTab(mRoot, paneId, aTarget.mNode->asPaneNode()->paneId(), -1);
					break;
				}
				case Layout::NodeType::Tab:
				{
					auto tabNode = aTarget.mNode->asTabNode();
					didMove = Layout::Ops::movePaneToTab(mRoot, paneId, const_cast<Layout::TabNode &>(*tabNode), 0);
					break;
				}
				default:
				{
					assert(!"Unhandled node type");
					break;
				}
			}
		}
		if (!didMove)
		{
			return;
		}

		Layout::Ops::cleanup(mRoot);

		#ifdef WXDOCKUI_DUMP_LAYOUT_ON_DOCK
			std::cout << "Layout after the dock:" << std::endl;
			dumpLayout(std::cout);
			std::flush(std::cout);
			Layout::Ops::validateLayoutTree(mRoot, &std::cerr);
		#endif

		// Call updateLayout after processing all events, since this performDock call is most likely called
		// from within an event handler for an object that could get destroyed by the re-layout
		mFrame.CallAfter([this](){
			updateLayout();
		});
	}





	void FrameDockManager::uncaptureMouse()
	{
		auto captured = mFrame.GetCapture();
		if (captured != nullptr)
		{
			captured->ReleaseMouse();
		}
	}





}  // namespace WxDockUI
