#pragma once

#include <string>
#include <vector>
#include <memory>
#include <ostream>

#include "WxDockUI/Enums.h"





namespace WxDockUI::Layout
{





	/** Specifies the type of the layout node.
	Used to avoid dynamic_cast from base class to descendant. */
	enum class NodeType
	{
		Root,
		Split,
		Tab,
		Pane
	};





	/** Specifies the direction of a SplitNode. */
	enum class SplitOrientation
	{
		Horizontal,
		Vertical
	};





	// fwd:
	class SplitNode;
	class TabNode;
	class PaneNode;
	class RootNode;





	/** The base class for all layout nodes. */
	class BaseNode
	{
		NodeType mType;
		BaseNode * mParent = nullptr;

		// Delete copy-constructors:
		BaseNode(const BaseNode &) = delete;
		BaseNode & operator = (const BaseNode &) = delete;

		// Default move-constructors:
		BaseNode(BaseNode &&) = default;
		BaseNode & operator = (BaseNode &&) = default;


	public:

		explicit BaseNode(NodeType aType);
		virtual ~BaseNode() = default;

		NodeType type() const { return mType; }

		BaseNode * parent() const { return mParent; }
		void setParent(BaseNode * aParent);

		/** Dumps the node and its children to the specified ostream with the specified base indent.
		Note that the format is directly usable to build a layout tree in a test in LayoutOpsTest.cpp. */
		virtual void dump(std::ostream & aOut, int aIndent) const = 0;

		/** Returns this if this is a RootNode instance, otherwise returns nullptr. */
		const RootNode * asRootNode() const;

		/** Returns this if this is a SplitNode instance, otherwise returns nullptr. */
		const SplitNode * asSplitNode() const;

		/** Returns this if this is a TabNode instance, otherwise returns nullptr. */
		const TabNode * asTabNode() const;

		/** Returns this if this is a PaneNode instance, otherwise returns nullptr. */
		const PaneNode * asPaneNode() const;

		/** Returns this if this is a RootNode instance, otherwise returns nullptr. */
		RootNode * asRootNode();

		/** Returns this if this is a SplitNode instance, otherwise returns nullptr. */
		SplitNode * asSplitNode();

		/** Returns this if this is a TabNode instance, otherwise returns nullptr. */
		TabNode * asTabNode();

		/** Returns this if this is a PaneNode instance, otherwise returns nullptr. */
		PaneNode * asPaneNode();


	protected:

		/** Outputs the specified amount of indent to the ostream. */
		static void indent(std::ostream & aOut, int aIndent);
	};





	/** The layout node containing a single pane. */
	class PaneNode final:
		public BaseNode
	{
		/** Unique identification of the pane within the entire DockSystem. */
		std::string mPaneId;

		/** The last intended docking position.
		Only set initially and updated when the user re-docks the pane somewhere,
		or the layout is restored from serialization. */
		WxDockUI::DockPosition mIntendedDockPos;


	public:

		/** Creates a fully-specified new instance. */
		PaneNode(const std::string & aPaneId, WxDockUI::DockPosition aIntendedDockPos);

		/** Creates a new instance with the specified ID and Floating intended dock. */
		explicit PaneNode(const std::string & aPaneId);

		~PaneNode();

		// Getters / setters:
		const std::string & paneId() const { return mPaneId; }
		WxDockUI::DockPosition intendedDockPos() const { return mIntendedDockPos; }
		void setIntendedDockPos(WxDockUI::DockPosition aIntendedDockPos) { mIntendedDockPos = aIntendedDockPos; }

		// BaseNode override:
		virtual void dump(std::ostream & aOut, int aIndent) const override;
	};





	/** The layout node representing a tab-switcher of panes.
	NOTE: A SplitNode can contain a TabNode, but a TabNode CANNOT contain a SplitNode. */
	class TabNode final:
		public BaseNode
	{
		std::vector<std::unique_ptr<PaneNode>> mPanes;
		int mActiveIndex = -1;


	public:

		TabNode();

		/** Inserts the specified pane at the specified index.
		If the index is out of bounds, inserts at the end.*/
		void insertPane(std::unique_ptr<PaneNode> aPaneNode, int aIndex);

		/** Removes the pane at the specified index and returns its owning ptr.
		Returns an empty ptr if no such child. */
		std::unique_ptr<PaneNode> removePane(int aIndex);

		/** Removes the specified pane and returns its owning ptr.
		Returns an empty ptr if no such child. */
		std::unique_ptr<PaneNode> removePane(const PaneNode * aPane);

		/** Sets the currently active index.
		Ignored if index is out of bounds. */
		void setActiveIndex(int aIndex);

		// Getters:
		const std::vector<std::unique_ptr<PaneNode>> & panes() const { return mPanes; }
		int activeIndex() const { return mActiveIndex; }
		const PaneNode * pane(size_t aIndex) const { return mPanes[aIndex].get(); }
		const PaneNode * activePane() const { return mPanes[mActiveIndex].get(); }

		// BaseNode override:
		virtual void dump(std::ostream & aOut, int aIndent) const override;
	};





	/** The layout node representing a single-direction-split of child layout nodes.
	NOTE: A SplitNode can contain a TabNode, but a TabNode CANNOT contain a SplitNode. */
	class SplitNode final:
		public BaseNode
	{
	public:

		/** Representation for a single child in the split - the node it contains and the ratio. */
		struct SplitChild
		{
			std::unique_ptr<BaseNode> mNode;
			float mRatio;
		};

	private:

		SplitOrientation mOrientation;
		std::vector<SplitChild> mChildren;


	public:

		explicit SplitNode(SplitOrientation aOrientation);

		/** Adds the specified child node with the specified ratio at the specified index of the children. */
		void insertChild(std::unique_ptr<BaseNode> aChild, float aRatio, size_t aIndex);

		/** Removes the node at the specified index from the children and returns its owning pointer.
		Returns an empty pointer if no such child found. */
		std::unique_ptr<BaseNode> removeChild(int aIndex);

		/** Removes the specified node from the children and returns its owning pointer.
		Returns an empty pointer if no such child found. */
		std::unique_ptr<BaseNode> removeChild(const BaseNode * aNode);

		/** Replaces the specified child node with the specified replacement.
		Returns the old node's owning pointer. */
		std::unique_ptr<BaseNode> replaceChild(const BaseNode * aNode, std::unique_ptr<BaseNode> aReplacement);

		// Getters:
		const std::vector<SplitChild> & children() const { return mChildren; }
		BaseNode * child(size_t aIndex) const { return mChildren[aIndex].mNode.get(); }
		SplitOrientation orientation() const { return mOrientation; }

		// BaseNode override:
		virtual void dump(std::ostream & aOut, int aIndent) const override;
	};





	/** Root of the layout nodes tree, usually attached to a real wxWidget object. */
	class RootNode final:
		public BaseNode
	{
		std::unique_ptr<BaseNode> mChild;


	public:

		RootNode();

		BaseNode * child() const { return mChild.get(); }

		/** Replaces the current child with the specified one, returning the previous child. */
		std::unique_ptr<BaseNode> setChild(std::unique_ptr<BaseNode> aChild);

		/** Returns the node specified by its successive indices in the children splits.
		Eg. {0, 1} specifies "take the second child of the first child of the root split.
		Returns nullptr if the lookup fails at any point. */
		BaseNode * walkSplits(const std::vector<size_t> & aSplitIndices);

		// BaseNode override:
		virtual void dump(std::ostream & aOut, int aIndent) const override;
	};





	/** Descriptor of a floating frame (NOT a LayoutNode!) */
	class FloatingFrame
	{
		std::string mFrameId;
		RootNode mRoot;

		int mX = 0;
		int mY = 0;
		int mWidth = 800;
		int mHeight = 600;

	public:
		explicit FloatingFrame(const std::string & aFrameId);

		const std::string & frameId() const { return mFrameId; }

		const RootNode * root() const { return &mRoot; }
		RootNode * root() { return &mRoot; }

		int x() const { return mX; }
		int y() const { return mY; }
		int width() const { return mWidth; }
		int height() const { return mHeight; }

		void setGeometry(int aX, int aY, int aWidth, int aHeight);
	};





	/** Converts the dock position into the splitter orientation that is required for docking at that edge. */
	SplitOrientation orientationForEdge(WxDockUI::DockPosition aEdge);





}  // namespace WxDockUI::Internal
