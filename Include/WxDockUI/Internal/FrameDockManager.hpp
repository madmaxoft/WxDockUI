#pragma once

#include <string>

#include <wx/frame.h>
#include <wx/sizer.h>

#include <WxDockUI/Internal/DockTarget.hpp>
#include <WxDockUI/Internal/DockOverlay.hpp>
#include <WxDockUI/Internal/Layout.hpp>
#include <WxDockUI/Internal/LayoutEngine.hpp>
#include <WxDockUI/Internal/PaneDragController.hpp>

#include <WxDockUI/Enums.hpp>
#include <WxDockUI/DockSystem.hpp>




// fwd:
namespace WxDockUI
{
	namespace Internal
	{
		class DragGhost;
		class FrameDockManager;
		class PaneContainer;
		class TabContainer;
	}
}





namespace WxDockUI::Internal
{





	/** The docking manager for a single top-level window. */
	class FrameDockManager
	{
		/** The layout engine used to manager the layout of the panes. */
		WxDockUI::Layout::LayoutEngine mLayoutEngine;

		/** The controller responsible for handling dragging panes around. */
		PaneDragController mPaneDragController;

		/** The overlay showing the dock icons and performing hit-testing on them. */
		DockOverlay mDockOverlay;

		/** The root of the layout managed by mLayoutEngine. */
		WxDockUI::Layout::RootNode mRoot;

		wxTopLevelWindow & mFrame;
		WxDockUI::DockSystem & mDockSystem;

		/** The ghost used to visualise dragging a pane around. */
		Internal::DragGhost * mDragGhost = nullptr;


		/** Called by WX when mFrame is resized. Adjusts the layout to fill the frame. */
		void onFrameSize(wxSizeEvent & aEvent);


	public:

		/** Creates a new instance of the manager bound to the specified frame,
		and attaches it to the specified dock system. */
		explicit FrameDockManager(wxTopLevelWindow & aFrame, DockSystem & aDockSystem);

		~FrameDockManager();

		/** Adds the specified pane to the layout tree for this frame. */
		void addPane(const PaneId & aPaneId, const PaneInfo & aPaneInfo);

		// Layout persistence
		std::string saveLayout() const;
		void loadLayout(const std::string & aLayout);

		// Layout update
		void updateLayout();

		/** Dumps the complete layout to the specified ostream, in an indented-tree format.
		Note that the format is directly usable to build a layout tree in a test in LayoutOpsTest.cpp. */
		void dumpLayout(std::ostream & aOut) const;

		// Getters for the specific action engines / controllers:
		WxDockUI::DockSystem & dockSystem() { return mDockSystem; }
		WxDockUI::Layout::LayoutEngine & layoutEngine() { return mLayoutEngine; }
		WxDockUI::Internal::PaneDragController & paneDragController() { return mPaneDragController; }
		WxDockUI::Internal::DockOverlay & dockOverlay() { return mDockOverlay; }

		wxTopLevelWindow * frame() const { return &mFrame; }

		/** Returns the wxWindow that is contained in the specified PaneNode, or nullptr if not found. */
		wxWindow * findPaneWindow(const Layout::PaneNode & aPaneNode) const { return findPaneWindow(aPaneNode.paneId()); }

		/** Returns the wxWindow that is contained in the specified pane, or nullptr if no such pane. */
		wxWindow * findPaneWindow(const PaneId & aPaneId) const { return mDockSystem.findPaneWindow(aPaneId); }

		/** Returns the PaneInfo for the specified pane, or nullptr if no such pane. */
		const PaneInfo * findPaneInfo(const PaneId & aPaneId) const { return mDockSystem.findPaneInfo(aPaneId); }

		/** Internal: Performs the dock operation on aDraggedPane specified by the target. */
		void performDock(const Layout::PaneNode & aDraggedPane, const Internal::DockTarget & aTarget);

		/** Internal: Call this before an expected breakpoint to allow the debugger / IDE to use the mouse. */
		void uncaptureMouse();
	};





}  // namespace WxDockUI
