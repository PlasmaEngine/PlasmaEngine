if(NOT EMSCRIPTEN)
	set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
	set(BUILD_CLAR OFF CACHE BOOL "")

	add_subdirectory(Repo)

	add_library(Libgit2 INTERFACE)

	target_link_libraries(Libgit2
		INTERFACE
	 	git2
	)

	# TODO: Investigate
	# some targets break linux biulds 
	if(WIN32) 
		set_property(TARGET "git2" PROPERTY FOLDER "External/Libgit2")
		set_property(TARGET "git2internal" PROPERTY FOLDER "External/Libgit2")
		set_property(TARGET "http-parser" PROPERTY FOLDER "External/Libgit2")
		set_property(TARGET "pcre" PROPERTY FOLDER "External/Libgit2")
	endif()
endif()
