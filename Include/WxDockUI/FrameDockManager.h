#pragma once

#include <string>

#include <wx/frame.h>
#include <wx/sizer.h>

#include <WxDockUI/Internal/DockTarget.h>
#include <WxDockUI/Internal/Layout.h>
#include <WxDockUI/Internal/LayoutEngine.h>
#include <WxDockUI/Internal/PaneDragController.h>
#include <WxDockUI/Internal/DockOverlay.h>

#include <WxDockUI/Enums.h>





namespace WxDockUI
{





	// fwd:
	class DockSystem;
	class FrameDockManager;
	namespace Internal
	{
		class PaneContainer;
		class DragGhost;
		class TabContainer;
	}





	/** ID used to identify individual panes in the docking manager. */
	using PaneId = std::string;





	class PaneInfo
	{
		PaneId mPaneId;
		wxString mCaption;

		DockPosition mInitialDock = DockPosition::Left;

		bool mIsClosable = true;
		bool mIsFloatable = true;
		bool mIsMovable = true;
		bool mIsVisible = true;

		int mBestWidth = -1;
		int mBestHeight = -1;


	public:

		PaneInfo() = default;
		PaneInfo(const PaneId & aPaneId);

		// Chainable mutators:
		PaneInfo & caption(const wxString & aCaption);
		PaneInfo & left();
		PaneInfo & right();
		PaneInfo & bottom();
		PaneInfo & top();
		PaneInfo & center();
		PaneInfo & bestSize(int aWidth, int aHeight);

		// Getters:
		const std::string & paneId() const { return mPaneId; }
		const wxString & caption() const { return mCaption; }
		DockPosition initialDock() const { return mInitialDock; }
		int bestWidth() const { return mBestWidth; }
		int bestHeight() const { return mBestHeight; }
	};





	/** The docking manager for a single top-level window. */
	class FrameDockManager
	{
		/** The layout engine used to manager the layout of the panes. */
		Layout::LayoutEngine mLayoutEngine;

		/** The controller responsible for handling dragging panes around. */
		Internal::PaneDragController mPaneDragController;

		/** The overlay showing the dock icons and performing hit-testing on them. */
		Internal::DockOverlay mDockOverlay;

		/** The root of the layout managed by mLayoutEngine. */
		Layout::RootNode mRoot;

		wxFrame & mFrame;
		DockSystem & mDockSystem;

		/** Mapping from layout pane IDs to actual wxWidget windows. */
		std::unordered_map<PaneId, wxWindow *> mPaneWindows;

		/** Mapping from layout pane IDs to their infos. */
		std::unordered_map<PaneId, PaneInfo> mPaneInfos;

		/** The ghost used to visualise dragging a pane around. */
		Internal::DragGhost * mDragGhost = nullptr;


		/** Called by WX when mFrame is resized. Adjusts the layout to fill the frame. */
		void onFrameSize(wxSizeEvent & aEvent);


	public:

		/** Creates a new instance of the manager bound to the specified frame,
		and attaches it to the specified dock system. */
		explicit FrameDockManager(wxFrame & aFrame, DockSystem & aDockSystem);

		~FrameDockManager();

		// Pane lifecycle
		void addPane(wxWindow * aWindow, const PaneInfo & aOptions);
		void removePane(const PaneId & aPaneId);

		// Visibility
		void showPane(const PaneId & aPaneId, bool aShouldShow);
		bool isPaneVisible(const PaneId & aPaneId) const;

		// Docking operations
		void dockPane(const PaneId & aPaneId, DockPosition aPosition);
		void floatPane(const PaneId & aPaneId);

		// Layout persistence
		std::string saveLayout() const;
		void loadLayout(const std::string & aLayout);

		// Layout update
		void updateLayout();

		/** Dumps the complete layout to the specified ostream, in an indented-tree format.
		Note that the format is directly usable to build a layout tree in a test in LayoutOpsTest.cpp. */
		void dumpLayout(std::ostream & aOut) const;

		// Getters for the specific action engines / controllers:
		Layout::LayoutEngine & layoutEngine() { return mLayoutEngine; }
		Internal::PaneDragController & paneDragController() { return mPaneDragController; }
		Internal::DockOverlay & dockOverlay() { return mDockOverlay; }

		wxFrame * frame() const { return &mFrame; }

		/** Internal: Returns the wxWindow that is contained in the specified pane. */
		wxWindow * findPaneWindow(const PaneId & aId) const;

		/** Internal: Returns the wxWindow that is contained in the specified pane. */
		wxWindow * findPaneWindow(const Layout::PaneNode & aPane) const;

		/** Internal: Returns the PaneInfo for the specified pane, or nullptr if no such pane. */
		const PaneInfo * findPaneInfo(const PaneId & aId);

		/** Internal: Performs the dock operation on aDraggedPane specified by the target. */
		void performDock(const Layout::PaneNode & aDraggedPane, const Internal::DockTarget & aTarget);

		/** Internal: Call this before an expected breakpoint to allow the debugger / IDE to use the mouse. */
		void uncaptureMouse();
	};





}  // namespace WxDockUI
