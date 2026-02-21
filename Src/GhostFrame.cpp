#include "WxDockUI/Internal/GhostFrame.h"





namespace WxDockUI::Internal
{





	GhostFrame::GhostFrame(wxWindow * aParent):
		Super(aParent, wxBORDER_SIMPLE)
	{
		SetBackgroundColour(wxColor(0, 120, 215, 100)); // semi-transparent blue
		// SetBackgroundStyle(wxBG_STYLE_TRANSPARENT);
		SetTransparent(160);
		mContent = new wxPanel(this);
	}





}  // namespace WxDockUI
