add_library(RendererBGFX)

plasma_setup_library(RendererBGFX ${CMAKE_CURRENT_LIST_DIR} TRUE)
plasma_use_precompiled_header(RendererBGFX ${CMAKE_CURRENT_LIST_DIR})

target_sources(RendererBGFX
  PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}/BGFXRenderer.cpp
      ${CMAKE_CURRENT_LIST_DIR}/BGFXRenderer.hpp
      ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
      ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
      ${CMAKE_CURRENT_LIST_DIR}/RendererBGFXStandard.hpp
      ${CMAKE_CURRENT_LIST_DIR}/RendererBGFXStandard.cpp
)

plasma_target_includes(RendererBGFX
  PUBLIC
    Common
    Support
    bgfx
)

target_link_libraries(RendererBGFX
  PUBLIC
    bgfx 
    tracy
)