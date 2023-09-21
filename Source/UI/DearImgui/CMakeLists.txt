add_library(DearImgui)

plasma_setup_library(DearImgui ${CMAKE_CURRENT_LIST_DIR} TRUE)
plasma_use_precompiled_header(DearImgui ${CMAKE_CURRENT_LIST_DIR})

target_sources(DearImgui
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/DearImgui.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DearImgui.hpp
    ${CMAKE_CURRENT_LIST_DIR}/DearImguiStandard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DearImguiStandard.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
)

plasma_target_includes(DearImgui
  PUBLIC
    Common
    Engine
    GraphicsRuntime
    IMGUI
)