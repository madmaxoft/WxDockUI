#pragma once

#include <map>

#include <wx/frame.h>

#include <WxDockUI/Internal/DockTarget.h>





// fwd:
namespace WxDockUI
{
	class FrameDockManager;
	namespace Layout
	{
		class BaseNode;
		class PaneNode;
	}
}





namespace WxDockUI::Internal
{





	class DockOverlay final:
		public wxFrame
	{
		using Super = wxFrame;

		static constexpr int ICON_SIZE = 32;
		static constexpr int ICON_SPACING = 4;
		static constexpr int ROOT_MARGIN = 20;


		FrameDockManager & mFrameDockManager;

		wxPoint mCurrentMouseScreenPos;

		const Layout::BaseNode * mCurrentDragNode = nullptr;
		const Layout::BaseNode * mHoveredNode = nullptr;
		wxRect mHoveredNodeRectScreen;

		/** Icon rects in screen coordinates */
		std::map<DockTarget::Kind, wxRect> mIconRects;


	public:

		explicit DockOverlay(FrameDockManager & aFrameDockManager);

		/** Notifies about the layout node that is currently being dragged.
		Used to remove targets within the currently dragged node. */
		void setCurrentDragNode(const Layout::BaseNode * aCurrentDragNode);

		/** Shows or hides the overlay. */
		void showOverlay(bool aShow);

		/** Updates the overlay based on the specified mouse position. */
		void updateMousePosition(const wxPoint & aScreenPos);

		/** Returns the dock target currently under the specified position. */
		DockTarget hitTest(const wxPoint & aScreenPos) const;


	private:

		void onPaint(wxPaintEvent &);

		void updateHoveredPane(const wxPoint & aScreenPos);
		void recalcIconRects();
	};





}  // namespace WxDockUI::Internal
