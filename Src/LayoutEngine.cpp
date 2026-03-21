#include <WxDockUI/Internal/LayoutEngine.h>
#include <WxDockUI/Internal/Layout.h>
#include <WxDockUI/Internal/PaneContainer.h>
#include <WxDockUI/Internal/SplitContainer.h>
#include <WxDockUI/Internal/TabContainer.h>
#include <WxDockUI/FrameDockManager.h>





namespace WxDockUI::Layout
{





	/** Removes those elements from aMap whose key is not in aSet. */
	template <typename KeyType, typename ValueType>
	void removeFromMapThoseNotInSet(std::unordered_map<KeyType, ValueType> & aMap, const std::unordered_set<KeyType> & aSet)
	{
		for (auto itr = aMap.begin(); itr != aMap.end(); )
		{
			if (aSet.find(itr->first) == aSet.end())
			{
				#ifdef WXDOCKUI_DEBUG_LIFETIME
					std::cout << "Pruning container at " << itr->second << "." << std::endl;
				#endif
				itr = aMap.erase(itr);
			}
			else
			{
				++itr;
			}
		}
	}





	void LayoutEngine::layoutNode(
		BaseNode & aNode,
		wxWindow * aParent,
		const wxRect & aRect
	)
	{
		assert(aRect.width > 0);
		assert(aRect.height > 0);

		switch (aNode.type())
		{
			case NodeType::Split: return layoutSplitNode(static_cast<SplitNode &>(aNode), aParent, aRect);
			case NodeType::Pane:  return layoutPaneNode (static_cast<PaneNode & >(aNode), aParent, aRect);
			case NodeType::Tab:   return layoutTabNode  (static_cast<TabNode &  >(aNode), aParent, aRect);
			default:
			{
				assert(!"Invalid node type");
				return;
			}
		}
	}






	void LayoutEngine::layoutSplitNode(
		SplitNode & aNode,
		wxWindow * aParent,
		const wxRect & aRect
	)
	{
		auto container = ensureSplitContainer(aNode);
		if (container == nullptr)
		{
			assert(!"Failed to create a split container");
			return;
		}
		if (container->GetParent() != aParent)
		{
			container->Reparent(aParent);
		}
		container->SetSize(aRect);
		container->updateLayout();
		container->Show();
	}






	void LayoutEngine::layoutPaneNode(
		PaneNode & aNode,
		wxWindow * aParent,
		const wxRect & aRect
	)
	{
		auto container = ensurePaneContainer(aNode);
		if (container == nullptr)
		{
			assert(!"Failed to create a pane container");
			return;
		}
		if (container->GetParent() != aParent)
		{
			container->Reparent(aParent);
		}
		container->showCaptionBar(true);
		container->SetSize(aRect);
		container->Show();
	}





	void LayoutEngine::layoutTabNode(
		TabNode & aNode,
		wxWindow * aParent,
		const wxRect & aRect
	)
	{
		auto * tc = ensureTabContainer(&aNode);
		if (tc == nullptr)
		{
			assert(!"Failed to create TabContainer");
			return;
		}
		tc->updateLayout();
		if (tc->GetParent() != aParent)
		{
			tc->Reparent(aParent);
		}
		tc->SetSize(aRect);
		tc->Show();
	}





	void LayoutEngine::collectNodes(
		const BaseNode & aNode,
		std::unordered_set<const PaneNode *> & aOutPaneNodes,
		std::unordered_set<const TabNode *> & aOutTabNodes,
		std::unordered_set<const SplitNode *> & aOutSplitNodes
	)
	{
		switch (aNode.type())
		{
			case NodeType::Root:
			{
				collectNodes(*(aNode.asRootNode()->child()), aOutPaneNodes, aOutTabNodes, aOutSplitNodes);
				return;
			}
			case NodeType::Pane:
			{
				aOutPaneNodes.insert(aNode.asPaneNode());
				return;
			}
			case NodeType::Tab:
			{
				aOutTabNodes.insert(aNode.asTabNode());
				for (const auto & pane: aNode.asTabNode()->panes())
				{
					aOutPaneNodes.insert(pane.get());
				}
				return;
			}
			case NodeType::Split:
			{
				aOutSplitNodes.insert(aNode.asSplitNode());
				for (const auto & ch: aNode.asSplitNode()->children())
				{
					collectNodes(*ch.mNode, aOutPaneNodes, aOutTabNodes, aOutSplitNodes);
				}
				return;
			}
		}
		assert(!"Unknown node type");
	}





	void LayoutEngine::pruneContainers(const RootNode & aRoot)
	{
		#ifdef WXDOCKUI_DEBUG_LIFETIME
			std::cout << "Pruning containers for the LayoutEngine at " << this << "." << std::endl;
		#endif

		// Collect all nodes currently in the root:
		std::unordered_set<const PaneNode *> paneNodes;
		std::unordered_set<const TabNode *> tabNodes;
		std::unordered_set<const SplitNode *> splitNodes;
		collectNodes(aRoot, paneNodes, tabNodes, splitNodes);

		removeFromMapThoseNotInSet(mPaneContainers,  paneNodes);
		removeFromMapThoseNotInSet(mTabContainers,   tabNodes);
		removeFromMapThoseNotInSet(mSplitContainers, splitNodes);
	}





	LayoutEngine::LayoutEngine(WxDockUI::FrameDockManager & aFrameDockManager):
		mFrameDockManager(aFrameDockManager)
	{
		#ifdef WXDOCKUI_DEBUG_LIFETIME
			std::cout << "Created a LayoutEngine at " << this << "." << std::endl;
		#endif
	}





	LayoutEngine::~LayoutEngine()
	{
		// Nothing explicit needed yet
		// The destructor must be defined in cpp file otherwise clients would need to include all XContainer classes.
		#ifdef WXDOCKUI_DEBUG_LIFETIME
			std::cout << "Deleting LayoutEngine at " << this << "." << std::endl;
		#endif
	}





	void LayoutEngine::applyLayout(
		RootNode & aRoot,
		wxWindow * aParent,
		const wxRect & aRect
	)
	{
		if (aRoot.child() == nullptr)
		{
			return;
		}

		pruneContainers(aRoot);
		layoutNode(*aRoot.child(), aParent, aRect);
	}





	void LayoutEngine::clear()
	{
		#ifdef WXDOCKUI_DEBUG_LIFETIME
			std::cout << "Clearing LayoutEngine at " << this << "." << std::endl;
		#endif

		for (auto & sc: mSplitContainers)
		{
			sc.second->clear();
		}
		mSplitContainers.clear();
		mPaneContainers.clear();
		mTabContainers.clear();
	}





	const Layout::PaneNode * LayoutEngine::paneNodeAtScreenPos(const wxPoint & aScreenPos)
	{
		for (const auto & container: mPaneContainers)
		{
			if (container.second->GetScreenRect().Contains(aScreenPos))
			{
				return &container.second->paneNode();
			}
		}
		for (const auto & tab: mTabContainers)
		{
			if (tab.second->GetScreenRect().Contains(aScreenPos))
			{
				return const_cast<Layout::PaneNode *>(tab.first->activePane());
			}
		}
		return nullptr;
	}





	Internal::TabContainer * LayoutEngine::ensureTabContainer(TabNode * aTabNode)
	{
		auto itr = mTabContainers.find(aTabNode);
		if (itr != mTabContainers.end())
		{
			return itr->second.get();
		}

		// Not found, create a new one:
		auto tc = std::make_unique<Internal::TabContainer>(mFrameDockManager, mFrameDockManager.frame(), *aTabNode);
		auto * raw = tc.get();
		mTabContainers.emplace(aTabNode, std::move(tc));
		return raw;
	}





	Internal::PaneContainer * LayoutEngine::ensurePaneContainer(const PaneNode & aPaneNode)
	{
		auto itr = mPaneContainers.find(&aPaneNode);
		if (itr != mPaneContainers.end())
		{
			return itr->second.get();
		}

		// Create a new one:
		auto window = mFrameDockManager.findPaneWindow(aPaneNode.paneId());
		if (window == nullptr)
		{
			assert(!"No window found for pane");
			return nullptr;
		}
		mPaneContainers[&aPaneNode].reset(new Internal::PaneContainer(mFrameDockManager, aPaneNode, mFrameDockManager.frame(), window, aPaneNode.paneId()));
		return mPaneContainers[&aPaneNode].get();
	}





	Internal::PaneContainer * LayoutEngine::maybePaneContainer(const PaneNode & aPaneNode)
	{
		auto itr = mPaneContainers.find(&aPaneNode);
		if (itr != mPaneContainers.end())
		{
			return itr->second.get();
		}
		return nullptr;
	}





	Internal::SplitContainer * LayoutEngine::ensureSplitContainer(SplitNode & aSplitNode)
	{
		auto itr = mSplitContainers.find(&aSplitNode);
		if (itr != mSplitContainers.end())
		{
			return itr->second.get();
		}

		// Not found, create a new one:
		auto tc = std::make_unique<Internal::SplitContainer>(mFrameDockManager, mFrameDockManager.frame(), aSplitNode);
		auto * raw = tc.get();
		mSplitContainers.emplace(&aSplitNode, std::move(tc));
		return raw;
	}





}  // namespace WxDockUI::Layout
