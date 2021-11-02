add_definitions(-DPlasmaCompilerGcc=1 -DPlasmaCompilerName="GCC")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

 set(PLASMA_C_CXX_FLAGS "\
    -Wno-unused\
    -Wno-sign-compare\
    -Wno-expansion-to-defined\
    -Winline\
    -ffast-math\
    -fanalyzer\
 ")


# set(PLASMA_C_CXX_FLAGS_DEBUG "\
#   -Z\
#   -MD\
#   -S\
#   -Od\
#   -Ob0\
#   -Oy-\
# ")

# set(PLASMA_C_CXX_FLAGS_RELWITHDEBINFO "\
#   -Z\
#   -MT\
#   -MP\
#   -GS\
#   -O2\
#   -Oi\
#   -Oy-\
# ")

# set(PLASMA_C_CXX_FLAGS_RELEASE "\
#   -MT\
#   -MP\
#   -GL\
#   -GS-\
#   -O2\
#   -Oi\
# ")

# set(PLASMA_C_CXX_FLAGS_MINSIZEREL "\
#   -MT\
#   -MP\
#   -GL\
#   -GS-\
#   -O1\
# ")

#set(PLASMA_LINKER_FLAGS "/ignore:4099,4221,4075,4251")
#set(PLASMA_LINKER_FLAGS_RELEASE "/LTCG")
)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}\
  -lstdc++fs\
")

function(plasma_toolchain_setup_library target)
endfunction()

function(plasma_use_precompiled_header target directory)
endfunction()

function(plasma_source_ignore_precompiled_header source)
endfunction()
