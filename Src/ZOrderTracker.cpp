#include <WxDockUI/Internal/ZOrderTracker.hpp>

#include <algorithm>

#include <WxDockUI/Internal/FrameDockManager.hpp>





namespace WxDockUI::Internal
{

	void ZOrderTracker::onActivate(wxActivateEvent & aEvent)
	{
		aEvent.Skip();
		auto mgr = managerFromEvent(aEvent);
		if (mgr == nullptr)
		{
			return;
		}
		if (aEvent.GetActive())
		{
			bringToFront(*mgr);
		}
	}





	void ZOrderTracker::onIconize(wxIconizeEvent & aEvent)
	{
		aEvent.Skip();
		auto mgr = managerFromEvent(aEvent);
		if (mgr == nullptr)
		{
			return;
		}
		if (aEvent.IsIconized())
		{
			// Push to bottom:
			auto itr = std::find(mZOrder.begin(), mZOrder.end(), mgr);
			if (itr != mZOrder.end())
			{
				mZOrder.erase(itr);
				mZOrder.push_back(mgr);
			}
		}
		else
		{
			bringToFront(*mgr);
		}
	}





	void ZOrderTracker::onClose(wxCloseEvent & aEvent)
	{
		aEvent.Skip();
		auto mgr = managerFromEvent(aEvent);
		if (mgr == nullptr)
		{
			return;
		}
		remove(*mgr);
	}





	FrameDockManager * ZOrderTracker::managerFromEvent(const wxEvent & aEvent)
	{
		auto * window = aEvent.GetEventObject();
		if (window == nullptr)
		{
			return nullptr;
		}
		for (auto mgr: mZOrder)
		{
			if (mgr->frame() == window)
			{
				return mgr;
			}
		}
		return nullptr;
	}





	void ZOrderTracker::add(FrameDockManager & aManager)
	{
		auto frame = aManager.frame();
		if (frame == nullptr)
		{
			return;
		}

		mZOrder.push_back(&aManager);

		frame->Bind(wxEVT_ACTIVATE, &ZOrderTracker::onActivate, this);
		frame->Bind(wxEVT_ICONIZE,  &ZOrderTracker::onIconize,  this);
		frame->Bind(wxEVT_CLOSE_WINDOW, &ZOrderTracker::onClose, this);
	}





	void ZOrderTracker::remove(FrameDockManager & aManager)
	{
		// Unbind the event handlers:
		wxTopLevelWindow * frame = aManager.frame();
		if (frame != nullptr)
		{
			frame->Unbind(wxEVT_ACTIVATE,     &ZOrderTracker::onActivate, this);
			frame->Unbind(wxEVT_ICONIZE,      &ZOrderTracker::onIconize,  this);
			frame->Unbind(wxEVT_CLOSE_WINDOW, &ZOrderTracker::onClose,    this);
		}

		// Remove from the ZOrder:
		auto itr = std::find(mZOrder.begin(), mZOrder.end(), &aManager);
		if (itr != mZOrder.end())
		{
			mZOrder.erase(itr);
		}
	}





	FrameDockManager * ZOrderTracker::hitTest(const wxPoint & aScreenPos) const
	{
		for (auto manager: mZOrder)
		{
			if (manager == nullptr)
			{
				continue;
			}
			auto frame = manager->frame();
			if (
				(frame == nullptr) ||
				!frame->IsShown() ||
				frame->IsIconized()
			)
			{
				continue;
			}
			if (frame->GetScreenRect().Contains(aScreenPos))
			{
				return manager;
			}
		}

		// Nothing hit:
		return nullptr;
	}





	void ZOrderTracker::bringToFront(FrameDockManager & aManager)
	{
		auto itr = std::find(mZOrder.begin(), mZOrder.end(), &aManager);
		if (itr == mZOrder.end())
		{
			assert(!"FrameDockManager not managed");
			return;
		}

		// Re-insert at the front:
		mZOrder.erase(itr);
		mZOrder.insert(mZOrder.begin(), &aManager);
	}

}  // namespace WxDockUI::Internal
