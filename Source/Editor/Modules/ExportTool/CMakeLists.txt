add_library(ExportTool)

plasma_setup_library(ExportTool ${CMAKE_CURRENT_LIST_DIR} TRUE)
plasma_use_precompiled_header(ExportTool ${CMAKE_CURRENT_LIST_DIR})

target_sources(ExportTool
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/EmscriptenExportTarget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Exporter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Exporter.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ExportToolStandard.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ExportToolStandard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
    ${CMAKE_CURRENT_LIST_DIR}/WindowsExportTarget.cpp
)

plasma_target_includes(ExportTool
  PUBLIC
    Common
    LightningCore
    Engine
    LightningShaders
)

set_property(TARGET "ExportTool" PROPERTY FOLDER "Editor/Modules")