#pragma once

#include <vector>

#include <wx/wx.h>





namespace WxDockUI::Internal
{




	// fwd:
	class FrameDockManager;




	/** Tracks the Z-order of the registered windows.
	Can perform hit-testing to see which of the tracked windows is the hit first at the specified point. */
	class ZOrderTracker
	{
		/** The Z-order of the FrameDockManager instances. mZOrder[0] is the topmost window. */
		std::vector<FrameDockManager *> mZOrder;

		// WX event callbacks:
		void onActivate(wxActivateEvent & aEvent);
		void onIconize(wxIconizeEvent & aEvent);
		void onClose(wxCloseEvent & aEvent);

		void bringToFront(FrameDockManager & aFrame);

		/** Returns the FrameDockManager in which the specified WX event has occured. */
		FrameDockManager * managerFromEvent(const wxEvent & aEvent);

	public:

		void add(FrameDockManager & aFrame);
		void remove(FrameDockManager & aFrame);

		FrameDockManager * hitTest(const wxPoint & aScreenPos) const;
	};

}  // namespace WxDockUI::Internal
