cmake_minimum_required(VERSION 3.16)

project(WxDockUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(srcs
	Src/DockOverlay.cpp
	Src/DockSystem.cpp
	Src/FrameDockManager.cpp
	Src/DragGhost.cpp
	Src/Layout.cpp
	Src/LayoutEngine.cpp
	Src/LayoutOps.cpp
	Src/PaneContainer.cpp
	Src/PaneInfo.cpp
	Src/PaneDragController.cpp
	Src/SplitContainer.cpp
	Src/TabContainer.cpp
	Src/ZOrderTracker.cpp
)

set(hdrs
	# Interface headers:
	Include/WxDockUI/DockSystem.hpp
	Include/WxDockUI/Enums.hpp
	Include/WxDockUI/PaneInfo.hpp

	# Internal headers:
	Include/WxDockUI/Internal/DockOverlay.hpp
	Include/WxDockUI/Internal/DockTarget.hpp
	Include/WxDockUI/Internal/DragGhost.hpp
	Include/WxDockUI/Internal/FrameDockManager.hpp
	Include/WxDockUI/Internal/Layout.hpp
	Include/WxDockUI/Internal/LayoutEngine.hpp
	Include/WxDockUI/Internal/LayoutOps.hpp
	Include/WxDockUI/Internal/PaneContainer.hpp
	Include/WxDockUI/Internal/PaneDragController.hpp
	Include/WxDockUI/Internal/SplitContainer.hpp
	Include/WxDockUI/Internal/TabContainer.hpp
	Include/WxDockUI/Internal/ZOrderTracker.hpp
)

# NOTE: shared library is disabled for now only for development comfort (QTCreator complains about multiple
# contexts for files). It should be perfectly fine re-enabling it.
#[[
add_library(WxDockUI SHARED ${srcs} ${hdrs})
target_link_libraries(WxDockUI PUBLIC ${wxWidgets_LIBRARIES})
target_include_directories(WxDockUI PUBLIC include)
]]

add_library(WxDockUI-static STATIC ${srcs} ${hdrs})
target_link_libraries(WxDockUI-static PUBLIC ${wxWidgets_LIBRARIES})
target_include_directories(WxDockUI-static PUBLIC Include)




# Debugging toggles:
# If enabled, object creation and deletion will get dumped to std::cout:
target_compile_definitions(WxDockUI-static PRIVATE WXDOCKUI_DEBUG_LIFETIME WXDOCKUI_DUMP_LAYOUT_ON_DOCK)

# If enabled, the FrameDockManager dumps its layout before and after performing a dock:
# target_compile_definitions(WxDockUI-static PRIVATE WXDOCKUI_DUMP_LAYOUT_ON_DOCK)
