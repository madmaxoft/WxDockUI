#pragma once

#include <wx/frame.h>





namespace WxDockUI
{





	// fwd: FrameDockManager.h
	class FrameDockManager;





	/** The overall docking ecosystem. Allows moving panes between multiple top-level windows (FrameDockManagers) */
	class DockSystem
	{
		std::vector<FrameDockManager *> mFrameDockManagers;

	public:
		void registerManager(FrameDockManager * aManager);
		void unregisterManager(FrameDockManager * aManager);
	};





}  // namespace WxDockUI
