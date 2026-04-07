#include <WxDockUI/DockSystem.hpp>

#include <format>

#include <WxDockUI/Internal/FrameDockManager.hpp>





namespace WxDockUI
{





	// Nothing explicit needed yet,
	// we just need the constructor to be defined in cpp file, so that unique_ptr<FrameDockManager> compiles.
	DockSystem::DockSystem() = default;





	// Nothing explicit needed yet,
	// we just need the destructor to be defined in cpp file, so that unique_ptr<FrameDockManager> compiles.
	DockSystem::~DockSystem() = default;





	void DockSystem::manageWindow(wxTopLevelWindow & aWindow)
		{
			// Check if the window is already managed:
			for (const auto & fdm: mManagedWindows)
			{
				if (fdm->frame() == &aWindow)
				{
					return;
				}
			}

			mManagedWindows.push_back(std::make_unique<Internal::FrameDockManager>(aWindow, *this));
		}





		void DockSystem::addPane(
			const wxTopLevelWindow & aParentDockWindow,
			wxWindow * aPaneWindow,
			const PaneId & aPaneId,
			const PaneInfo & aPaneInfo
		)
		{
			// Check if the pane is already present:
			auto itr = mPanes.find(aPaneId);
			if (itr != mPanes.end())
			{
				throw std::runtime_error(std::format("Pane {} already present", aPaneId));
			}

			// Add the pane, if the parent dock window is managed:
			for (const auto & fdm: mManagedWindows)
			{
				if (fdm->frame() == &aParentDockWindow)
				{
					mPanes[aPaneId] = {std::move(aPaneInfo), aPaneWindow};
					fdm->addPane(aPaneId, aPaneInfo);
					return;
				}
			}

			// Parent dock window not found:
			throw std::runtime_error("Cannot add pane, the ParentDockFrame is not managed");
		}





		wxWindow * DockSystem::findPaneWindow(const PaneId & aId) const
		{
			auto itr = mPanes.find(aId);
			if (itr == mPanes.end())
			{
				return nullptr;
			}
			return itr->second.second;
		}





		const PaneInfo * DockSystem::findPaneInfo(const PaneId & aId) const
		{
			auto itr = mPanes.find(aId);
			if (itr == mPanes.end())
			{
				return nullptr;
			}
			return &(itr->second.first);
		}

}  // namespace WxDockUI
