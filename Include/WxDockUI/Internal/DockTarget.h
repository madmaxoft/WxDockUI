#pragma once

#include <WxDockUI/Enums.h>





// fwd:
namespace WxDockUI::Layout
{
	class PaneNode;
}





namespace WxDockUI::Internal
{





	/** Describes the target for a dock operation. */
	struct DockTarget
	{
		enum class Kind
		{
			Invalid,
			RootSplitTop,
			RootSplitLeft,
			RootSplitBottom,
			RootSplitRight,
			PaneSplitTop,
			PaneSplitLeft,
			PaneSplitBottom,
			PaneSplitRight,
			PaneTab
		} mKind = Kind::Invalid;

		WxDockUI::Layout::PaneNode * mPane = nullptr;

		bool isValid() const
		{
			return (mKind != Kind::Invalid);
		}

		bool operator ==(const DockTarget & aOther) const
		{
			return (mKind == aOther.mKind) && (mPane == aOther.mPane);
		}

		bool operator !=(const DockTarget & aOther) const
		{
			return !(*this == aOther);
		}



		/** Converts the stored Kind into a relevant DockPosition. */
		WxDockUI::DockPosition dockPosition() const
		{
			return dockPositionFromKind(mKind);
		}



		/** Converts the specified Kind into a relevant DockPosition. */
		static WxDockUI::DockPosition dockPositionFromKind(Kind aKind)
		{
			switch (aKind)
			{
				case Kind::RootSplitLeft:
				case Kind::PaneSplitLeft:
				{
					return WxDockUI::DockPosition::Left;
				}
				case Kind::RootSplitRight:
				case Kind::PaneSplitRight:
				{
					return WxDockUI::DockPosition::Right;
				}
				case Kind::RootSplitTop:
				case Kind::PaneSplitTop:
				{
					return WxDockUI::DockPosition::Top;
				}
				case Kind::RootSplitBottom:
				case Kind::PaneSplitBottom:
				{
					return WxDockUI::DockPosition::Bottom;
				}
				case Kind::PaneTab:
				{
					return WxDockUI::DockPosition::Center;
				}
				default:
				{
					return WxDockUI::DockPosition::Floating;
				}
			}
		}



		/** Returns whether the stored kind is a RootSplit. */
		bool isRootSplit() const
		{
			return isRootSplitKind(mKind);
		}



		/** Returns whether the specified kind is a RootSplit. */
		static bool isRootSplitKind(Kind aKind)
		{
			return (
				(aKind == Kind::RootSplitLeft)   ||
				(aKind == Kind::RootSplitRight)  ||
				(aKind == Kind::RootSplitTop)    ||
				(aKind == Kind::RootSplitBottom)
			);
		}



		/** Returns whether the stored kind is a PaneSplit. */
		bool isPaneSplit() const
		{
			return isPaneSplitKind(mKind);
		}



		/** Returns whether the specified kind is a PaneSplit. */
		static bool isPaneSplitKind(Kind aKind)
		{
			return (
				(aKind == Kind::PaneSplitLeft)   ||
				(aKind == Kind::PaneSplitRight)  ||
				(aKind == Kind::PaneSplitTop)    ||
				(aKind == Kind::PaneSplitBottom)
			);
		}
	};





}  // namespace WxDockUI::Internal
