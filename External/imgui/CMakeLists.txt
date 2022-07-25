add_library(IMGUI)

plasma_setup_library(IMGUI ${CMAKE_CURRENT_LIST_DIR} FALSE)

target_sources(IMGUI
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/Repo/imgui.h
  ${CMAKE_CURRENT_LIST_DIR}/Repo/imgui.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Repo/imgui_draw.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Repo/imgui_demo.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Repo/imgui_internal.h
  ${CMAKE_CURRENT_LIST_DIR}/Repo/imgui_tables.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Repo/imgui_widgets.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Repo/backends/imgui_impl_opengl3.h
  ${CMAKE_CURRENT_LIST_DIR}/Repo/backends/imgui_impl_opengl3.cpp
)

if(WIN32)
target_sources(IMGUI
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/Repo/backends/imgui_impl_win32.h
  ${CMAKE_CURRENT_LIST_DIR}/Repo/backends/imgui_impl_win32.cpp
)
endif()

target_include_directories(IMGUI
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Repo/
    ${CMAKE_CURRENT_LIST_DIR}/Repo/backends
)

set_target_properties(IMGUI PROPERTIES FOLDER "External/ImGui" )
