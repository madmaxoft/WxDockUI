#include "WxDockUI/DockSystem.h"





namespace WxDockUI
{





	void DockSystem::registerManager(FrameDockManager * aManager)
	{
		mFrameDockManagers.push_back(aManager);
	}





	void DockSystem::unregisterManager(FrameDockManager * aManager)
	{
		mFrameDockManagers.erase(std::remove(mFrameDockManagers.begin(), mFrameDockManagers.end(), aManager));
	}





}  // namespace WxDockUI
