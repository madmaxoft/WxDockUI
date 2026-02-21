#pragma once

#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/sizer.h>





// fwd:
namespace WxDockUI
{
	class FrameDockManager;

	namespace Layout
	{
		class PaneNode;
	}
}





namespace WxDockUI::Internal
{





	/** A visual container for a single Layout::PaneNode.
	Provides the caption, close button, etc. */
	class PaneContainer final:
		public wxPanel
	{
		using Super = wxPanel;

		/** The movement threshold before a drag operation is started. */
		static constexpr int DRAG_THRESHOLD_PIXELS = 5;

		FrameDockManager & mManager;

		/** The layout's PaneNode being represented by this UI container. */
		Layout::PaneNode & mPaneNode;

		// Visual:
		wxPanel * mCaptionBar = nullptr;
		wxStaticText * mCaptionText = nullptr;
		wxWindow * mClientWindow = nullptr;

		// Dragging support:
		wxPoint mDragStartPos;
		bool mIsDragging = false;


		// Events:
		void onMouseLeftDown(wxMouseEvent & aEvent);
		void onMouseMotion(wxMouseEvent & aEvent);
		void onMouseLeftUp(wxMouseEvent & aEvent);
		void onMouseCaptureLost(wxMouseCaptureLostEvent & aEvent);


	public:

		PaneContainer(
			FrameDockManager & aManager,
			Layout::PaneNode & aPaneNode,
			wxWindow * aParent,
			wxWindow * aClientWindow,
			const wxString & aCaption
		);

		// Getters:
		wxWindow * clientWindow() const { return mClientWindow; }
		Layout::PaneNode & paneNode() const { return mPaneNode; }
	};




}  // namespace WxDockUI::Internal
