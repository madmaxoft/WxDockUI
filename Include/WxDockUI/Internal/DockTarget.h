#pragma once

#include <string>
#include <format>
#include <cassert>

#include <WxDockUI/Enums.h>
#include <WxDockUI/Internal/Layout.h>





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

		const WxDockUI::Layout::BaseNode * mNode = nullptr;

		bool isValid() const
		{
			return (mKind != Kind::Invalid);
		}

		bool operator ==(const DockTarget & aOther) const
		{
			return (mKind == aOther.mKind) && (mNode == aOther.mNode);
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





		/** Returns whether the stored kind is a horizontal split. */
		bool isHorizontalSplit() const
		{
			return isHorizontalSplitKind(mKind);
		}





		/** Returns whether the specified kind is a horizontal split. */
		static bool isHorizontalSplitKind(Kind aKind)
		{
			return (
				(aKind == Kind::PaneSplitLeft) ||
				(aKind == Kind::PaneSplitRight) ||
				(aKind == Kind::RootSplitLeft) ||
				(aKind == Kind::RootSplitRight)
			);
		}





		/** Returns a human-readable description of the target, used for debugging dumps. */
		std::string describe() const
		{
			auto pane = mNode->asPaneNode();
			auto desc = (pane != nullptr) ? pane->paneId() : std::format("Node %p/%d", static_cast<const void *>(mNode), static_cast<int>(mNode->type()));
			switch (mKind)
			{
				case Kind::Invalid:         return "Invalid";
				case Kind::RootSplitTop:    return "RootSplitTop";
				case Kind::RootSplitLeft:   return "RootSplitLeft";
				case Kind::RootSplitBottom: return "RootSplitBottom";
				case Kind::RootSplitRight:  return "RootSplitRight";
				case Kind::PaneSplitTop:    return std::format("PaneSplitTop({})",    desc);
				case Kind::PaneSplitLeft:   return std::format("PaneSplitLeft({})",   desc);
				case Kind::PaneSplitBottom: return std::format("PaneSplitBottom({})", desc);
				case Kind::PaneSplitRight:  return std::format("PaneSplitRight({})",  desc);
				case Kind::PaneTab:         return std::format("PaneTab({})",         desc);
			}
			assert(!"Unknown target kind");
			return "Unknown target kind";
		}
	};





}  // namespace WxDockUI::Internal
