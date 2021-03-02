add_subdirectory(EditorCore)
add_subdirectory(PlasmaEditor)

set_property(TARGET "EditorCore" PROPERTY FOLDER "Editor")
set_property(TARGET "PlasmaEditor" PROPERTY FOLDER "Editor")

add_subdirectory(Modules)