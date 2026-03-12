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
	Can provide a caption, close button, etc., based on the settings.
	Each PaneNode has an instance of this class, tracked and owned by a LayoutEngine. */
	class PaneContainer final:
		public wxPanel
	{
		using Super = wxPanel;

		/** The movement threshold before a drag operation is started. */
		static constexpr int DRAG_THRESHOLD_PIXELS = 5;

		FrameDockManager & mFrameDockManager;

		/** The layout's PaneNode being represented by this UI container. */
		const Layout::PaneNode & mPaneNode;

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

		/** Creates a new container and reparents the client window into it. */
		PaneContainer(
			FrameDockManager & aFrameDockManager,
			const Layout::PaneNode & aPaneNode,
			wxWindow * aParent,
			wxWindow * aClientWindow,
			const wxString & aCaption
		);

		/** Destroys the instance, reparenting the client window back into mFrameDockManager's frame. */
		~PaneContainer();

		// Getters:
		wxWindow * clientWindow() const { return mClientWindow; }
		const Layout::PaneNode & paneNode() const { return mPaneNode; }
	};




}  // namespace WxDockUI::Internal
