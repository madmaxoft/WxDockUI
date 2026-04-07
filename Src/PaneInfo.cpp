#include <WxDockUI/PaneInfo.hpp>





namespace WxDockUI
{





	PaneInfo & PaneInfo::caption(const wxString & aCaption)
	{
		mCaption = aCaption;
		return *this;
	}





	PaneInfo & PaneInfo::left()
	{
		mInitialDock = DockPosition::Left;
		return *this;
	}





	PaneInfo & PaneInfo::right()
	{
		mInitialDock = DockPosition::Right;
		return *this;
	}





	PaneInfo & PaneInfo::bottom()
	{
		mInitialDock = DockPosition::Bottom;
		return *this;
	}





	PaneInfo & PaneInfo::top()
	{
		mInitialDock = DockPosition::Top;
		return *this;
	}





	PaneInfo & PaneInfo::center()
	{
		mInitialDock = DockPosition::Center;
		return *this;
	}





	PaneInfo & PaneInfo::bestSize(int aWidth, int aHeight)
	{
		mBestWidth = aWidth;
		mBestHeight = aHeight;
		return *this;
	}

}  // namespace WxDockUI
