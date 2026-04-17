#include <WxDockUI/DockSystem.hpp>

#include <format>

#include <WxDockUI/Internal/FrameDockManager.hpp>
#include <WxDockUI/Internal/FloatingDockFrame.hpp>
#include <WxDockUI/Internal/LayoutOps.hpp>
#include <WxDockUI/Internal/PerfTrace.hpp>





namespace WxDockUI
{





	DockSystem::DockSystem():
		mPaneDragController(*this)
	{
	}





	// Nothing explicit needed yet,
	// we just need the destructor to be defined in cpp file, so that unique_ptr<FrameDockManager> compiles.
	DockSystem::~DockSystem() = default;





	Internal::FrameDockManager & DockSystem::ensureDockTargetFrame(const Internal::DockTarget & aDockTarget)
	{
		if (aDockTarget.mTargetFrame != nullptr)
		{
			return const_cast<Internal::FrameDockManager &>(*(aDockTarget.mTargetFrame));
		}

		// Create a new frame and FrameDockManager for the new floating window:
		auto frame = new Internal::FloatingDockFrame(*this);
		auto mgr = std::make_unique<Internal::FrameDockManager>(*frame, *this, Internal::FrameDockManager::Role::Floating);
		frame->setFrameDockManager(*mgr);
		mZOrderTracker.add(*mgr);
		mManagedWindows.push_back(std::move(mgr));
		return frame->frameDockManager();
	}





	void DockSystem::performDock(
		Internal::FrameDockManager & aSourceFrame,
		const Layout::PaneNode & aDraggedPane,
		const Internal::DockTarget & aTarget
	)
	{
		WXDOCKUI_PROFILE_SCOPE("DockSystem::performDock");
		if (!aTarget.isValid())
		{
			return;
		}

		auto & targetFrame = ensureDockTargetFrame(aTarget);
		const std::string & paneId = aDraggedPane.paneId();
		bool didMove = false;
		#ifdef WXDOCKUI_DUMP_LAYOUT_ON_DOCK
			std::cout << "Performing a dock of pane \"" << paneId << "\" into target " << aTarget.describe() << std::endl;
			std::cout << "Layout before the dock:" << std::endl;
			aSourceFrame.dumpLayout(std::cout);
			if (&aSourceFrame != &targetFrame)
			{
				std::cout << "Target:" << std::endl;
				targetFrame.dumpLayout(std::cout);
			}
		#endif
		if (aTarget.isRootSplit())
		{
			auto pos = aTarget.dockPosition();
			auto pane = Layout::Ops::removePane(aSourceFrame.rootNode(), paneId);
			if (pane.get() == nullptr)
			{
				return;
			}
			auto clientRect = targetFrame.frame()->GetClientRect();
			auto sizePx = aTarget.isHorizontalSplit() ? (clientRect.width / 5) : (clientRect.height / 5);
			Layout::Ops::insertEdgePane(targetFrame.rootNode(), std::move(pane), pos, sizePx);
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
					(tab->panes().size() == 1)  // The source pane is the only pane in a tab
				)
				{
					return;
				}
			}
			auto pos = aTarget.dockPosition();
			didMove = Layout::Ops::movePaneToNodeEdge(aSourceFrame.rootNode(), paneId, targetFrame.rootNode(), const_cast<Layout::BaseNode &>(*aTarget.mNode), pos);
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
					didMove = Layout::Ops::mergePanesIntoTab(aSourceFrame.rootNode(), paneId, targetFrame.rootNode(), aTarget.mNode->asPaneNode()->paneId(), -1);
					break;
				}
				case Layout::NodeType::Tab:
				{
					auto tabNode = aTarget.mNode->asTabNode();
					didMove = Layout::Ops::movePaneToTab(aSourceFrame.rootNode(), paneId, targetFrame.rootNode(), const_cast<Layout::TabNode &>(*tabNode), -1);
					break;
				}
				default:
				{
					assert(!"Unhandled node type");
					break;
				}
			}
		}
		else
		{
			assert(targetFrame.rootNode().child()->type() == Layout::NodeType::Tab);
			assert(targetFrame.rootNode().child()->asTabNode()->panes().empty());
			auto pane = Layout::Ops::removePane(aSourceFrame.rootNode(), paneId);
			if (pane == nullptr)
			{
				assert(!"Unknown pane");
				return;
			}
			targetFrame.rootNode().setChild(std::move(pane));
			didMove = true;
		}
		if (!didMove)
		{
			return;
		}

		Layout::Ops::cleanup(aSourceFrame.rootNode());
		if (&aSourceFrame != &targetFrame)
		{
			Layout::Ops::cleanup(targetFrame.rootNode());
		}

		#ifdef WXDOCKUI_DUMP_LAYOUT_ON_DOCK
			std::cout << "Layout after the dock:" << std::endl;
			aSourceFrame.dumpLayout(std::cout);
			if (&aSourceFrame != &targetFrame)
			{
				std::cout << "Target:" << std::endl;
				targetFrame.dumpLayout(std::cout);
			}
			std::flush(std::cout);
			Layout::Ops::validateLayoutTree(aSourceFrame.rootNode(), &std::cerr);
			if (&aSourceFrame != &targetFrame)
			{
				Layout::Ops::validateLayoutTree(targetFrame.rootNode(), &std::cerr);
			}
		#endif

		// Call updateLayout after processing all events, since this performDock call is most likely called
		// from within an event handler for an object that could get destroyed by the re-layout
		aSourceFrame.frame()->CallAfter([this, &aSourceFrame](){
			aSourceFrame.updateLayout();
			destroyIfEmptyFloating(aSourceFrame);
		});
		if (&aSourceFrame != &targetFrame)
		{
			targetFrame.frame()->CallAfter([this, &targetFrame](){
				targetFrame.updateLayout();
				if (targetFrame.role() == Internal::FrameDockManager::Role::Floating)
				{
					static_cast<Internal::FloatingDockFrame *>(targetFrame.frame())->updateCaption();
					targetFrame.frame()->Show();
					targetFrame.frame()->Raise();
				}
			});
		}
	}





	void DockSystem::destroyIfEmptyFloating(Internal::FrameDockManager & aFrameDockManager)
	{
		if (aFrameDockManager.role() != Internal::FrameDockManager::Role::Floating)
		{
			return;
		}
		if (!aFrameDockManager.isEmpty())
		{
			return;
		}
		destroyManagedWindow(aFrameDockManager);
	}





	void DockSystem::destroyManagedWindow(Internal::FrameDockManager & aFrameDockManager)
	{
		aFrameDockManager.frame()->Destroy();
	}





	void DockSystem::onManagedWindowDestroy(Internal::FrameDockManager & aFrameDockManager)
	{
		mZOrderTracker.remove(aFrameDockManager);
		for (auto itr = mManagedWindows.begin(), end = mManagedWindows.end(); itr != end; ++itr)
		{
			if (itr->get() == &aFrameDockManager)
			{
				mManagedWindows.erase(itr);
				break;
			}
		}
	}





	Internal::FrameDockManager * DockSystem::managedWindowAtScreenPos(const wxPoint & aScreenPos)
	{
		return mZOrderTracker.hitTest(aScreenPos);
	}





	void DockSystem::manageWindow(wxTopLevelWindow & aWindow)
	{
		// Check if the window is already managed:
		for (const auto & fdm: mManagedWindows)
		{
			if (fdm->frame() == &aWindow)
			{
				return;
			}
		}

		auto mgr = std::make_unique<Internal::FrameDockManager>(aWindow, *this, Internal::FrameDockManager::Role::Host);
		mZOrderTracker.add(*mgr);
		mManagedWindows.push_back(std::move(mgr));
	}





	void DockSystem::addPane(
		const wxTopLevelWindow & aParentDockWindow,
		wxWindow * aPaneWindow,
		const PaneId & aPaneId,
		const PaneInfo & aPaneInfo
	)
	{
		// Check if the pane is already present:
		auto itr = mPanes.find(aPaneId);
		if (itr != mPanes.end())
		{
			throw std::runtime_error(std::format("Pane {} already present", aPaneId));
		}

		// Add the pane, if the parent dock window is managed:
		for (const auto & fdm: mManagedWindows)
		{
			if (fdm->frame() == &aParentDockWindow)
			{
				mPanes[aPaneId] = {std::move(aPaneInfo), aPaneWindow};
				fdm->addPane(aPaneId, aPaneInfo);
				return;
			}
		}

		// Parent dock window not found:
		throw std::runtime_error("Cannot add pane, the ParentDockFrame is not managed");
	}





	wxWindow * DockSystem::findPaneWindow(const PaneId & aId) const
	{
		auto itr = mPanes.find(aId);
		if (itr == mPanes.end())
		{
			return nullptr;
		}
		return itr->second.second;
	}





	const PaneInfo * DockSystem::findPaneInfo(const PaneId & aId) const
	{
		auto itr = mPanes.find(aId);
		if (itr == mPanes.end())
		{
			return nullptr;
		}
		return &(itr->second.first);
	}

}  // namespace WxDockUI
