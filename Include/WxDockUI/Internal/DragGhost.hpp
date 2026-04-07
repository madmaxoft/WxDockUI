#pragma once

#include <wx/frame.h>
#include <wx/panel.h>





namespace WxDockUI::Internal
{





	/** Represents a visual cue displayed while dragging a PaneContainer. */
	class DragGhost final:
		public wxFrame
	{
		using Super = wxFrame;


		wxPanel * mContent = nullptr;


	public:

		DragGhost();
	};





}  // namespace WxDockUI
