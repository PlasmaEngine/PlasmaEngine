# Libraries.

add_subdirectory(GraphicsRuntime)
add_subdirectory(RendererVK)

set_property(TARGET "GraphicsRuntime" PROPERTY FOLDER "Graphics")
set_property(TARGET "RendererVK" PROPERTY FOLDER "Graphics")