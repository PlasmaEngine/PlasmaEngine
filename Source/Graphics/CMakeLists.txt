# Libraries.

add_subdirectory(GraphicsRuntime)
add_subdirectory(RendererBGFX)

set_property(TARGET "GraphicsRuntime" PROPERTY FOLDER "Graphics")
set_property(TARGET "RendererBGFX" PROPERTY FOLDER "Graphics")