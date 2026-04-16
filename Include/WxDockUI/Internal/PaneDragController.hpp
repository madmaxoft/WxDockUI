#pragma once

#include <optional>

#include <WxDockUI/Internal/DockTarget.hpp>
#include <WxDockUI/Internal/DragGhost.hpp>

#include <wx/wx.h>





// fwd:
namespace WxDockUI
{
	class DockSystem;
	namespace Internal
	{
		class FrameDockManager;
		class PaneContainer;
	}
}





namespace WxDockUI::Internal
{





	/** Handles dragging panes around multiple FrameDockManagers. Tracks the source and target and
	peforms the final dock. */
	class PaneDragController
	{
		/** The DockSystem in which the controller lives. */
		WxDockUI::DockSystem & mDockSystem;

		/** The frame from which the pane is being dragged.
		Nullptr if not dragging anything (primary flag for "is anything being dragged?") */
		FrameDockManager * mSourceFrame = nullptr;

		/** The frame into which the pane is currently being dragged. May change during a single drag operation if
		the user moves the mouse to another frame.
		Nullptr if not targeting a frame (the pane will get floated). */
		FrameDockManager * mTargetFrame = nullptr;

		/** The pane being currently dragged.
		nullptr if not dragging anything. */
		const Layout::PaneNode * mDraggedPane = nullptr;

		std::optional<DockTarget> mCurrentTarget;

		/** The ghost used to visualise dragging a pane around. */
		DragGhost * mDragGhost = nullptr;


	public:

		explicit PaneDragController(DockSystem & aDockSystem);

		bool isDragging() const;

		/** Begins a drag operation for the specified pane dragged out of the specified source frame. The source
		frame is assumed to be the current target frame as well. */
		void beginDrag(FrameDockManager & aSourceFrame, const Layout::PaneNode * aDraggedPane, const wxPoint & aScreenPos);

		/** Updates the current drag operation to target the specified frame and mouse position. */
		void updateDrag(const wxPoint & aScreenPos);

		/** Ends the current drag operation, performs the actual drag into the target frame. */
		void endDrag(const wxPoint & aScreenPos);

		/** Cancels the current drag operation without performing the actual drag. */
		void cancelDrag();


	private:

		/** Clears the current drag state - removes the ghost, source, target, overlay. */
		void clearState();

		/** Updates the overlay and ghost, based on the current target and screen pos. */
		void updateUI(const wxPoint & aScreenPos);

		/** Moves the mDragGhost to indicate mCurrentTarget. */
		void moveDragGhostToTarget(const wxPoint & aScreenPos);

		/** Moves the mDragGhost so that it covers the current target pane at the specified percentages of the pane. */
		void moveDragGhostToTargetPane(int aTopPercent, int aLeftPercent, int aBottomPercent, int aRightPercent);

		/** Moves the mDragGhost so that it covers the root at the specified percentages of the root. */
		void moveDragGhostToTargetRoot(int aTopPercent, int aLeftPercent, int aBottomPercent, int aRightPercent);
	};





}  // namespace WxDockUI::Internal
