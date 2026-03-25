#pragma once

#include <memory>
#include <wx/panel.h>





// fwd:
namespace WxDockUI
{
	class FrameDockManager;
	namespace Layout
	{
		class SplitNode;
		class PaneNode;
	}
}





namespace WxDockUI::Internal
{





	/** A visual container for a single Layout::SplitNode.
	Provides the wxPanel representing the entire node, draws the splitters, handles resizing children. */
	class SplitContainer final:
		public wxPanel
	{
		using Super = wxPanel;


		/** The manager responsible for creating and storing housekeeping UI elements. */
		FrameDockManager & mFrameDockManager;

		/** The layout node this container reflects. */
		Layout::SplitNode & mSplitNode;

		/** The rectangles representing the splitters between the children.
		Splitter n is in between child n and n + 1. */
		std::vector<wxRect> mSplitterRects;

		/** Sizes of individual children, in pixels.
		Converted from SplitNode ratios upon layouting.
		Adjusted by the user dragging in onSplitterMouseMove. */
		std::vector<int> mSplitterPixelSizes;

		/** The index of the spliter currently being dragged. -1 if not dragging anything. */
		int mDraggedSplitter = -1;


		/** Updates the mSplitterPixelSizes based on the current split children and ratios. */
		void recalculatePixelSizes();

		/** Applies the current mouse position in the relevant direction into the position
		of the splitter being currently dragged.
		Updates the mSplitterPixelSizes[] for both sides of the splitter being dragged, as well as mSplitterRects[]. */
		void applyDraggedSplitterPos(int aMousePos);

		/** Called by WX when the user presses left-click on the widget. */
		void onSplitterMouseLeftDown(wxMouseEvent & aEvent);

		/** Called by WX when the user drags the mouse over the widget. */
		void onSplitterMouseMotion(wxMouseEvent & aEvent);

		/** Called by WX when the user releases left-click on the widget. */
		void onSplitterMouseLeftUp(wxMouseEvent & aEvent);

		/** Called by WX when the mouse capture is lost while the widget had it captured. */
		void onSplitterMouseCaptureLost(wxMouseCaptureLostEvent & aEvent);

		/** Called by WX to draw the contents. Draws the splitter handles. */
		void onPaint(wxPaintEvent & aEvent);

		/** Called by WX when it needs to set the cursor. */
		void onSetCursor(wxSetCursorEvent & aEvent);


	public:

		/** Creates a new instance representing the specified split node. */
		SplitContainer(
			FrameDockManager & aFrameDockManager,
			wxWindow * aParent,
			Layout::SplitNode & aSplitNode
		);

		/** Destroys the instance. */
		~SplitContainer();

		/** Updates the layout of the container. */
		void updateLayout();

		/** Reparents all children into mFrameDockManager's frame.
		Used before destruction of this class to avoid actually destroying child windows that are owned elsewhere. */
		void clear();
	};




}  // namespace WxDockUI::Internal
