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
      ${CMAKE_CURRENT_LIST_DIR}/VulkanValidationLayers.hpp
      ${CMAKE_CURRENT_LIST_DIR}/VulkanInstance.cpp
      ${CMAKE_CURRENT_LIST_DIR}/VulkanSurface.cpp
      ${CMAKE_CURRENT_LIST_DIR}/VulkanDevice.cpp
      ${CMAKE_CURRENT_LIST_DIR}/VulkanDevice.hpp
      ${CMAKE_CURRENT_LIST_DIR}/VulkanCommandPool.cpp
      ${CMAKE_CURRENT_LIST_DIR}/VulkanSyncronization.cpp
      ${CMAKE_CURRENT_LIST_DIR}/VulkanSyncronization.hpp
      ${CMAKE_CURRENT_LIST_DIR}/VulkanMemory.cpp
      ${CMAKE_CURRENT_LIST_DIR}/VulkanMemory.hpp
      ${CMAKE_CURRENT_LIST_DIR}/VulkanImage.cpp
      ${CMAKE_CURRENT_LIST_DIR}/VulkanImage.hpp
      ${CMAKE_CURRENT_LIST_DIR}/VulkanBuffer.cpp
      ${CMAKE_CURRENT_LIST_DIR}/VulkanBuffer.hpp
      ${CMAKE_CURRENT_LIST_DIR}/VulkanType.hpp
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