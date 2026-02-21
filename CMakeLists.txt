cmake_minimum_required(VERSION 3.16)

project(WxDockUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(srcs
	Src/DockOverlay.cpp
	Src/DockSystem.cpp
	Src/FrameDockManager.cpp
	Src/GhostFrame.cpp
	Src/Layout.cpp
	Src/LayoutEngine.cpp
	Src/LayoutOps.cpp
	Src/PaneContainer.cpp
	Src/PaneDragController.cpp
	Src/TabContainerWindow.cpp
)

set(hdrs
	# Interface headers:
	Include/WxDockUI/DockSystem.h
	Include/WxDockUI/Enums.h
	Include/WxDockUI/FrameDockManager.h
	Include/WxDockUI/WxDockUI.h

	# Internal headers:
	Include/WxDockUI/Internal/DockOverlay.h
	Include/WxDockUI/Internal/DockTarget.h
	Include/WxDockUI/Internal/GhostFrame.h
	Include/WxDockUI/Internal/Layout.h
	Include/WxDockUI/Internal/LayoutEngine.h
	Include/WxDockUI/Internal/LayoutOps.h
	Include/WxDockUI/Internal/PaneContainer.h
	Include/WxDockUI/Internal/PaneDragController.h
	Include/WxDockUI/Internal/TabContainerWindow.h
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
