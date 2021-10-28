add_executable(PlasmaPlayerWindows)

plasma_setup_library(PlasmaPlayerWindows ${CMAKE_CURRENT_LIST_DIR} TRUE)
plasma_use_precompiled_header(PlasmaPlayerWindows ${CMAKE_CURRENT_LIST_DIR})

target_sources(PlasmaPlayerWindows
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/GameStartup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GameStartup.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
    ${CMAKE_CURRENT_LIST_DIR}/PlasmaCrashCallbacks.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PlasmaCrashCallbacks.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RC/PlasmaPlayerWIndow.rc
)

target_link_libraries(PlasmaPlayerWindows
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

target_compile_definitions(PlasmaPlayerWindows PUBLIC TRACY_IMPORTS)

plasma_copy_from_linked_libraries(PlasmaPlayerWindows)