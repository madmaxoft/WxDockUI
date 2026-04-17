#include <WxDockUI/Internal/FloatingDockFrame.hpp>

#include <WxDockUI/DockSystem.hpp>




namespace WxDockUI::Internal
{
	namespace
	{
		long floatingFrameStyle()
		{
			#ifdef __WXMSW__
				return wxCAPTION | wxCLOSE_BOX | wxSYSTEM_MENU | wxFRAME_NO_TASKBAR | wxSTAY_ON_TOP;
			#else
				return wxCAPTION | wxFRAME_NO_TASKBAR | wxSTAY_ON_TOP | wxFRAME_TOOL_WINDOW;
			#endif
		}
	}





	FloatingDockFrame::FloatingDockFrame(WxDockUI::DockSystem & aDockSystem):
		Super(
			nullptr,  // No parent
			wxID_ANY,
			"",  // No caption
			wxGetMousePosition(),
			wxSize(200, 200),
			floatingFrameStyle()
		),
		mDockSystem(aDockSystem)
	{
		Bind(wxEVT_CLOSE_WINDOW, &FloatingDockFrame::onClose, this);
	}





	void FloatingDockFrame::setFrameDockManager(FrameDockManager & aFrameDockManager)
	{
		assert(mFrameDockManager == nullptr);
		mFrameDockManager = &aFrameDockManager;
	}





	void FloatingDockFrame::onClose(wxCloseEvent & aEvent)
	{
		if (mFrameDockManager == nullptr)
		{
			aEvent.Skip();
			return;
		}

		mDockSystem.destroyManagedWindow(*mFrameDockManager);
	}





	FrameDockManager & FloatingDockFrame::frameDockManager()
	{
		assert(mFrameDockManager != nullptr);
		return *mFrameDockManager;
	}





	void FloatingDockFrame::updateCaption()
	{
		// TODO
	}

}  // namespace WxDockUI::Internal
