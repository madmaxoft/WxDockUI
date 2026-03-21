#include <WxDockUI/Internal/SplitContainer.h>

#include <wx/dcbuffer.h>

#include <WxDockUI/FrameDockManager.h>
#include <WxDockUI/Internal/Layout.h>





namespace WxDockUI::Internal
{




	using namespace WxDockUI::Layout;





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

		SetBackgroundStyle(wxBG_STYLE_PAINT);
		SetCursor((aSplitNode.orientation() == SplitOrientation::Horizontal) ? wxCURSOR_SIZEWE : wxCURSOR_SIZENS);

		Bind(wxEVT_PAINT, &SplitContainer::onPaint, this);
		Bind(wxEVT_LEFT_DOWN,          &SplitContainer::onSplitterMouseLeftDown, this);
		Bind(wxEVT_MOTION,             &SplitContainer::onSplitterMouseMotion, this);
		Bind(wxEVT_LEFT_UP,            &SplitContainer::onSplitterMouseLeftUp, this);
		Bind(wxEVT_MOUSE_CAPTURE_LOST, &SplitContainer::onSplitterMouseCaptureLost, this);
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
			if (mSplitNode.orientation() == SplitOrientation::Horizontal)
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





	void SplitContainer::onSplitterMouseLeftDown(wxMouseEvent & aEvent)
	{
		// Find which splitter, if any, is being dragged:
		auto pos = aEvent.GetPosition();
		for (size_t i = 0; i < mSplitterRects.size(); ++i)
		{
			if (!mSplitterRects[i].Contains(pos))
			{
				continue;
			}
			mDraggedSplitter = static_cast<int>(i);
			CaptureMouse();
			return;
		}
	}





	void SplitContainer::onSplitterMouseMotion(wxMouseEvent & aEvent)
	{
		if (mDraggedSplitter < 0)
		{
			aEvent.Skip();
			return;
		}
		assert(mDraggedSplitter < mSplitterPixelSizes.size() - 1);
		assert(mDraggedSplitter < mSplitterRects.size() - 1);

		// Clamp the pos between the two neighboring splitters:
		auto pos = aEvent.GetPosition();
		auto mousePos = (mSplitNode.orientation() == SplitOrientation::Horizontal) ? pos.x : pos.y;
		int minPos = 0;
		for (int idx = 0; idx < mDraggedSplitter; ++idx)
		{
			minPos += mSplitterPixelSizes[idx];
		}
		auto maxPos = minPos + mSplitterPixelSizes[mDraggedSplitter] + mSplitterPixelSizes[mDraggedSplitter + 1];
		if (mousePos <= minPos)
		{
			mousePos = minPos + 1;
		}
		if (mousePos >= maxPos)
		{
			mousePos = maxPos - 1;
		}

		// Apply into the pixel sizes and rects:
		mSplitterPixelSizes[mDraggedSplitter] = mousePos - minPos;
		mSplitterPixelSizes[mDraggedSplitter + 1] = maxPos - mousePos;
		if (mSplitNode.orientation() == SplitOrientation::Horizontal)
		{
			mSplitterRects[mDraggedSplitter].SetLeft(mousePos);
		}
		else
		{
			mSplitterRects[mDraggedSplitter].SetTop(mousePos);
		}
		updateLayout();
		aEvent.Skip();
	}





	void SplitContainer::onSplitterMouseLeftUp(wxMouseEvent & aEvent)
	{
		// Finalize any outstanding motion:
		onSplitterMouseMotion(aEvent);

		if (HasCapture())
		{
			ReleaseMouse();
		}

		mDraggedSplitter = -1;

		// Apply the pixel sizes back into split ratios:
		size_t idx = 0;
		for (auto & ch: mSplitNode.children())
		{
			ch.mRatio = mSplitterPixelSizes[idx++];
		}

		aEvent.Skip();
	}





	void SplitContainer::onSplitterMouseCaptureLost(wxMouseCaptureLostEvent & aEvent)
	{
		mDraggedSplitter = -1;
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
		if (mDraggedSplitter < 0)
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
			if (mSplitNode.orientation() == SplitOrientation::Horizontal)
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
			assert(childRect.width > 0);
			assert(childRect.height > 0);
			mFrameDockManager.layoutEngine().layoutNode(*child.mNode, this, childRect);
			++idx;
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
