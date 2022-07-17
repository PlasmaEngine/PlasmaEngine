add_definitions(-DPlasmaCompilerClang=1 -DPlasmaCompilerName="Clang")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(PLASMA_C_CXX_FLAGS "\
  -Wno-address-of-packed-member\
  -Wno-empty-body\
  -fexceptions\
  -frtti\
  -fno-vectorize\
  -fno-slp-vectorize\
  -fno-tree-vectorize\
  -pthread\
")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Xlinker /ignore:4049,4217")
if(APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -c")
else()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Xlinker --start-group -Wunused-command-line-argument")
endif()

set(PLASMA_C_CXX_EXTERNAL_FLAGS -Wno-everything)

function(plasma_toolchain_setup_library target)
#    target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>)
#
#    target_link_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>)
endfunction()

function(plasma_use_precompiled_header target directory)
    #add_library(${target}PrecompiledHeader)
    #s
    #target_sources(${target}PrecompiledHeader
    #  PRIVATE
    #    ${directory}/Precompiled.hpp
    #    ${directory}/Precompiled.cpp
    #)
    #
    #set_source_files_properties(${directory}/Precompiled.hpp PROPERTIES
    #  COMPILE_FLAGS "-xc++-header -c"
    #  LANGUAGE CXX
    #)
    #
    #get_target_property(targetIncludeDirectories ${target} INCLUDE_DIRECTORIES)
    #set_target_properties(${target}PrecompiledHeader PROPERTIES INCLUDE_DIRECTORIES "${targetIncludeDirectories}")
    #
    #get_target_property(binaryDir "${target}PrecompiledHeader" BINARY_DIR)
    #
    #set_target_properties(${target} PROPERTIES COMPILE_FLAGS "-include-pch ${binaryDir}/CMakeFiles/${target}PrecompiledHeader.dir/Precompiled.hpp.o")
    #
    #add_dependencies(${target} ${target}PrecompiledHeader)
endfunction()

function(plasma_source_ignore_precompiled_header source)
endfunction()