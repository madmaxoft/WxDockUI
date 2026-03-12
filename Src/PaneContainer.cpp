#include <WxDockUI/Internal/PaneContainer.h>
#include <WxDockUI/FrameDockManager.h>





namespace WxDockUI::Internal
{






	void PaneContainer::onMouseLeftDown(wxMouseEvent & aEvent)
	{
		mDragStartPos = aEvent.GetPosition();
		mIsDragging = false;

		CaptureMouse();
		aEvent.Skip();
	}





	void PaneContainer::onMouseMotion(wxMouseEvent & aEvent)
	{
		if (!HasCapture())
		{
			return;
		}

		wxPoint pos = aEvent.GetPosition();
		wxPoint delta = pos - mDragStartPos;

		if (
			!mIsDragging &&
			((abs(delta.x) >= DRAG_THRESHOLD_PIXELS) || (abs(delta.y) >= DRAG_THRESHOLD_PIXELS))
		)
		{
			mIsDragging = true;
			mManager.paneDragController().beginDrag(&mPaneNode, wxGetMousePosition());
		}

		// Forward the updated mouse position in screen coordinates:
		if (mIsDragging)
		{
			mManager.paneDragController().updateDrag(&mPaneNode, wxGetMousePosition());
		}

		aEvent.Skip();
	}





	void PaneContainer::onMouseLeftUp(wxMouseEvent & aEvent)
	{
		if (HasCapture())
		{
			ReleaseMouse();
		}

		if (mIsDragging)
		{
			mIsDragging = false;
			mManager.paneDragController().endDrag(&mPaneNode, wxGetMousePosition());
		}

		aEvent.Skip();
	}





	void PaneContainer::onMouseCaptureLost(wxMouseCaptureLostEvent & aEvent)
	{
		if (mIsDragging)
		{
			mManager.paneDragController().cancelDrag(&mPaneNode);
			mIsDragging = false;
		}
	}





	PaneContainer::PaneContainer(
		FrameDockManager & aManager,
		const Layout::PaneNode & aPaneNode,
		wxWindow * aParent,
		wxWindow * aClientWindow,
		const wxString & aCaption
	):
		wxPanel(aParent),
		mManager(aManager),
		mPaneNode(aPaneNode),
		mClientWindow(aClientWindow)
	{
		#ifndef NDEBUG
			std::cout << "Creating a PaneContainer for pane " << aPaneNode.paneId() << std::endl;
		#endif
		auto * rootSizer = new wxBoxSizer(wxVERTICAL);
		mCaptionBar = new wxPanel(this);
		mCaptionBar->SetMinSize(wxSize(-1, 22));
		mCaptionBar->Bind(wxEVT_LEFT_DOWN,          &PaneContainer::onMouseLeftDown, this);
		mCaptionBar->Bind(wxEVT_MOTION,             &PaneContainer::onMouseMotion, this);
		mCaptionBar->Bind(wxEVT_LEFT_UP,            &PaneContainer::onMouseLeftUp, this);
		mCaptionBar->Bind(wxEVT_MOUSE_CAPTURE_LOST, &PaneContainer::onMouseCaptureLost, this);
		auto * captionSizer = new wxBoxSizer(wxHORIZONTAL);
		mCaptionText = new wxStaticText(mCaptionBar, wxID_ANY, aCaption);
		mCaptionText->Bind(wxEVT_LEFT_DOWN,          &PaneContainer::onMouseLeftDown, this);
		mCaptionText->Bind(wxEVT_MOTION,             &PaneContainer::onMouseMotion, this);
		mCaptionText->Bind(wxEVT_LEFT_UP,            &PaneContainer::onMouseLeftUp, this);
		mCaptionText->Bind(wxEVT_MOUSE_CAPTURE_LOST, &PaneContainer::onMouseCaptureLost, this);
		captionSizer->Add(mCaptionText, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 4);
		mCaptionBar->SetSizer(captionSizer);
		mClientWindow->Reparent(this);
		rootSizer->Add(mCaptionBar, 0, wxEXPAND);
		rootSizer->Add(mClientWindow, 1, wxEXPAND);
		SetSizer(rootSizer);
		Bind(wxEVT_MOTION,  &PaneContainer::onMouseMotion, this);
		Bind(wxEVT_LEFT_UP, &PaneContainer::onMouseLeftUp, this);
	}





	PaneContainer::~PaneContainer()
	{
		#ifndef NDEBUG
			std::cout << "Deleting a PaneContainer for pane " << mPaneNode.paneId() << std::endl;
		#endif
	}





}  // namespace WxDockUI::Internal
