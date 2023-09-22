add_library(RendererGL)

plasma_setup_library(RendererGL ${CMAKE_CURRENT_LIST_DIR} TRUE)
plasma_use_precompiled_header(RendererGL ${CMAKE_CURRENT_LIST_DIR})

if(${PLASMA_PLATFORM} STREQUAL "Windows")
  set(RendererPlatformSource 
    ${CMAKE_CURRENT_LIST_DIR}/Windows/OpenglRendererWindows.cpp
  )
elseif(${PLASMA_PLATFORM} STREQUAL "Emscripten")
  set(RendererPlatformSource 
    ${CMAKE_CURRENT_LIST_DIR}/Emscripten/OpenglRendererEmscripten.cpp
  )
elseif(${PLASMA_PLATFORM} STREQUAL "Linux")
  set(RendererPlatformSource 
    ${CMAKE_CURRENT_LIST_DIR}/SDLSTDEmpty/OpenglRendererSDL.cpp
  )
else()
  message(FATAL_ERROR "No renderer available for target '${PLASMA_PLATFORM}'.")
endif()


target_sources(RendererGL
  PRIVATE
      ${RendererPlatformSource}
      ${CMAKE_CURRENT_LIST_DIR}/OpenglRenderer.cpp
      ${CMAKE_CURRENT_LIST_DIR}/OpenglRenderer.hpp
      ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
      ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
      ${CMAKE_CURRENT_LIST_DIR}/RendererGLStandard.hpp
      ${CMAKE_CURRENT_LIST_DIR}/RendererGLStandard.cpp  
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
    IMGUI
)

if (NOT WIN32)
  target_link_libraries(RendererGL
    PUBLIC
      SDL
  )
endif()