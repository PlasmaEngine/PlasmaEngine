add_executable(PlasmaLauncherShell)

plasma_setup_library(PlasmaLauncherShell ${CMAKE_CURRENT_LIST_DIR} TRUE)
plasma_use_precompiled_header(PlasmaLauncherShell ${CMAKE_CURRENT_LIST_DIR})

target_sources(PlasmaLauncherShell
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/VirtualFileSystem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
)

target_link_libraries(PlasmaLauncherShell
  PUBLIC
    Common
    Platform
    ZLib
    Support
)

add_dependencies(PlasmaLauncherShell
  PlasmaLauncher
)

plasma_copy_from_linked_libraries(PlasmaLauncherShell)
