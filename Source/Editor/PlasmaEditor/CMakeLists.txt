add_executable(PlasmaEditor)

plasma_setup_library(PlasmaEditor ${CMAKE_CURRENT_LIST_DIR} TRUE)
plasma_use_precompiled_header(PlasmaEditor ${CMAKE_CURRENT_LIST_DIR})

target_sources(PlasmaEditor
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/VirtualFileSystem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GameOrEditorStartup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GameOrEditorStartup.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
    ${CMAKE_CURRENT_LIST_DIR}/PlasmaCrashCallbacks.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PlasmaCrashCallbacks.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RC/PlasmaEditor.rc
)

target_link_libraries(PlasmaEditor
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
    NetworkCore
    Nvtt
    Opus
    Physics
    Platform
    Replication
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
)

target_compile_definitions(PlasmaEditor PUBLIC TRACY_IMPORTS)

plasma_copy_from_linked_libraries(PlasmaEditor)