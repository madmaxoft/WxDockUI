#pragma once

#include <wx/wx.h>

#include "Enums.hpp"





namespace WxDockUI
{

	/** ID used to identify individual panes in the DockSystem. */
	using PaneId = std::string;





	/** Provides additional information about a pane, used when registering the pane for inclusion in a DockSystem. */
	class PaneInfo
	{
		wxString mCaption;

		DockPosition mInitialDock = DockPosition::Left;

		bool mIsClosable = true;
		bool mIsFloatable = true;
		bool mIsMovable = true;
		bool mIsVisible = true;

		int mBestWidth = -1;
		int mBestHeight = -1;


	public:

		PaneInfo() = default;

		// Chainable mutators:
		PaneInfo & caption(const wxString & aCaption);
		PaneInfo & left();
		PaneInfo & right();
		PaneInfo & bottom();
		PaneInfo & top();
		PaneInfo & center();
		PaneInfo & bestSize(int aWidth, int aHeight);

		// Getters:
		const wxString & caption() const { return mCaption; }
		DockPosition initialDock() const { return mInitialDock; }
		int bestWidth() const { return mBestWidth; }
		int bestHeight() const { return mBestHeight; }
	};

}  // namespace WxDockUI
