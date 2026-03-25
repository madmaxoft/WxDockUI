#pragma once

#include "WxDockUI/Enums.h"
#include "Layout.h"





namespace WxDockUI::Layout::Ops
{




	/** Inserts the specified pane node as the center pane in the root's hierarchy. */
	void insertCenterPane(RootNode & aRoot, std::unique_ptr<PaneNode> aPaneNode);

	/** Inserts the specified pane node to the specified dock edge in the root's hierarchy. */
	void insertEdgePane(RootNode & aRoot, std::unique_ptr<PaneNode> aPaneNode, WxDockUI::DockPosition aPos);

	/** Returns the PaneNode instance representing the specified pane ID in the hierarchy under aNode.
	Returns nullptr if not found. */
	const PaneNode * findPaneNodeRecursive(const BaseNode & aNode, const std::string & aPaneId);

	/** If the split has a single child, replaces the split in its parent with the child.
	Returns true if replaced, false if not. */
	bool tryReplaceSplitWithOnlyChild(SplitNode & aSplitNode);

	/** If the tab has only one child pane and it is not central, replaces the tab in its parent with the child.
	Returns true if replaced, false if not. */
	bool tryReplaceTabWithOnlyChild(TabNode & aTabNode);

	/** Removes the specified pane from the hierarchy under root.
	Returns the pane's owning pointer. */
	std::unique_ptr<PaneNode> removePane(RootNode & aRoot, const std::string & aPaneId);

	/** Moves the specified pane as a tab in aTargetTabNode at the specified index.
	The pane is expected to already be in the root's hierarchy.
	If the insertion index is not valid, it is moved to the closest valid value.
	Returns true on success, false on failure (pane not found). */
	bool movePaneToTab(
		RootNode & aRoot,
		const std::string & aPaneId,
		TabNode & aTargetTabNode,
		int aInsertIndex
	);

	/** Moves the specified pane to the specified edge of the target node, using a SplitNode either already
	containing or newly replacing the target node.
	If the target is a PaneNode within a TabNode, the split is made above the TabNode.
	Basically this corresponds to the GUI operation "(nearest) dock to the side of this pane". */
	bool movePaneToNodeEdge(
		RootNode & aRoot,
		const std::string & aPaneId,
		BaseNode & aTargetNode,
		WxDockUI::DockPosition aEdge
	);

	/** Moves the source pane so that it makes a tab with the target pane.
	Returns true if moved, false on failure (pane not found). */
	bool mergePanesIntoTab(
		RootNode & aRoot,
		const std::string & aSourcePaneId,
		const std::string & aTargetPaneId,
		int aInsertIndex
	);

	/** Wraps the specified pane in a TabNode, if not already within a tab.
	Returns the TabNode containing the pane.
	Returns nullptr on error (pane not found). */
	TabNode * wrapPaneInTab(RootNode & aRoot, const std::string & aPaneId);

	/** Merges child SplitNodes that have the same orientation as aNode.
	Returns true if any modification was made. */
	bool mergeSameOrientationSplits(SplitNode & aSplitNode);

	/** Cleans the specified root's hierarchy. */
	void cleanup(RootNode & aRoot);

	/** Validates the layout tree.
	If a discrepancy is found, outputs to *aLog (if not nullptr).
	Returns true on success, false on failure. */
	bool validateLayoutTree(const RootNode & aRoot, std::ostream * aLog);





}  // namespace WxDockUI::Layout::Ops
