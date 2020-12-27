if(NOT EMSCRIPTEN)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
  set(BUILD_CLAR OFF CACHE BOOL "")

  add_subdirectory(Repo)

  add_library(Libgit2 INTERFACE)

  target_link_libraries(Libgit2
    INTERFACE
      git2
  )

set_target_properties( git2 PROPERTIES FOLDER "External/Libgit2" )
set_target_properties( git2internal PROPERTIES FOLDER "External/Libgit2" )
set_target_properties( http-parser PROPERTIES FOLDER "External/Libgit2" )
set_target_properties( pcre PROPERTIES FOLDER "External/Libgit2" )
set_target_properties( Libgit2 PROPERTIES FOLDER "External/Libgit2" )
endif()
