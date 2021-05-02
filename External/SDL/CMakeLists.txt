set(SDL_SHARED_ENABLED_BY_DEFAULT OFF)


if(NOT PLASMA_TARGETOS STREQUAL "Windows")
  add_definitions(-DSDL_MAIN_AVAILABLE)
endif()

add_subdirectory(Repo)

add_library(SDL INTERFACE)

target_link_libraries(SDL
  INTERFACE
    SDL2-static
    SDL2main
)

set_target_properties( SDL2main PROPERTIES FOLDER "External/SDL" )
set_target_properties( SDL2-static PROPERTIES FOLDER "External/SDL" )
