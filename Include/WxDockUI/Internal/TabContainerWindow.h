#pragma once

#include <wx/panel.h>
#include <wx/notebook.h>





// fwd:
namespace WxDockUI
{
	class FrameDockManager;
	namespace Layout
	{
		class TabNode;
		class PaneNode;
	}
}





namespace WxDockUI::Internal
{





	/** A visual container for a single Layout::TabNode.
	Provides the wxNotebook for switching panes, close button, etc. */
	class TabContainerWindow final:
		public wxPanel
	{
		using Super = wxPanel;

		/** The movement threshold before a drag operation is started. */
		static constexpr int DRAG_THRESHOLD_PIXELS = 5;

		/** The manager responsible for creating and storing housekeeping UI elements. */
		FrameDockManager & mFrameDockManager;

		/** The layout node this container reflects. */
		Layout::TabNode & mTabNode;

		/** The GUI page switcher. **/
		wxNotebook * mNotebook = nullptr;

		// Dragging support:
		/** If true, the user is dragging mouse within mNotebook and hasn't started a pane drag yet. */
		bool mIsDraggingTab = false;

		/** The PaneNode to be detached from this TabNode being dragged around. */
		Layout::PaneNode * mDraggedPane = nullptr;

		int mDragTabIndex = wxNOT_FOUND;
		wxPoint mDragStartScreenPos;


		/** Called by WX when the active page in mNotebook changes. */
		void onNotebookChanged(wxBookCtrlEvent & aEvent);

		/** Called by WX when the user presses left-click on mNotebook. */
		void onNotebookLeftDown(wxMouseEvent & aEvent);

		/** Called by WX when the user drags the mouse on mNotebook. */
		void onNotebookMouseMove(wxMouseEvent & aEvent);

		/** Called by WX when the user releases left-click on mNotebook. */
		void onNotebookLeftUp(wxMouseEvent & aEvent);

		/** Starts a drag operation for the mDragTabIndex-th tab. */
		void startTabDrag();


	public:
		TabContainerWindow(
			FrameDockManager & aFrameDockManager,
			wxWindow * aParent,
			Layout::TabNode & aTabNode
		);

		/** Updates the layout of the tab container, shows active pane. */
		void updateLayout();

		/** Updates the GUI tab switcher to show all the children of mTabNode. */
		void updateTabs();
	};




}  // namespace WxDockUI::Internal
