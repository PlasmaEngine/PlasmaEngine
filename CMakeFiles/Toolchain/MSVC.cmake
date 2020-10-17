add_definitions(-DPlasmaCompilerMsvc=1 -DPlasmaCompilerName="Msvc")

set(PLASMA_C_CXX_FLAGS "\
  -W3\
  -Zc:wchar_t\
  -Zc:inline\
  -Zc:forScope\
  -fp:fast\
  -errorReport:prompt\
  -Gd\
  -EHsc\
  -nologo\
  -analyze-\
  -bigobj\
")

set(PLASMA_C_CXX_FLAGS_DEBUG "\
  -Zi\
  -Gm\
  -MDd\
  -GS\
  -Od\
  -Ob0\
  -Oy-\
")

set(PLASMA_C_CXX_FLAGS_RELWITHDEBINFO "\
  -Zi\
  -Gm\
  -MT\
  -MP\
  -GS\
  -O2\
  -Oi\
  -Oy-\
")

set(PLASMA_C_CXX_FLAGS_RELEASE "\
  -Gm-\
  -MT\
  -MP\
  -GL\
  -GS-\
  -O2\
  -Oi\
")

set(PLASMA_C_CXX_FLAGS_MINSIZEREL "\
  -Gm-\
  -MT\
  -MP\
  -GL\
  -GS-\
  -O1\
")

set(PLASMA_LINKER_FLAGS "/ignore:4099,4221,4075,4251")
set(PLASMA_LINKER_FLAGS_RELEASE "/LTCG")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO /SUBSYSTEM:WINDOWS /STACK:8388608")

set(PLASMA_C_CXX_EXTERNAL_FLAGS /W0 /wd4267)

function(plasma_toolchain_setup_library target)
endfunction()

function(plasma_use_precompiled_header target directory)
  target_compile_options(${target} PRIVATE "/FIPrecompiled.hpp")
  set_source_files_properties(${directory}/Precompiled.cpp PROPERTIES COMPILE_FLAGS "/YcPrecompiled.hpp")
  set_target_properties(${target} PROPERTIES COMPILE_FLAGS "/YuPrecompiled.hpp")
endfunction()

function(plasma_source_ignore_precompiled_header source)
  set_source_files_properties(${source} PROPERTIES COMPILE_FLAGS "/Y-")
endfunction()
