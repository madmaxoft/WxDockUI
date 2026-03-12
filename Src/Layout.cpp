#include <WxDockUI/Internal/Layout.h>
#include <algorithm>
#include <cassert>
#include <iostream>





namespace WxDockUI::Layout
{





	///////////////////////////////////////////////////////////////////////////////
	// BaseNode:

	BaseNode::BaseNode(NodeType aType):
		mType(aType)
	{
	}





	void BaseNode::setParent(BaseNode * aParent)
	{
		mParent = aParent;
	}





	const RootNode * BaseNode::asRootNode() const
	{
		if (mType == NodeType::Root)
		{
			return static_cast<const RootNode *>(this);
		}
		return nullptr;
	}





	const SplitNode * BaseNode::asSplitNode() const
	{
		if (mType == NodeType::Split)
		{
			return static_cast<const SplitNode *>(this);
		}
		return nullptr;
	}





	const TabNode * BaseNode::asTabNode() const
	{
		if (mType == NodeType::Tab)
		{
			return static_cast<const TabNode *>(this);
		}
		return nullptr;
	}





	const PaneNode * BaseNode::asPaneNode() const
	{
		if (mType == NodeType::Pane)
		{
			return static_cast<const PaneNode *>(this);
		}
		return nullptr;
	}





	RootNode * BaseNode::asRootNode()
	{
		if (mType == NodeType::Root)
		{
			return static_cast<RootNode *>(this);
		}
		return nullptr;
	}





	SplitNode * BaseNode::asSplitNode()
	{
		if (mType == NodeType::Split)
		{
			return static_cast<SplitNode *>(this);
		}
		return nullptr;
	}





	TabNode * BaseNode::asTabNode()
	{
		if (mType == NodeType::Tab)
		{
			return static_cast<TabNode *>(this);
		}
		return nullptr;
	}





	PaneNode * BaseNode::asPaneNode()
	{
		if (mType == NodeType::Pane)
		{
			return static_cast<PaneNode *>(this);
		}
		return nullptr;
	}





	void BaseNode::indent(std::ostream & aOut, int aIndent)
	{
		for (int i = 0; i < aIndent; ++i)
		{
			aOut << "\t";
		}
	}





	///////////////////////////////////////////////////////////////////////////////
	// PaneNode:

	PaneNode::PaneNode(const std::string & aPaneId, WxDockUI::DockPosition aIntendedDockPos):
		BaseNode(NodeType::Pane),
		mPaneId(aPaneId),
		mIntendedDockPos(aIntendedDockPos)
	{
		#ifdef WXDOCKUI_DEBUG_LIFETIME
			std::cout << "Created a PaneNode at " << this << ", id " << mPaneId << std::endl;
		#endif
	}





	PaneNode::PaneNode(const std::string & aPaneId):
		PaneNode(aPaneId, WxDockUI::DockPosition::Floating)
	{
		#ifdef WXDOCKUI_DEBUG_LIFETIME
			std::cout << "Created a PaneNode at " << this << ", id " << mPaneId << std::endl;
		#endif
	}





	PaneNode::~PaneNode()
	{
		#ifdef WXDOCKUI_DEBUG_LIFETIME
			std::cout << "Deleting a PaneNode at " << this << ", id " << mPaneId << std::endl;
		#endif
	}





	void PaneNode::dump(std::ostream & aOut, int aIndent) const
	{
		indent(aOut, aIndent);
		aOut << "Pane(\"" << mPaneId << "\")";
	}





	///////////////////////////////////////////////////////////////////////////////
	// TabNode:

	TabNode::TabNode():
		BaseNode(NodeType::Tab),
		mActiveIndex(-1)
	{
	}





	void TabNode::insertPane(std::unique_ptr<PaneNode> aPaneNode, int aIndex)
	{
		// Check params:
		if (aPaneNode == nullptr)
		{
			throw std::runtime_error("TabNode::addPane() received a null pane");
		}
		if (aIndex < 0)
		{
			aIndex = 0;
		}
		else if (aIndex > static_cast<int>(mPanes.size()))
		{
			aIndex = static_cast<int>(mPanes.size());
		}

		// Insert:
		aPaneNode->setParent(this);
		mPanes.insert(mPanes.begin() + aIndex, std::move(aPaneNode));

		// Fix active index:
		if (mActiveIndex < 0)
		{
			mActiveIndex = 0;
		}
		else if (mActiveIndex >= aIndex)
		{
			mActiveIndex += 1;
		}
	}





	std::unique_ptr<PaneNode> TabNode::removePane(int aIndex)
	{
		// Check index for validity:
		if ((aIndex < 0) || (aIndex >= static_cast<int>(mPanes.size())))
		{
			return nullptr;
		}

		auto removed = std::move(mPanes[aIndex]);
		mPanes.erase(mPanes.begin() + aIndex);
		if (mPanes.empty())
		{
			mActiveIndex = -1;
		}
		else if (mActiveIndex >= aIndex)
		{
			mActiveIndex = std::max(0, mActiveIndex - 1);
		}
		removed->setParent(nullptr);
		return removed;
	}





	std::unique_ptr<PaneNode> TabNode::removePane(const PaneNode * aPane)
	{
		// Find the pane in mPanes:
		auto itr = std::find_if(
			mPanes.begin(),
			mPanes.end(),
			[aPane](const std::unique_ptr<PaneNode> & aPtr)
			{
				return (aPtr.get() == aPane);
			}
		);
		if (itr == mPanes.end())
		{
			return nullptr;
		}

		return removePane(static_cast<int>(std::distance(mPanes.begin(), itr)));
	}





	void TabNode::setActiveIndex(int aIndex)
	{
		// If no panes, always stay with "invalid" index:
		if (mPanes.empty())
		{
			mActiveIndex = -1;
			return;
		}

		// Ignore out-of-bounds:
		if ((aIndex < 0) || (aIndex >= static_cast<int>(mPanes.size())))
		{
			return;
		}

		mActiveIndex = aIndex;
	}





	void TabNode::dump(std::ostream & aOut, int aIndent) const
	{
		indent(aOut, aIndent);
		aOut << "Tab({  // " << mPanes.size() << " panes\n";

		for (const auto & pane: mPanes)
		{
			indent(aOut, aIndent + 1);
			aOut << "\"" << pane->paneId() << "\",\n";
		}
		indent(aOut, aIndent);
		aOut << "})";
	}





	///////////////////////////////////////////////////////////////////////////////
	// SplitNode:

	SplitNode::SplitNode(SplitOrientation aOrientation):
		BaseNode(NodeType::Split),
		mOrientation(aOrientation)
	{
	}





	void SplitNode::insertChild(
		std::unique_ptr<BaseNode> aChild,
		float aRatio,
		size_t aIndex
	)
	{
		// Check params:
		if (aChild == nullptr)
		{
			throw std::runtime_error("SplitNode::insertChild() received a null child");
		}
		if (aIndex > mChildren.size())
		{
			aIndex = mChildren.size();
		}

		// Insert:
		aChild->setParent(this);
		SplitChild child;
		child.mNode = std::move(aChild);
		child.mRatio = aRatio;
		mChildren.insert(mChildren.begin() + aIndex, std::move(child));
	}





	std::unique_ptr<BaseNode> SplitNode::removeChild(int aIndex)
	{
		// Check params:
		if ((aIndex < 0) || (aIndex >= static_cast<int>(mChildren.size())))
		{
			return nullptr;
		}

		auto removed = std::move(mChildren[aIndex].mNode);
		mChildren.erase(mChildren.begin() + aIndex);
		removed->setParent(nullptr);
		return removed;
	}





	std::unique_ptr<BaseNode> SplitNode::removeChild(const BaseNode * aNode)
	{
		// Find the child in mChildren:
		auto itr = std::find_if(
			mChildren.begin(),
			mChildren.end(),
			[aNode](const SplitChild & aChild)
			{
				return (aChild.mNode.get() == aNode);
			}
		);
		if (itr == mChildren.end())
		{
			return nullptr;
		}

		return removeChild(static_cast<int>(std::distance(mChildren.begin(), itr)));
	}





	std::unique_ptr<BaseNode> SplitNode::replaceChild(const BaseNode * aNode, std::unique_ptr<BaseNode> aReplacement)
	{
		for (auto & ch: mChildren)
		{
			if (ch.mNode.get() == aNode)
			{
				auto oldNode = std::move(ch.mNode);
				ch.mNode = std::move(aReplacement);
				oldNode->setParent(nullptr);
				ch.mNode->setParent(this);
				return oldNode;
			}
		}
		return nullptr;
	}





	void SplitNode::dump(std::ostream & aOut, int aIndent) const
	{
		indent(aOut, aIndent);

		aOut << ((mOrientation == SplitOrientation::Horizontal) ? "HorzSplit" : "VertSplit");
		aOut << "({  // " << mChildren.size() << " children\n";

		for (const auto & ch: mChildren)
		{
			ch.mNode->dump(aOut, aIndent + 1);
			aOut << ",\n";
		}
		indent(aOut, aIndent);
		aOut << "})";
	}





	///////////////////////////////////////////////////////////////////////////////
	// RootNode:

	RootNode::RootNode():
		BaseNode(NodeType::Root)
	{
	}





	std::unique_ptr<BaseNode> RootNode::setChild(std::unique_ptr<BaseNode> aChild)
	{
		if (aChild != nullptr)
		{
			aChild->setParent(this);
		}
		auto res = std::move(mChild);
		mChild = std::move(aChild);
		if (res != nullptr)
		{
			res->setParent(nullptr);
		}
		return res;
	}





	BaseNode * RootNode::walkSplits(const std::vector<size_t> & aSplitIndices)
	{
		BaseNode * res = mChild.get();
		for (const auto & idx: aSplitIndices)
		{
			auto split = res->asSplitNode();
			if (split == nullptr)
			{
				return nullptr;
			}
			res = split->child(idx);
		}
		return res;
	}





	void RootNode::dump(std::ostream & aOut, int aIndent) const
	{
		if (mChild == nullptr)
		{
			indent(aOut, aIndent);
			aOut << "Root()\n";
			return;
		}
		indent(aOut, aIndent);
		aOut << "Root(\n";
		mChild->dump(aOut, aIndent + 1);
		aOut << "\n";
		indent(aOut, aIndent);
		aOut << ");\n";
	}





	///////////////////////////////////////////////////////////////////////////////
	// FloatingFrame:

	FloatingFrame::FloatingFrame(const std::string & aFrameId):
		mFrameId(aFrameId)
	{
	}





	void FloatingFrame::setGeometry(int aX, int aY, int aWidth, int aHeight)
	{
		mX = aX;
		mY = aY;
		mWidth = aWidth;
		mHeight = aHeight;
	}





	///////////////////////////////////////////////////////////////////////////////
	// Globals:

	SplitOrientation orientationForEdge(DockPosition aEdge)
	{
		switch (aEdge)
		{
			case DockPosition::Left:
			case DockPosition::Right:
			{
				return SplitOrientation::Horizontal;
			}
			case DockPosition::Top:
			case DockPosition::Bottom:
			{
				return SplitOrientation::Vertical;
			}
			default:
			{
				assert(!"Invalid DockPosition for a splitter");
				return SplitOrientation::Horizontal;
			}
		}
	}




}  // namespace WxDockUI::Layout
