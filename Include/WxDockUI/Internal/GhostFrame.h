#pragma once

#include <wx/popupwin.h>
#include <wx/panel.h>





namespace WxDockUI::Internal
{





	/** Represents a visual cue displayed while dragging a PaneContainer. */
	class GhostFrame final:
		public wxPopupWindow
	{
		using Super = wxPopupWindow;


		wxPanel * mContent = nullptr;


	public:

		explicit GhostFrame(wxWindow * aParent);
	};





}  // namespace WxDockUI
