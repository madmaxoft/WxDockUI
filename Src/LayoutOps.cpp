#include <WxDockUI/Internal/LayoutOps.h>
#include <WxDockUI/Internal/Layout.h>

#include <cassert>





using namespace WxDockUI::Layout;





	namespace
	{

		/** Validates the invariants for a node, recursively.
		If a discrepancy is found, outputs to *aLog (if not nullptr).
		Returns true on success, false on failure. */
		bool validateNode(
			const BaseNode & aNode,
			const BaseNode * aExpectedParent,
			std::ostream * aLog
		)
		{
			if (aNode.parent() != aExpectedParent)
			{
				if (aLog != nullptr)
				{
					(*aLog) << "Parent mismatch\n";
				}
				return false;
			}

			switch (aNode.type())
			{
				case NodeType::Pane:
				{
					return true;
				}

				case NodeType::Tab:
				{
					const auto & tab = static_cast<const TabNode &>(aNode);
					const auto & panes = tab.panes();

					if (panes.empty())
					{
						if ((tab.activeIndex() != -1))
						{
							if (aLog != nullptr)
							{
								(*aLog) << "Empty TabNode must have activeIndex == -1\n";
							}
							return false;
						}
					}
					else
					{
						if ((tab.activeIndex() < 0) || (tab.activeIndex() >= static_cast<int>(panes.size())))
						{
							if (aLog != nullptr)
							{
								(*aLog) << "TabNode activeIndex out of range\n";
							}
							return false;
						}
					}

					for (const auto & pane: panes)
					{
						if (pane == nullptr)
						{
							if (aLog != nullptr)
							{
								(*aLog) << "TabNode contains null pane\n";
							}
							return false;
						}

						if (!validateNode(*pane, &aNode, aLog))
						{
							return false;
						}
					}

					return true;
				}

				case NodeType::Split:
				{
					const auto & split = static_cast<const SplitNode &>(aNode);
					const auto & children = split.children();

					if (children.empty())
					{
						if (aLog != nullptr)
						{
							(*aLog) << "SplitNode has no children\n";
						}
						return false;
					}

					for (const auto & ch: children)
					{
						if (ch.mNode == nullptr)
						{
							if (aLog != nullptr)
							{
								(*aLog) << "SplitNode contains null child\n";
							}
							return false;
						}

						auto * childSplit = ch.mNode->asSplitNode();
						if (
							(childSplit != nullptr) &&
							(childSplit->orientation() == split.orientation())
						)
						{
							if (aLog != nullptr)
							{
								(*aLog) << "Nested same-orientation SplitNode detected\n";
							}
							return false;
						}

						if (!validateNode(*ch.mNode, &aNode, aLog))
						{
							return false;
						}
					}

					return true;
				}

				case NodeType::Root:
				{
					const auto & root = static_cast<const RootNode &>(aNode);

					if (root.child() == nullptr)
					{
						return true;
					}

					if (root.child()->type() == NodeType::Root)
					{
						if (aLog != nullptr)
						{
							(*aLog) << "RootNode cannot contain RootNode\n";
						}
						return false;
					}

					return validateNode(*root.child(), &aNode, aLog);
				}
			}

			return false;
		}

	} // anonymous namespace





namespace WxDockUI::Layout::Ops
{




	/** Recursively searches for a PaneNode or TabNode with a Center intendedDockPos.
	If none found, returns nullptr. */
	static BaseNode * findCenterTarget(const BaseNode & aNode)
	{
		switch (aNode.type())
		{
			case NodeType::Root:
			{
				return findCenterTarget(*aNode.asRootNode()->child());
			}
			case NodeType::Split:
			{
				for (const auto & ch: aNode.asSplitNode()->children())
				{
					auto res = findCenterTarget(*ch.mNode);
					if (res != nullptr)
					{
						return res;
					}
				}
				return nullptr;
			}
			case NodeType::Tab:
			{
				auto tabNode = aNode.asTabNode();
				if (tabNode->panes().empty())
				{
					// An empty TabNode is considered a Center node:
					return const_cast<BaseNode *>(&aNode);
				}
				return findCenterTarget(*tabNode->pane(0));
			}
			case NodeType::Pane:
			{
				if (aNode.asPaneNode()->intendedDockPos() == DockPosition::Center)
				{
					return const_cast<BaseNode *>(&aNode);
				}
				return nullptr;
			}
		}
		return nullptr;
	}





	void insertCenterPane(
		RootNode & aRoot,
		std::unique_ptr<PaneNode> aPaneNode
	)
	{
		assert(aPaneNode != nullptr);

		// Empty root
		if (aRoot.child() == nullptr)
		{
			aPaneNode->setIntendedDockPos(DockPosition::Center);
			aRoot.setChild(std::move(aPaneNode));
			return;
		}

		auto center = findCenterTarget(*aRoot.child());
		if (center == nullptr)
		{
			assert(!"Unable to insert center pane, cannot find a target");
			return;
		}

		// If the center target is already wrapped in a Tab, use the tab as the target:
		if (center->parent() != nullptr)
		{
			if (
				(center->type() == NodeType::Pane) &&
				(center->parent()->type() == NodeType::Tab)
			)
			{
				center = center->parent();
			}
		}

		switch (center->type())
		{
			case NodeType::Pane:
			{
				// Extract the pane from the parent:
				auto * parent = center->parent();
				std::unique_ptr<BaseNode> extractedNode;
				if ((parent == nullptr) || (parent->type() == NodeType::Root))
				{
					extractedNode = aRoot.setChild(nullptr);
				}
				else if (parent->type() == NodeType::Split)
				{
					auto & split = static_cast<SplitNode &>(*parent);
					extractedNode = split.removeChild(center);
				}
				else
				{
					assert(!"Pane parent must be Root or Split");  // We checked for Tab above
				}
				auto oldPane = std::unique_ptr<PaneNode>(static_cast<PaneNode *>(extractedNode.release()));

				// Insert a tab in place of the original pane:
				assert(oldPane->intendedDockPos() == DockPosition::Center);
				aPaneNode->setIntendedDockPos(DockPosition::Center);
				auto tab = std::make_unique<TabNode>();
				tab->insertPane(std::move(oldPane), 0);
				tab->insertPane(std::move(aPaneNode), 1);
				tab->setActiveIndex(1);
				if ((parent == nullptr) || (parent->type() == NodeType::Root))
				{
					aRoot.setChild(std::move(tab));
				}
				else
				{
					auto & split = static_cast<SplitNode &>(*parent);
					split.insertChild(std::move(tab), 1.0f, 0);
				}
				return;
			}
			case NodeType::Tab:
			{
				aPaneNode->setIntendedDockPos(DockPosition::Center);
				auto tab = center->asTabNode();
				tab->insertPane(std::move(aPaneNode), tab->panes().size());
				tab->setActiveIndex(tab->panes().size() - 1);
				return;
			}
			default:
			{
				assert(!"Unknown center node type");
			}
		}
	}





	void insertEdgePane(
		RootNode & aRoot,
		std::unique_ptr<PaneNode> aPaneNode,
		DockPosition aPosition
	)
	{
		auto oldChild = aRoot.setChild(nullptr);

		// Empty layout:
		if (oldChild == nullptr)
		{
			aPaneNode->setIntendedDockPos(aPosition);
			aRoot.setChild(std::move(aPaneNode));
			return;
		}

		// If there's already a split in the correct orientation, reuse it:
		if (oldChild->type() == NodeType::Split)
		{
			const auto desiredOrientation =
				((aPosition == DockPosition::Left) || (aPosition == DockPosition::Right))
				? SplitOrientation::Horizontal
				: SplitOrientation::Vertical;
			auto split = static_cast<SplitNode *>(oldChild.get());
			if (split->orientation() == desiredOrientation)
			{
				// Insert at the correct end:
				aPaneNode->setIntendedDockPos(aPosition);
				const size_t index = ((aPosition == DockPosition::Left) || (aPosition == DockPosition::Top))
					? 0
					: split->children().size();
				split->insertChild(std::move(aPaneNode), split->sumRatios() / 4, index);
				aRoot.setChild(std::move(oldChild));
				return;
			}
		}

		// Decide on the splitter orientation and new pane's position:
		SplitOrientation orientation;
		bool paneFirst = false;
		switch (aPosition)
		{
			case DockPosition::Left:
			{
				orientation = SplitOrientation::Horizontal;
				paneFirst = true;
				break;
			}
			case DockPosition::Right:
			{
				orientation = SplitOrientation::Horizontal;
				paneFirst = false;
				break;
			}
			case DockPosition::Top:
			{
				orientation = SplitOrientation::Vertical;
				paneFirst = true;
				break;
			}
			case DockPosition::Bottom:
			{
				orientation = SplitOrientation::Vertical;
				paneFirst = false;
				break;
			}
			default:
			{
				assert(!"Invalid edge docking position");
				aRoot.setChild(std::move(oldChild));
				return;
			}
		}

		// Create the split:
		auto split = std::make_unique<SplitNode>(orientation);
		aPaneNode->setIntendedDockPos(aPosition);
		if (paneFirst)
		{
			split->insertChild(std::move(aPaneNode), 1.0f, 0);
			split->insertChild(std::move(oldChild), 4.0f, 1);
		}
		else
		{
			split->insertChild(std::move(oldChild), 4.0f, 0);
			split->insertChild(std::move(aPaneNode), 1.0f, 1);
		}
		aRoot.setChild(std::move(split));
	}





	const PaneNode * findPaneNodeRecursive(const BaseNode & aNode, const std::string & aPaneId)
	{
		switch (aNode.type())
		{
			case NodeType::Root:
			{
				return findPaneNodeRecursive(*static_cast<const RootNode &>(aNode).child(), aPaneId);
			}
			case NodeType::Split:
			{
				auto splitNode = aNode.asSplitNode();
				for (const auto & ch: splitNode->children())
				{
					auto res = findPaneNodeRecursive(*ch.mNode, aPaneId);
					if (res != nullptr)
					{
						return res;
					}
				}
				return nullptr;
			}
			case NodeType::Tab:
			{
				auto tabNode = aNode.asTabNode();
				for (const auto & pane: tabNode->panes())
				{
					if (pane->paneId() == aPaneId)
					{
						return pane.get();
					}
				}
				return nullptr;
			}
			case NodeType::Pane:
			{
				if (aNode.asPaneNode()->paneId() == aPaneId)
				{
					return aNode.asPaneNode();
				}
				return nullptr;
			}
		}
		return nullptr;
	}





	bool tryReplaceSplitWithOnlyChild(SplitNode & aSplitNode)
	{
		if (aSplitNode.children().size() != 1)
		{
			return false;
		}

		auto parent = aSplitNode.parent();
		auto remaining = aSplitNode.removeChild(0);
		switch (parent->type())
		{
			case NodeType::Split:
			{
				auto parentSplit = parent->asSplitNode();
				parentSplit->replaceChild(&aSplitNode, std::move(remaining));
				return true;
			}
			case NodeType::Root:
			{
				auto root = static_cast<RootNode *>(parent);
				root->setChild(std::move(remaining));
				return true;
			}
			default:
			{
				assert(!"Invalid parent type");
				return false;
			}
		}
	}





	std::unique_ptr<PaneNode> removePane(RootNode & aRoot, const std::string & aPaneId)
	{
		// Find the pane node:
		if (aRoot.child() == nullptr)
		{
			return nullptr;
		}
		auto pane = findPaneNodeRecursive(*aRoot.child(), aPaneId);
		if (pane == nullptr)
		{
			return nullptr;
		}

		// Remove from parent:
		BaseNode * parent = pane->parent();
		if (parent == nullptr)
		{
			assert(!"Pane has no parent, this should not happen");
			return nullptr;
		}
		switch (parent->type())
		{
			case NodeType::Tab: return parent->asTabNode()->removePane(pane);
			case NodeType::Root:
			{
				auto res = parent->asRootNode()->setChild(nullptr);
				return std::unique_ptr<PaneNode>(static_cast<PaneNode *>(res.release()));
			}
			case NodeType::Split:
			{
				auto splitNode = parent->asSplitNode();
				auto oldPane = splitNode->removeChild(pane);
				tryReplaceSplitWithOnlyChild(*splitNode);
				return std::unique_ptr<PaneNode>(static_cast<PaneNode *>(oldPane.release()));
			}
			default:
			{
				assert(!"Invalid parent node type");
				return nullptr;
			}
		}
	}





	bool movePaneToTab(
		RootNode & aRoot,
		const std::string & aPaneId,
		TabNode & aTargetTabNode,
		int aInsertIndex
	)
	{
		auto removed = removePane(aRoot, aPaneId);
		if (removed == nullptr)
		{
			return false;
		}

		aTargetTabNode.insertPane(std::move(removed), aInsertIndex);
		return true;
	}





	bool movePaneToNodeEdge(
		RootNode & aRoot,
		const std::string & aPaneId,
		BaseNode & aTargetNode,
		WxDockUI::DockPosition aEdge
	)
	{
		auto removed = removePane(aRoot, aPaneId);
		if (removed == nullptr)
		{
			return false;
		}

		auto orientation = orientationForEdge(aEdge);
		auto parent = aTargetNode.parent();
		assert(parent != nullptr);  // Cannot replace the root itself

		// If parent is a TabNode, operate on the TabNode itself instead
		BaseNode * effectiveTarget = &aTargetNode;
		if (parent->type() == NodeType::Tab)
		{
			// Move up: we want to split the tab container, not the pane inside it
			effectiveTarget = parent;
			parent = parent->parent();
		}

		// Check if parent is a compatible split we can reuse
		auto parentSplit = parent->asSplitNode();
		if ((parentSplit != nullptr) && (parentSplit->orientation() == orientation))
		{
			// Find the effective target's index in the parent
			int targetIndex = -1;
			for (size_t i = 0; i < parentSplit->children().size(); ++i)
			{
				if (parentSplit->children()[i].mNode.get() == effectiveTarget)
				{
					targetIndex = static_cast<int>(i);
					break;
				}
			}
			assert(targetIndex != -1);

			// Insert before or after the target
			int insertIndex = targetIndex;
			if ((aEdge == DockPosition::Right) || (aEdge == DockPosition::Bottom))
			{
				insertIndex = targetIndex + 1;
			}

			parentSplit->insertChild(std::move(removed), 1.0f, insertIndex);
			return true;
		}

		// Parent is incompatible (wrong orientation or not a split)
		// Create a new split and replace the effective target with it
		auto newSplit = std::make_unique<SplitNode>(orientation);
		auto newSplitRaw = newSplit.get();

		// Extract the effective target from its parent
		std::unique_ptr<BaseNode> targetOwnership;
		auto parentSplitNode = parent->asSplitNode();
		if (parentSplitNode != nullptr)
		{
			targetOwnership = parentSplitNode->replaceChild(effectiveTarget, std::move(newSplit));

			// Now insert the effective target and removed pane into the new split
			if ((aEdge == DockPosition::Left) || (aEdge == DockPosition::Top))
			{
				newSplitRaw->insertChild(std::move(removed), 1.0f, 0);
				newSplitRaw->insertChild(std::move(targetOwnership), 1.0f, 1);
			}
			else
			{
				newSplitRaw->insertChild(std::move(targetOwnership), 1.0f, 0);
				newSplitRaw->insertChild(std::move(removed), 1.0f, 1);
			}
		}
		else
		{
			// Parent must be root
			assert(parent->asRootNode() != nullptr);
			targetOwnership = static_cast<RootNode *>(parent)->setChild(nullptr);

			// Insert effective target and removed pane into the split
			if ((aEdge == DockPosition::Left) || (aEdge == DockPosition::Top))
			{
				newSplit->insertChild(std::move(removed), 1.0f, 0);
				newSplit->insertChild(std::move(targetOwnership), 1.0f, 1);
			}
			else
			{
				newSplit->insertChild(std::move(targetOwnership), 1.0f, 0);
				newSplit->insertChild(std::move(removed), 1.0f, 1);
			}

			static_cast<RootNode *>(parent)->setChild(std::move(newSplit));
		}
		return true;
	}





	bool mergePanesIntoTab(
		RootNode & aRoot,
		const std::string & aToMovePaneId,
		const std::string & aTargetPaneId,
		int aInsertIndex
	)
	{
		// Check inputs:
		if (aToMovePaneId == aTargetPaneId)
		{
			// Moving onto self, do nothing:
			return false;
		}
		if (findPaneNodeRecursive(aRoot, aTargetPaneId) == nullptr)
		{
			// Target pane not found, do nothing:
			return false;
		}
		auto toMovePane = removePane(aRoot, aToMovePaneId);
		if (toMovePane == nullptr)
		{
			// ToMove pane not found, do nothing:
			return false;
		}

		// Wrap the target into a TabNode, if needed:
		auto targetTab = wrapPaneInTab(aRoot, aTargetPaneId);
		assert(targetTab != nullptr);

		// Copy the intended dock pos from the target pane
		assert(targetTab->panes().size() >= 1);
		auto targetPane = targetTab->pane(0);
		assert(targetPane != nullptr);
		toMovePane->setIntendedDockPos(targetPane->intendedDockPos());

		targetTab->insertPane(std::move(toMovePane), aInsertIndex);
		return true;
	}





	TabNode * wrapPaneInTab(RootNode & aRoot, const std::string & aPaneId)
	{
		// Find the pane:
		auto paneNode = findPaneNodeRecursive(aRoot, aPaneId);
		if (paneNode == nullptr)
		{
			return nullptr;
		}
		auto * parent = paneNode->parent();

		// Wrap in a new TabNode, if needed:
		switch (parent->type())
		{
			case NodeType::Root:
			{
				auto * root = parent->asRootNode();
				auto ownedPane = root->setChild(nullptr);
				auto newTab = std::make_unique<TabNode>();
				newTab->insertPane(std::unique_ptr<PaneNode>(static_cast<PaneNode *>(ownedPane.release())), 0);
				root->setChild(std::move(newTab));
				return root->child()->asTabNode();
			}
			case NodeType::Split:
			{
				auto * split = parent->asSplitNode();
				auto newTab = std::make_unique<TabNode>();
				auto newTabRaw = newTab.get();
				auto ownedPane = split->replaceChild(paneNode, std::move(newTab));
				newTabRaw->insertPane(std::unique_ptr<PaneNode>(static_cast<PaneNode *>(ownedPane.release())), 0);
				return newTabRaw;
			}
			case NodeType::Tab:
			{
				return parent->asTabNode();
			}
			default:
			{
				assert(!"Invalid parent type");
				return nullptr;
			}
		}
	}





	/** Merges child SplitNodes that have the same orientation as aNode.
	Returns true if any modification was made. */
	bool mergeSameOrientationSplits(SplitNode & aSplitNode)
	{
		bool hasChanged = false;
		auto & children = const_cast<std::vector<SplitNode::SplitChild> &>(aSplitNode.children());
		for (size_t i = 0; i < children.size();)  // Do not increment i here — we re-evaluate absorbed children as well
		{
			auto * childSplit = children[i].mNode->asSplitNode();
			if ((childSplit == nullptr) || (childSplit->orientation() != aSplitNode.orientation()))
			{
				++i;
				continue;
			}

			// Absorb this split:
			auto parentRatio = children[i].mRatio;
			auto ownedChildNode = aSplitNode.removeChild(static_cast<int>(i));
			auto * absorbedSplit = ownedChildNode->asSplitNode();
			assert(absorbedSplit != nullptr);
			auto absorbedChildren = std::move(
				const_cast<std::vector<SplitNode::SplitChild> &>(absorbedSplit->children())
			);
			size_t insertPos = i;
			for (auto & grandChild: absorbedChildren)
			{
				float newRatio = parentRatio * grandChild.mRatio;
				aSplitNode.insertChild(std::move(grandChild.mNode), newRatio, insertPos);
				++insertPos;
			}
			hasChanged = true;
		}

		return hasChanged;
	}





	static void cleanNode(BaseNode * aNode)
	{
		if (aNode == nullptr)
		{
			return;
		}

		switch (aNode->type())
		{
			case NodeType::Root:
			{
				auto * root = aNode->asRootNode();
				cleanNode(root->child());
				return;
			}
			case NodeType::Split:
			{
				auto * split = aNode->asSplitNode();

				// First clean children
				for (auto & child: split->children())
				{
					cleanNode(child.mNode.get());
				}

				// Merge compatible nested splits
				mergeSameOrientationSplits(*split);

				// Collapse single-child split
				tryReplaceSplitWithOnlyChild(*split);

				return;
			}
			case NodeType::Tab:
			{
				// TabNode contains only PaneNodes — nothing structural to clean
				return;
			}
			case NodeType::Pane:
			{
				return;
			}
		}
	}





	void cleanup(RootNode & aRoot)
	{
		// Run multiple passes until no structural changes occur.
		// This avoids dependency on traversal order.
		for (;;)
		{
			auto before = aRoot.child();
			cleanNode(&aRoot);
			if (before == aRoot.child())
			{
				break;
			}
		}
	}




	bool validateLayoutTree(const RootNode & aRoot, std::ostream * aLog)
	{
		return validateNode(aRoot, nullptr, aLog);
	}





}  // namespace WxDockUI::Layout::Ops
