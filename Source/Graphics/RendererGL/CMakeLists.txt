add_library(RendererGL)

plasma_setup_library(RendererGL ${CMAKE_CURRENT_LIST_DIR} TRUE)
plasma_use_precompiled_header(RendererGL ${CMAKE_CURRENT_LIST_DIR})

target_sources(RendererGL
  PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}/OpenglRenderer.cpp
      ${CMAKE_CURRENT_LIST_DIR}/OpenglRenderer.hpp
      ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
      ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
      ${CMAKE_CURRENT_LIST_DIR}/RendererGLStandard.hpp
      ${CMAKE_CURRENT_LIST_DIR}/RendererGLStandard.cpp  
      ${CMAKE_CURRENT_LIST_DIR}/OpenglRendererWindows.cpp
)

plasma_target_includes(RendererGL
  PUBLIC
    Common
    Support
)

target_link_libraries(RendererGL
  PUBLIC
    GL
    Glew
    tracy
)