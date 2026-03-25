#include <WxDockUI/Internal/DockOverlay.h>
#include <WxDockUI/Internal/PaneContainer.h>
#include <WxDockUI/FrameDockManager.h>
#include <wx/dcbuffer.h>






namespace WxDockUI::Internal
{




	using namespace WxDockUI;





	DockOverlay::DockOverlay(FrameDockManager & aFrameDockManager):
		Super(
			nullptr,
			wxID_ANY,
			wxEmptyString,
			wxDefaultPosition,
			wxDefaultSize,
			wxFRAME_NO_TASKBAR | wxFRAME_TOOL_WINDOW | wxSTAY_ON_TOP | wxBORDER_NONE
		),
		mFrameDockManager(aFrameDockManager)
	{
		Bind(wxEVT_PAINT, &DockOverlay::onPaint, this);
		SetBackgroundStyle(wxBG_STYLE_PAINT);
		SetTransparent(180);   // Slight transparency
		Hide();
	}





	void DockOverlay::setCurrentDragNode(const Layout::BaseNode * aCurrentDragNode)
	{
		mCurrentDragNode = aCurrentDragNode;
		recalcIconRects();
	}





	void DockOverlay::showOverlay(bool aShow)
	{
		if (aShow)
		{
			auto frame = mFrameDockManager.frame();
			const auto client = frame->GetClientRect();
			const auto screenPos = frame->ClientToScreen(client.GetTopLeft());
			SetSize(wxRect(screenPos, client.GetSize()));
			Show();
			Raise();
		}
		else
		{
			Hide();
			mHoveredPane = nullptr;
			mIconRects.clear();
		}
	}





	void DockOverlay::updateMousePosition(const wxPoint & aScreenPos)
	{
		mCurrentMouseScreenPos = aScreenPos;
		updateHoveredPane(aScreenPos);
		recalcIconRects();
		Refresh();
	}





	void DockOverlay::onPaint(wxPaintEvent &)
	{
		wxAutoBufferedPaintDC dc(this);
		dc.Clear();

		dc.SetBrush(wxBrush(wxColour(0, 120, 215, 180)));
		dc.SetPen(*wxTRANSPARENT_PEN);

		for (const auto & [type, rect]: mIconRects)
		{
			wxRect r(this->ScreenToClient(rect.GetLeftTop()), rect.GetSize());
			dc.DrawRoundedRectangle(r, 6);
		}
	}





	DockTarget DockOverlay::hitTest(const wxPoint & aScreenPos) const
	{
		DockTarget result;
		for (const auto & [kind, rect]: mIconRects)
		{
			if (rect.Contains(aScreenPos))
			{
				result.mKind = kind;
				result.mPane = mHoveredPane;
				return result;
			}
		}
		return result;
	}





	void DockOverlay::updateHoveredPane(const wxPoint & aScreenPos)
	{
		mHoveredPane = mFrameDockManager.layoutEngine().paneNodeAtScreenPos(aScreenPos);
		if (mHoveredPane == nullptr)
		{
			return;
		}

		auto window = mFrameDockManager.findPaneWindow(*mHoveredPane);
		if (window == nullptr)
		{
			assert(!"Invalid pane window");
			mHoveredPane = nullptr;
			return;
		}
		auto paneRect = window->GetRect();
		auto paneScreen = window->GetParent()->ClientToScreen(paneRect.GetTopLeft());
		mHoveredPaneRectScreen = wxRect(paneScreen, paneRect.GetSize());
	}





	void DockOverlay::recalcIconRects()
	{
		mIconRects.clear();

		// Root icons:
		const wxRect frameRect = GetRect();
		const int cx = frameRect.GetWidth() / 2;
		const int cy = frameRect.GetHeight() / 2;
		mIconRects[DockTarget::Kind::RootSplitLeft] =
			wxRect(
				frameRect.x + ROOT_MARGIN,
				frameRect.y + cy - ICON_SIZE / 2,
				ICON_SIZE,
				ICON_SIZE
			);
		mIconRects[DockTarget::Kind::RootSplitRight] =
			wxRect(
				frameRect.GetRight() - ROOT_MARGIN - ICON_SIZE,
				frameRect.y + cy - ICON_SIZE / 2,
				ICON_SIZE,
				ICON_SIZE
			);
		mIconRects[DockTarget::Kind::RootSplitTop] =
			wxRect(
				frameRect.x + cx - ICON_SIZE / 2,
				frameRect.y + ROOT_MARGIN,
				ICON_SIZE,
				ICON_SIZE
			);
		mIconRects[DockTarget::Kind::RootSplitBottom] =
			wxRect(
				frameRect.x + cx - ICON_SIZE / 2,
				frameRect.GetBottom() - ROOT_MARGIN - ICON_SIZE,
				ICON_SIZE,
				ICON_SIZE
			);

		// Pane icons:
		if (mHoveredPane == nullptr)
		{
			return;
		}
		const wxRect r = mHoveredPaneRectScreen;
		const int pcx = r.x + r.width / 2;
		const int pcy = r.y + r.height / 2;
		mIconRects[DockTarget::Kind::PaneTab] = wxRect(pcx - ICON_SIZE / 2, pcy - ICON_SIZE / 2, ICON_SIZE, ICON_SIZE);
		if (
			(mHoveredPane != mCurrentDragNode) ||  // Dragging into another pane
			(
				(mHoveredPane->parent()->type() == Layout::NodeType::Tab)  &&  // Dragging into self, but self is within a tab node
				(mHoveredPane->parent()->asTabNode()->panes().size() > 1)      // Not the only child in the tab node
			)
		)
		{
			mIconRects[DockTarget::Kind::PaneSplitLeft]   = wxRect(pcx - 3 * ICON_SIZE / 2 - ICON_SPACING, pcy - ICON_SIZE / 2, ICON_SIZE, ICON_SIZE);
			mIconRects[DockTarget::Kind::PaneSplitRight]  = wxRect(pcx + ICON_SIZE / 2 + ICON_SPACING, pcy - ICON_SIZE / 2, ICON_SIZE, ICON_SIZE);
			mIconRects[DockTarget::Kind::PaneSplitTop]    = wxRect(pcx - ICON_SIZE / 2, pcy - 3 * ICON_SIZE / 2 - ICON_SPACING, ICON_SIZE, ICON_SIZE);
			mIconRects[DockTarget::Kind::PaneSplitBottom] = wxRect(pcx - ICON_SIZE / 2, pcy + ICON_SIZE / 2 + ICON_SPACING, ICON_SIZE, ICON_SIZE);
		}
	}





}  // namespace WxDockUI::Internal
