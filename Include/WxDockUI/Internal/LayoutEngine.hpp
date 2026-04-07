#pragma once

#include <unordered_map>
#include <unordered_set>
#include <memory>

#include <wx/window.h>





// fwd:
namespace WxDockUI
{
	namespace Internal
	{
		class FrameDockManager;
		class PaneContainer;
		class SplitContainer;
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
	Attaches PaneContainer instances to PaneNodes, TabContainerWindow instances to TabNodes and
	SplitContainer instances to SplitNodes.
	Since it knows about all the geometry, it also performs hit-testing */
	class LayoutEngine
	{
		friend class WxDockUI::Internal::SplitContainer;


		/** The FrameDockManager responsible for handling all the PaneContainers created by this class. */
		Internal::FrameDockManager & mFrameDockManager;

		/** The PaneContainer instances created by this layout engine in mFrameDockManager. */
		std::unordered_map<const PaneNode *, std::unique_ptr<Internal::PaneContainer>> mPaneContainers;

		/** Mapping of layout TabNodes to the TabContainer instances representing them. */
		std::unordered_map<const Layout::TabNode *, std::unique_ptr<WxDockUI::Internal::TabContainer>> mTabContainers;

		/** Mapping of layout SplitNodes to the SplitContainer instances representing them. */
		std::unordered_map<const Layout::SplitNode *, std::unique_ptr<WxDockUI::Internal::SplitContainer>> mSplitContainers;



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

		/** Recursively collects all pane nodes, tab nodes and split nodes within the specified
		node into the appropriate sets. */
		void collectNodes(const BaseNode & aNode,
			std::unordered_set<const PaneNode *> & aOutPaneNodes,
			std::unordered_set<const TabNode *> & aOutTabNodes,
			std::unordered_set<const SplitNode *> & aOutSplitNodes
		);

		/** Removes SplitContainers, TabContainers and PaneContainers that no longer have a corresponding
		layout node within the specified root. */
		void pruneContainers(const RootNode & aRoot);


	public:

		/** Creates a new instance bound to the specified FraomeDockManager. */
		explicit LayoutEngine(Internal::FrameDockManager & aFrameDockManager);

		/** Destroys the instance. */
		~LayoutEngine();

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

		/** Returns the TabContainer at the specified screen position, or nullptr if none. */
		const Internal::TabContainer * tabContainerAtScreenPos(const wxPoint & aScreenPos);

		/** Internal: Returns the TabContainer representing the specified layout tab node.
		If no such window exists, creates a new one and remembers it in mTabContainerWindows. */
		Internal::TabContainer * ensureTabContainer(TabNode * aTabNode);

		/** Internal: Returns the TabContainer representing the specified layout tab node.
		If no such window exists, returns nullptr. */
		Internal::TabContainer * maybeTabContainer(const TabNode * aTabNode);

		/** Internal: Returns the PaneContainer representing the specified layout pane node.
		If no such container exists, creates a new one and remembers it in mPaneContainers.
		Returns nullptr (and asserts) if the pane's UI window cannot be found in mFrameDockManager. */
		Internal::PaneContainer * ensurePaneContainer(const PaneNode & aPaneNode);

		/** Internal: Returns the PaneContainer representing the specified layout pane node.
		If no such container exists, returns nullptr. */
		Internal::PaneContainer * maybePaneContainer(const PaneNode & aPaneNode);

		/** Internal: Returns the SplitContainer representing the specified layout split node.
		If no such container exists, creates a new one and remembers it in mSplitContainers. */
		Internal::SplitContainer * ensureSplitContainer(SplitNode & aSplitNode);
	};





}  // namespace WxDockUI::Internal
