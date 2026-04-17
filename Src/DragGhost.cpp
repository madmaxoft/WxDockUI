#include "WxDockUI/Internal/DragGhost.hpp"





namespace WxDockUI::Internal
{





	DragGhost::DragGhost():
		Super(
			nullptr,  // No parent
			wxID_ANY,
			"DragGhost",
			wxDefaultPosition,
			wxDefaultSize,
			wxFRAME_NO_TASKBAR | wxSTAY_ON_TOP | wxBORDER_NONE | wxFRAME_SHAPED
		)
	{
		SetWindowStyleFlag(wxFRAME_NO_TASKBAR | wxSTAY_ON_TOP | wxBORDER_NONE | wxFRAME_SHAPED);
		SetBackgroundColour(wxColor(0, 120, 215, 100)); // semi-transparent blue
		SetTransparent(160);
		mContent = new wxPanel(this);
	}





}  // namespace WxDockUI
