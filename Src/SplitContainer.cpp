#include <WxDockUI/Internal/SplitContainer.h>

#include <wx/dcbuffer.h>

#include <WxDockUI/FrameDockManager.h>
#include <WxDockUI/Internal/Layout.h>





namespace WxDockUI::Internal
{





	static constexpr int SPLITTER_SIZE = 6;





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

		Bind(wxEVT_PAINT, &SplitContainer::onPaint, this);
		SetBackgroundStyle(wxBG_STYLE_PAINT);
	}





	SplitContainer::~SplitContainer()
	{
		#ifdef WXDOCKUI_DEBUG_LIFETIME
			std::cout << "Deleting a SplitContainer at " << this << " with " << mSplitNode.children().size() << " children." << std::endl;
		#endif

		clear();
	}





	void SplitContainer::recalculatePixelSizes()
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
		mSplitterPixelSizes.clear();
		mSplitterRects.clear();
		for (const auto & child: mSplitNode.children())
		{
			float fraction = child.mRatio / totalRatio;
			wxRect childRect = rect;
			if (mSplitNode.orientation() == WxDockUI::Layout::SplitOrientation::Horizontal)
			{
				auto width = static_cast<int>(totalWidth * fraction);
				mSplitterPixelSizes.push_back(width);
				offset += width + SPLITTER_SIZE;
				mSplitterRects.emplace_back(offset - SPLITTER_SIZE, 0, SPLITTER_SIZE, rect.height);
			}
			else
			{
				auto height = static_cast<int>(totalHeight * fraction);
				mSplitterPixelSizes.push_back(height);
				offset += height + SPLITTER_SIZE;
				mSplitterRects.emplace_back(0, offset - SPLITTER_SIZE, rect.width, SPLITTER_SIZE);
			}
		}
	}





	void SplitContainer::onSplitterLeftDown(wxMouseEvent & aEvent)
	{
		/*
		auto pos = aEvent.GetPosition();
		for (size_t i = 0; i < mSplitterRects.size(); ++i)
		{
			if (mSplitterRects[i].Contains(pos))
			{
				mDraggingSplitter = static_cast<int>(i);
				if (mOrientation == Orientation::Horizontal)
				{
					mDragStartPos = pos.x;
				}
				else
				{
					mDragStartPos = pos.y;
				}

				auto & childA = mChildren[i];
				auto & childB = mChildren[i + 1];

				wxRect rectA = childA->GetRect();
				wxRect rectB = childB->GetRect();

				mDragStartSizeA = (mOrientation == Orientation::Horizontal) ? rectA.width  : rectA.height;
				mDragStartSizeB = (mOrientation == Orientation::Horizontal) ? rectB.width  : rectB.height;

				CaptureMouse();
				return;
			}
		}
		*/
	}





	void SplitContainer::onPaint(wxPaintEvent & aEvent)
	{
		wxAutoBufferedPaintDC dc(this);
		dc.Clear();
		wxPen penLighter(wxColor(255, 255, 255));
		wxPen penDarker(wxColor(127, 127, 127));
		for (auto & r: mSplitterRects)
		{
			dc.SetPen(penLighter);
			if (mSplitNode.orientation() == Layout::SplitOrientation::Horizontal)
			{
				dc.DrawLine(r.x, r.y, r.x, r.y + r.height);
				dc.DrawLine(r.x + r.width - 2, r.y, r.x + r.width - 2, r.y + r.height);
			}
			else
			{
				dc.DrawLine(r.x, r.y, r.x + r.width, r.y);
				dc.DrawLine(r.x, r.y + r.height - 2, r.x + r.width, r.y + r.height - 2);
			}
			dc.SetPen(penDarker);
			if (mSplitNode.orientation() == Layout::SplitOrientation::Horizontal)
			{
				dc.DrawLine(r.x + 1, r.y, r.x + 1, r.y + r.height);
				dc.DrawLine(r.x + r.width - 1, r.y, r.x + r.width - 1, r.y + r.height);
			}
			else
			{
				dc.DrawLine(r.x, r.y + 1, r.x + r.width, r.y + 1);
				dc.DrawLine(r.x, r.y + r.height - 1, r.x + r.width, r.y + r.height - 1);
			}
		}
	}





	void SplitContainer::updateLayout()
	{
		// If not dragging, update the pixel sizes from ratios:
		if (mDragSplitter < 0)
		{
			recalculatePixelSizes();
		}
		assert(mSplitterPixelSizes.size() == mSplitNode.children().size());
		assert(mSplitterRects.size() == mSplitNode.children().size());

		// Update the children's layout, based on the current splitter pixel positions:
		size_t idx = 0;
		auto rect = GetClientRect();
		int offset = 0;
		for (const auto & child: mSplitNode.children())
		{
			wxRect childRect = rect;
			if (mSplitNode.orientation() == WxDockUI::Layout::SplitOrientation::Horizontal)
			{
				int width = mSplitterPixelSizes[idx];
				childRect.x += offset;
				childRect.width = width;
				offset += width + SPLITTER_SIZE;
			}
			else
			{
				int height = mSplitterPixelSizes[idx];
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
