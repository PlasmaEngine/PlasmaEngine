# Libraries.

add_subdirectory(GraphicsRuntime)
add_subdirectory(RendererGL)

set_property(TARGET "GraphicsRuntime" PROPERTY FOLDER "Graphics")
set_property(TARGET "RendererGL" PROPERTY FOLDER "Graphics")