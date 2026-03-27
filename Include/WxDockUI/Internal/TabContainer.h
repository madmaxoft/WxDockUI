#pragma once

#include <memory>
#include <wx/panel.h>
#include <wx/notebook.h>





// fwd:
class wxBoxSizer;

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
	class TabContainer final:
		public wxPanel
	{
		using Super = wxPanel;

		/** Wrapper for a single pane's tab. */
		struct Tab
		{
			/** The panel directly inside mNotebook. The pane's contents are reparented into this.
			Owns the panel, mNotebook "borrows" the pointer but never actually deletes it. */
			std::unique_ptr<wxPanel> mPanel;

			/** The sizer that takes care of sizing the pane's contents into mPanel upon mPanel's size change.
			Owned by mPanel. */
			wxBoxSizer * mSizer = nullptr;

			/** The pane's caption. */
			std::string mCaption;
		};

		/** The movement threshold before a drag operation is started. */
		static constexpr int DRAG_THRESHOLD_PIXELS = 5;

		/** The manager responsible for creating and storing housekeeping UI elements. */
		FrameDockManager & mFrameDockManager;

		/** The layout node this container reflects. */
		const Layout::TabNode & mTabNode;

		/** The GUI page switcher. **/
		wxNotebook * mNotebook = nullptr;

		/** Map of PaneNode -> Tab inside mNotebook containing the pane's contents. */
		std::unordered_map<const Layout::PaneNode *, Tab> mPaneTabs;

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

		/** Returns the Tab instance representing the specified pane.
		If no such instance exists, creates it and adds it to mPaneTabs as well. */
		Tab & ensureTabForPane(const Layout::PaneNode & aPaneNode);


	public:
		TabContainer(
			FrameDockManager & aFrameDockManager,
			wxWindow * aParent,
			const Layout::TabNode & aTabNode
		);

		~TabContainer();

		/** Updates the layout of the tab container, shows active pane. */
		void updateLayout();

		/** Updates the GUI tab switcher to show all the children of mTabNode. */
		void updateTabs();

		const Layout::TabNode & tabNode() const { return mTabNode; }
	};




}  // namespace WxDockUI::Internal
