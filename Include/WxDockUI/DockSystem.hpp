#pragma once

#include <memory>
#include <wx/wx.h>
#include <WxDockUI/PaneInfo.hpp>





// fwd:
namespace WxDockUI::Internal
{
	class FrameDockManager;
}





namespace WxDockUI
{





	/** The overall docking ecosystem. This is the main public API entrypoint.
	This is used to register the top-level windows for which docking is to be enabled, and all the panes that
	can be moved around. */
	class DockSystem
	{
		/** All the managed windows, wrapped in a FrameDockManager instance. */
		std::vector<std::unique_ptr<Internal::FrameDockManager>> mManagedWindows;

		/** Mapping from pane IDs to the pane info and actual wxWidget windows for the pane. */
		std::unordered_map<PaneId, std::pair<PaneInfo, wxWindow *>> mPanes;


	public:

		DockSystem();
		~DockSystem();

		/** Adds the window to be managed by the dock system - the window will work as a dock target.
		Silently ignored if the window is already managed. */
		void manageWindow(wxTopLevelWindow & aWindow);

		/** Adds a new pane to the dock system.
		The parent dock window needs to be already managed (by manageWindow()).
		The pane ID needs to be unique.
		Throws an exception if the parent dock is not registered or the ID is not unique. */
		void addPane(
			const wxTopLevelWindow & aParentDockWindow,
			wxWindow * aPaneWindow,
			const PaneId & aPaneId,
			const PaneInfo & aPaneInfo
		);


		/** Returns the wxWindow that is contained in the specified pane. */
		wxWindow * findPaneWindow(const PaneId & aPaneId) const;

		/** Returns the PaneInfo for the specified pane, or nullptr if no such pane. */
		const PaneInfo * findPaneInfo(const PaneId & aPaneId) const;
	};





}  // namespace WxDockUI
