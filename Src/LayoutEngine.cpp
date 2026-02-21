#include <WxDockUI/Internal/LayoutEngine.h>
#include <WxDockUI/Internal/Layout.h>
#include <WxDockUI/Internal/PaneContainer.h>
#include <WxDockUI/Internal/TabContainerWindow.h>
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
		Internal::PaneContainer * container = nullptr;
		auto itr = mPaneContainers.find(&aNode);
		if (itr == mPaneContainers.end())
		{
			auto window = mFrameDockManager.findPaneWindow(aNode.paneId());
			if (window == nullptr)
			{
				assert(!"No window found for pane");
				return;
			}
			container = new Internal::PaneContainer(mFrameDockManager, aNode, aParent, window, aNode.paneId());
			mPaneContainers[&aNode] = container;
		}
		else
		{
			container = itr->second;
		}

		container->SetSize(aRect);
		container->Show();
	}





	void LayoutEngine::layoutTabNode(
		TabNode & aNode,
		wxWindow * aParent,
		const wxRect & aRect
	)
	{
		auto * tabWindow = tabContainerWindow(&aNode);
		if (tabWindow == nullptr)
		{
			return;
		}
		tabWindow->updateLayout();
		if (tabWindow->GetParent() != aParent)
		{
			tabWindow->Reparent(aParent);
		}
		tabWindow->SetSize(aRect);
		tabWindow->Show();
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





	Layout::PaneNode * LayoutEngine::paneNodeAtScreenPos(const wxPoint & aScreenPos)
	{
		for (const auto & container: mPaneContainers)
		{
			if (container.second->GetScreenRect().Contains(aScreenPos))
			{
				return &container.second->paneNode();
			}
		}
		for (const auto & tab: mTabContainerWindows)
		{
			if (tab.second->GetScreenRect().Contains(aScreenPos))
			{
				return const_cast<Layout::PaneNode *>(tab.first->activePane());
			}
		}
		return nullptr;
	}





	Internal::TabContainerWindow * LayoutEngine::tabContainerWindow(TabNode * aTabNode)
	{
		auto itr = mTabContainerWindows.find(aTabNode);
		if (itr != mTabContainerWindows.end())
		{
			return itr->second.get();
		}

		// Not found, create a new one:
		auto tcw = std::make_unique<Internal::TabContainerWindow>(mFrameDockManager, mFrameDockManager.frame(), *aTabNode);
		auto * raw = tcw.get();
		mTabContainerWindows.emplace(aTabNode, std::move(tcw));
		return raw;
	}





}  // namespace WxDockUI::Layout
