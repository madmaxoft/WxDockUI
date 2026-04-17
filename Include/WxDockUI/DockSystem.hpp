#pragma once

#include <memory>
#include <wx/wx.h>
#include <WxDockUI/PaneInfo.hpp>
#include <WxDockUI/Internal/PaneDragController.hpp>
#include <WxDockUI/Internal/ZOrderTracker.hpp>





// fwd:
namespace WxDockUI::Internal
{
	class FrameDockManager;
	class FloatingDockFrame;
}





namespace WxDockUI
{





	/** The overall docking ecosystem. This is the main public API entrypoint.
	This is used to register the top-level windows for which docking is to be enabled, and all the panes that
	can be moved around. */
	class DockSystem
	{
		friend class Internal::FrameDockManager;
		friend class Internal::PaneDragController;
		friend class Internal::FloatingDockFrame;

		/** All the managed windows, wrapped in a FrameDockManager instance. */
		std::vector<std::unique_ptr<Internal::FrameDockManager>> mManagedWindows;

		/** Mapping from pane IDs to the pane info and actual wxWidget windows for the pane. */
		std::unordered_map<PaneId, std::pair<PaneInfo, wxWindow *>> mPanes;

		/** The controller responsible for handling dragging panes around. */
		Internal::PaneDragController mPaneDragController;

		/** Tracker for the Z-order of all mManagedWindows. Used for hit-testing while dragging a pane. */
		Internal::ZOrderTracker mZOrderTracker;


		Internal::PaneDragController & paneDragController() { return mPaneDragController; }

		/** Returns the target frame for a dock operation, creating a new one if needed (-> floating) */
		Internal::FrameDockManager & ensureDockTargetFrame(const Internal::DockTarget & aDockTarget);

		/** Docks the specified dragged pane into the specified target. */
		void performDock(
			Internal::FrameDockManager & aSourceFrame,
			const Layout::PaneNode & aDraggedPane,
			const Internal::DockTarget & aTarget
		);

		/** Destroys the frame and its manager if it is floating and is practically empty. */
		void destroyIfEmptyFloating(Internal::FrameDockManager & aFrameDockManager);

		/** Destroys the specified frame and its manager, removing it from the registry and ZOrderTracker. */
		void destroyManagedWindow(Internal::FrameDockManager & aFrameDockManager);

		/** Called by aFrameDockManager when the frame is about to be destroyed.
		Removes the management and deletes the aFrameDockManager. */
		void onManagedWindowDestroy(Internal::FrameDockManager & aFrameDockManager);

		/** Returns the managed window that is visible at the specified screen position.
		Returns nullptr if none. */
		Internal::FrameDockManager * managedWindowAtScreenPos(const wxPoint & aScreenPos);


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
