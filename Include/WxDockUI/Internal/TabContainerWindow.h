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


		/** The manager responsible for creating and storing housekeeping UI elements. */
		FrameDockManager & mFrameDockManager;

		/** The layout node this container reflects. */
		Layout::TabNode & mTabNode;

		/** The GUI page switcher. **/
		wxNotebook * mNotebook = nullptr;


		/** Called by WX when the active page in mNotebook changes. */
		void onNotebookChanged(wxBookCtrlEvent & aEvent);


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
