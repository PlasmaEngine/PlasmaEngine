add_library(LightningScript)

plasma_setup_library(LightningScript ${CMAKE_CURRENT_LIST_DIR} TRUE)
plasma_use_precompiled_header(LightningScript ${CMAKE_CURRENT_LIST_DIR})

target_sources(LightningScript
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
    ${CMAKE_CURRENT_LIST_DIR}/LightningPlugin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LightningPlugin.hpp
    ${CMAKE_CURRENT_LIST_DIR}/LightningScript.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LightningScript.hpp
    ${CMAKE_CURRENT_LIST_DIR}/LightningScriptStandard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LightningScriptStandard.hpp
    ${CMAKE_CURRENT_LIST_DIR}/LightningPlasma.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LightningPlasma.hpp
)

plasma_target_includes(LightningScript
  PUBLIC
    Common
    tracy
    Geometry
    EditorCore
)
