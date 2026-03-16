#include <WxDockUI/Internal/SplitContainer.h>

#include <WxDockUI/FrameDockManager.h>
#include <WxDockUI/Internal/Layout.h>





namespace WxDockUI::Internal
{





	static constexpr int SPLITTER_SIZE = 8;





	SplitContainer::SplitContainer(
		FrameDockManager & aFrameDockManager,
		wxWindow * aParent,
		Layout::SplitNode & aSplitNode
	):
		Super(aParent, wxID_ANY),
		mFrameDockManager(aFrameDockManager),
		mSplitNode(aSplitNode)
	{
		#ifdef WXDOCKUI_DEBUG_LIFETIME
			std::cout << "Created a SplitContainer at " << this << " with " << aSplitNode.children().size() << " children." << std::endl;
		#endif
	}





	SplitContainer::~SplitContainer()
	{
		#ifdef WXDOCKUI_DEBUG_LIFETIME
			std::cout << "Deleting a SplitContainer at " << this << " with " << mSplitNode.children().size() << " children." << std::endl;
		#endif

		clear();
	}





	void SplitContainer::updateLayout()
	{
		float totalRatio = 0;
		for (const auto & child: mSplitNode.children())
		{
			totalRatio += child.mRatio;
		}

		int offset = 0;
		auto rect = GetClientRect();
		auto totalWidth  = rect.width  - (mSplitNode.children().size() - 1) * SPLITTER_SIZE;
		auto totalHeight = rect.height - (mSplitNode.children().size() - 1) * SPLITTER_SIZE;
		for (const auto & child: mSplitNode.children())
		{
			float fraction = child.mRatio / totalRatio;
			wxRect childRect = rect;
			if (mSplitNode.orientation() == WxDockUI::Layout::SplitOrientation::Horizontal)
			{
				int width = static_cast<int>(totalWidth * fraction);
				childRect.x += offset;
				childRect.width = width;
				offset += width + SPLITTER_SIZE;
			}
			else
			{
				int height = static_cast<int>(totalHeight * fraction);
				childRect.y += offset;
				childRect.height = height;
				offset += height + SPLITTER_SIZE;
			}

			mFrameDockManager.layoutEngine().layoutNode(*child.mNode, this, childRect);
		}
	}





	void SplitContainer::clear()
	{
		#ifdef WXDOCKUI_DEBUG_LIFETIME
			std::cout << "Clearing a SplitContainer at " << this << " with " << mSplitNode.children().size() << " children." << std::endl;
		#endif

		// Reparent all children from this to mFrameDockManager, to avoid deleting them in the destructor:
		// (Must first create a vector, since Reparent() invalidates the list returned by GetChildren())
		std::vector<wxWindow *> children;
		for (auto child: this->GetChildren())
		{
			children.push_back(child);
		}
		for (auto child: children)
		{
			child->Reparent(mFrameDockManager.frame());
		}
	}





}  // namespace WxDockUI::Internal
