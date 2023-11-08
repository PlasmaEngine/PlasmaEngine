cmake_minimum_required(VERSION 3.12.1 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CONFIGURATION_TYPES Debug RelWithDebInfo Release MinSizeRel)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT PlasmaEditor)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_program(PLASMA_IWYU_PATH NAMES include-what-you-use iwyu)
if(PLASMA_IWYU_PATH)
  message(STATUS "Found iwyu at ${PLASMA_IWYU_PATH}")
endif()

find_program(PLASMA_CCACHE_PATH ccache)
if(PLASMA_CCACHE_PATH)
  message(STATUS "Found ccache at ${PLASMA_CCACHE_PATH}")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${PLASMA_CCACHE_PATH}")
endif()

if(APPLE)
	set(HAVE_FLAG_SEARCH_PATHS_FIRST 0)
endif()

project(Plasma)

if(APPLE)
	set(CMAKE_CXX_LINK_FLAGS "")
endif()

set(PLASMA_CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(PLASMA_CMAKE_DIR ${PLASMA_CORE_DIR}/CMakeFiles/)
set(PLASMA_TOOLCHAIN_DIR ${PLASMA_CMAKE_DIR}/Toolchain/)
set(PLASMA_TARGETOS_DIR ${PLASMA_CMAKE_DIR}/TargetOs/)
set(PLASMA_LIBRARIES_DIR ${PLASMA_CORE_DIR}/Source/)
set(PLASMA_PLATFORM_DATA_DIR ${PLASMA_CORE_DIR}/PlatformData/${PLASMA_TOOLCHAIN})
 

include(${PLASMA_CMAKE_DIR}/PlasmaGit.cmake)
include(${PLASMA_CMAKE_DIR}/PlasmaVersion.cmake)
include(${PLASMA_CMAKE_DIR}/PlasmaPlatform.cmake)

set(PLASMA_LIGHTNING_CUSTOM_PLUGIN_SHARED_DIR ${PLASMA_CORE_DIR}/Data/LightningCustomPluginShared/) 
set(PLASMA_ALL_TO_ONE_TOOL ${PLASMA_CORE_DIR}/Tools/${PLASMA_PLATFORM}/LightningToOneCpp) 
set(PLASMA_ARCHITECTURE "x64")

add_definitions(-DPlasmaConfigName="${PLASMA_CONFIG}")
add_definitions(-DPlasmaArchitectureName=${PLASMA_ARCHITECTURE})


include(${PLASMA_CMAKE_DIR}/PlasmaFunctions.cmake)
 

include(${PLASMA_TOOLCHAIN_DIR}/${CMAKE_CXX_COMPILER_ID}.cmake)
include(${PLASMA_TARGETOS_DIR}/${CMAKE_SYSTEM_NAME}.cmake)

include(${PLASMA_CMAKE_DIR}/CMakeFlags.cmake)

add_subdirectory(Source/Platform/${PLASMA_PLATFORM})

add_subdirectory(External)
add_subdirectory(Source)

# Enable colored output for Ninja if supported
if (CMAKE_GENERATOR STREQUAL "Ninja")
	include(CheckCXXCompilerFlag)
	
	check_cxx_compiler_flag("-fdiagnostics-color=always" F_DIAGNOSTIC_COLOR_ALWAYS)
	if (F_DIAGNOSTIC_COLOR_ALWAYS)
		add_compile_options("-fdiagnostics-color=always")
	endif()
endif()


message(STATUS "[PLASMA BUILD] GIT Revision ID: ${PLASMA_REVISION_ID}")
message(STATUS "[PLASMA BUILD] GIT Short ChangeSet: ${PLASMA_SHORT_CHANGE_SET}")
message(STATUS "[PLASMA BUILD] GIT ChangeSet: ${PLASMA_CHANGE_SET}")
message(STATUS "[PLASMA BUILD] GIT ChangeSet Date: ${PLASMA_CHANGE_SET_DATE}")
message(STATUS "[PLASMA BUILD] Plasma Engine version ${PLASMA_VERSION}")
message(STATUS "[PLASMA BUILD] Build platform: ${PLASMA_PLATFORM_NAME}")
