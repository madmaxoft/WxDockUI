#include <WxDockUI/Internal/LayoutEngine.h>
#include <WxDockUI/Internal/Layout.h>
#include <WxDockUI/Internal/PaneContainer.h>
#include <WxDockUI/Internal/TabContainer.h>
#include <WxDockUI/FrameDockManager.h>





namespace WxDockUI::Layout
{





	void LayoutEngine::layoutNode(
		BaseNode & aNode,
		wxWindow * aParent,
		const wxRect & aRect
	)
	{
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
		float totalRatio = 0.0f;
		for (const auto & child: aNode.children())
		{
			totalRatio += child.mRatio;
		}

		int offset = 0;
		for (const auto & child : aNode.children())
		{
			float fraction = child.mRatio / totalRatio;
			wxRect childRect = aRect;
			if (aNode.orientation() == SplitOrientation::Horizontal)
			{
				int width = int(aRect.width * fraction);
				childRect.x += offset;
				childRect.width = width;
				offset += width;
			}
			else
			{
				int height = int(aRect.height * fraction);
				childRect.y += offset;
				childRect.height = height;
				offset += height;
			}

			layoutNode(*child.mNode, aParent, childRect);
		}
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
		container->Reparent(aParent);
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





	LayoutEngine::LayoutEngine(WxDockUI::FrameDockManager & aFrameDockManager):
		mFrameDockManager(aFrameDockManager)
	{
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

		layoutNode(*aRoot.child(), aParent, aRect);
	}





	void LayoutEngine::clear()
	{
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





}  // namespace WxDockUI::Layout
