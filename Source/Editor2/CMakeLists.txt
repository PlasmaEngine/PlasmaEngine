add_executable(NewPlasmaEditor)

plasma_setup_library(NewPlasmaEditor ${CMAKE_CURRENT_LIST_DIR} TRUE)
plasma_use_precompiled_header(NewPlasmaEditor ${CMAKE_CURRENT_LIST_DIR})

target_sources(NewPlasmaEditor
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/GameOrEditorStartup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GameOrEditorStartup.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
    ${CMAKE_CURRENT_LIST_DIR}/PlasmaCrashCallbacks.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PlasmaCrashCallbacks.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RC/PlasmaEditor.rc
)

if(APPLE)
    set(NetworkingLibs)
else()
    set(NetworkingLibs NetworkCore Replication)
endif()

target_link_libraries(NewPlasmaEditor
  PUBLIC
    assimp
    CodeTranslator
    Common
    Content
    EditorCore
    ExportTool
    Engine
    FreeType
    Gameplay
    Geometry
    GraphicsRuntime
    RendererGL
    Libpng
    Meta
    Nvtt
    ${NetworkingLibs}
    Opus
    Physics
    Platform
    Scintilla
    Serialization
    Sound
    SpatialPartition
    SpirvCross
    SpirvHeaders
    SpirvTools
    Startup
    Support
    UiWidget
    Widget
    ZLib
    LightningCore
    LightningScript
    LightningShaders
    tracy
    IMGUI
)

target_compile_definitions(NewPlasmaEditor PUBLIC TRACY_IMPORTS)

plasma_copy_from_linked_libraries(NewPlasmaEditor)