add_library(RendererVK)

plasma_setup_library(RendererVK ${CMAKE_CURRENT_LIST_DIR} TRUE)
plasma_use_precompiled_header(RendererVK ${CMAKE_CURRENT_LIST_DIR})

target_sources(RendererVK
  PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}/VulkanRenderer.cpp
      ${CMAKE_CURRENT_LIST_DIR}/VulkanRenderer.hpp
      ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
      ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
      ${CMAKE_CURRENT_LIST_DIR}/RendererVKStandard.hpp
      ${CMAKE_CURRENT_LIST_DIR}/RendererVKStandard.cpp
)

plasma_target_includes(RendererVK
  PUBLIC
    Common
    Support
)

target_link_libraries(RendererVK
  PUBLIC
    tracy
)

if (NOT WIN32)
  target_link_libraries(RendererVK
    PUBLIC
      SDL
  )
endif()