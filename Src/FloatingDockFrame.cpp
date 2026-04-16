#include <WxDockUI/Internal/FloatingDockFrame.hpp>




namespace WxDockUI::Internal
{





	FloatingDockFrame::FloatingDockFrame(WxDockUI::DockSystem & aDockSystem):
		Super(
			nullptr,  // No parent
			wxID_ANY,
			"",  // No caption
			wxGetMousePosition(),
			wxSize(200, 200),
			wxFRAME_NO_TASKBAR | wxSTAY_ON_TOP | wxFRAME_TOOL_WINDOW
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
		aEvent.Skip();
		// TODO: Notify mDockSystem
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
