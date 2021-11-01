add_definitions(-DPlasmaCompilerGcc=1 -DPlasmaCompilerName="GCC")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

function(plasma_toolchain_setup_library target)
endfunction()

function(plasma_use_precompiled_header target directory)
endfunction()

function(plasma_source_ignore_precompiled_header source)
endfunction()
