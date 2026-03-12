#pragma once

#include <unordered_map>
#include <memory>

#include <wx/window.h>





// fwd:
namespace WxDockUI
{
	class FrameDockManager;
	namespace Internal
	{
		class PaneContainer;
		class TabContainer;
	}
	namespace Layout
	{
		class RootNode;
		class BaseNode;
		class SplitNode;
		class PaneNode;
		class TabNode;
	}
}





namespace WxDockUI::Layout
{





	/** Processes layout into actual UI elements that can be displayed.
	Attaches PaneContainer instances to PaneNodes and TabContainerWindow instances to TabNodes.
	Since it knows about all the geometry, it also performs hit-testing */
	class LayoutEngine
	{
		/** The FrameDockManager responsible for handling all the PaneContainers created by this class. */
		WxDockUI::FrameDockManager & mFrameDockManager;

		/** The PaneContainer instances created by this layout engine in mFrameDockManager. */
		std::unordered_map<const PaneNode *, std::unique_ptr<Internal::PaneContainer>> mPaneContainers;

		/** Mapping of layout TabNodes to the TabContainer instances representing them. */
		std::unordered_map<Layout::TabNode *, std::unique_ptr<WxDockUI::Internal::TabContainer>> mTabContainers;



		void layoutNode(
			BaseNode & aNode,
			wxWindow * aParent,
			const wxRect & aRect
		);

		void layoutSplitNode(
			SplitNode & aNode,
			wxWindow * aParent,
			const wxRect & aRect
		);

		void layoutPaneNode(
			PaneNode & aNode,
			wxWindow * aParent,
			const wxRect & aRect
		);

		void layoutTabNode(
			TabNode & aNode,
			wxWindow * aParent,
			const wxRect & aRect
		);


	public:

		/** Creates a new instance bound to the specified FraomeDockManager. */
		explicit LayoutEngine(WxDockUI::FrameDockManager & aFrameDockManager);

		/** Applies the layout from the root node into the specified window and rectangle. */
		void applyLayout(
			RootNode & aRoot,
			wxWindow * aParent,
			const wxRect & aRect
		);

		/** Deletes all UI. */
		void clear();

		/** Returns the PaneNode at the specified screen position, or nullptr if none. */
		const Layout::PaneNode * paneNodeAtScreenPos(const wxPoint & aScreenPos);

		/** Internal: Returns the TabContainer representing the specified layout tab node.
		If no such window exists, creates a new one and remembers it in mTabContainerWindows. */
		Internal::TabContainer * ensureTabContainer(TabNode * aTabNode);

		/** Internal: Returns the PaneContainer representing the specified layout pane node.
		If no such container exists, creates a new one and remembers it in mPaneContainers.
		Returns nullptr (and asserts) if the pane's UI window cannot be found in mFrameDockManager. */
		Internal::PaneContainer * ensurePaneContainer(const PaneNode & aPaneNode);

		/** Internal: Returns the PaneContainer representing the specified layout pane node.
		If no such container exists, returns nullptr. */
		Internal::PaneContainer * maybePaneContainer(const PaneNode & aPaneNode);
	};





}  // namespace WxDockUI::Internal
