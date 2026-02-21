#pragma once

#include <optional>

#include <WxDockUI/Internal/DockTarget.h>
#include <WxDockUI/Internal/GhostFrame.h>

#include <wx/wx.h>





// fwd:
namespace WxDockUI
{
	class FrameDockManager;
	namespace Internal
	{
		class PaneContainer;
	}
}





namespace WxDockUI::Internal
{





	/** Handles dragging panes around and finalizing their docking. */
	class PaneDragController
	{
		/** The parent manager handling the actual docking and overlay. */
		FrameDockManager & mFrameDockManager;

		/** The pane being currently dragged.
		nullptr if not dragging anything. */
		Layout::PaneNode * mDraggedPane = nullptr;

		std::optional<WxDockUI::Internal::DockTarget> mCurrentTarget;

		/** The ghost used to visualise dragging a pane around. */
		Internal::GhostFrame * mDragGhost = nullptr;


	public:

		explicit PaneDragController(WxDockUI::FrameDockManager & aFrameDockManager);

		bool isDragging() const;

		void beginDrag(Internal::PaneContainer * aPane, const wxPoint & aScreenPos);
		void updateDrag(Internal::PaneContainer * aPane, const wxPoint & aScreenPos);
		void endDrag(Internal::PaneContainer * aPane, const wxPoint & aScreenPos);
		void cancelDrag(Internal::PaneContainer * aPane);

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
