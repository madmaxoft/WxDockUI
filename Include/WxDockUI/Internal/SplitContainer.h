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


		/** The movement threshold before a spliter-drag operation is started. */
		static constexpr int DRAG_THRESHOLD_PIXELS = 5;

		/** The manager responsible for creating and storing housekeeping UI elements. */
		FrameDockManager & mFrameDockManager;

		/** The layout node this container reflects. */
		Layout::SplitNode & mSplitNode;


		/** Called by WX when the user presses left-click on mNotebook. */
		void onSplitterLeftDown(wxMouseEvent & aEvent);

		/** Called by WX when the user drags the mouse on mNotebook. */
		void onSplitterMouseMove(wxMouseEvent & aEvent);

		/** Called by WX when the user releases left-click on mNotebook. */
		void onSplitterLeftUp(wxMouseEvent & aEvent);


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
