add_subdirectory(Repo)

if(WIN32)
	set_property(TARGET "assimp" PROPERTY FOLDER "External/Assimp")
	set_property(TARGET "uninstall" PROPERTY FOLDER "External/Assimp")
	set_property(TARGET "UpdateAssimpLibsDebugSymbolsAndDLLs" PROPERTY FOLDER "External/Assimp")
endif()