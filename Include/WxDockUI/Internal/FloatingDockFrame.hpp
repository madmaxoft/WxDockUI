#pragma once

#include <wx/wx.h>





// fwd:
namespace WxDockUI
{
	class DockSystem;
	namespace Internal
	{
		class FrameDockManager;
	}
}





namespace WxDockUI::Internal
{




	/** The frame for a floating dock, auto-created by DockSystem when a pane is dragged out of a window. */
	class FloatingDockFrame:
		public wxFrame
	{
		using Super = wxFrame;
		friend class WxDockUI::DockSystem;

		/** The DockSystem that created this frame. */
		WxDockUI::DockSystem & mDockSystem;

		/** The FDM handling this frame's docking. Set by DockSystem after creating the frame. */
		FrameDockManager * mFrameDockManager = nullptr;


		/** Sets the mFrameDockManager after it is created for the instance within DockSystem. */
		void setFrameDockManager(FrameDockManager & aFrameDockManager);

		// WX events:
		void onClose(wxCloseEvent & aEvent);


	public:
		explicit FloatingDockFrame(WxDockUI::DockSystem & aDockSystem);

		// Getters:
		FrameDockManager & frameDockManager();

		/** Updates the frame caption based on contained panes. */
		void updateCaption();
	};

}  // namespace WxDockUI::Internal
