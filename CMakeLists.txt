cmake_minimum_required(VERSION 3.12.1 FATAL_ERROR)

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

execute_process(COMMAND git rev-list --count HEAD
                OUTPUT_VARIABLE GIT_REV
                ERROR_QUIET)


execute_process(
    COMMAND bash -c "git diff --quiet --exit-code || echo +"
    OUTPUT_VARIABLE GIT_DIFF)
execute_process(
    COMMAND git describe --exact-match --tags
    OUTPUT_VARIABLE GIT_TAG ERROR_QUIET)
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    OUTPUT_VARIABLE GIT_BRANCH)
execute_process(
    COMMAND git log -1 --pretty=%h --abbrev=12
    OUTPUT_VARIABLE GIT_SHORT_CHANGELIST) 
execute_process(
    COMMAND git log -1 --pretty=%H
    OUTPUT_VARIABLE GIT_CHANGELIST)
execute_process(
    COMMAND git log -1 --pretty=%cd --date=format:%Y-%m-%d
    OUTPUT_VARIABLE GIT_CHANGESET_DATE)

string(STRIP "${GIT_REV}" GIT_REV)
string(STRIP "${GIT_DIFF}" GIT_DIFF)
string(STRIP "${GIT_TAG}" GIT_TAG)
string(STRIP "${GIT_BRANCH}" GIT_BRANCH)

add_definitions(-DPlasmaMsSinceEpoch=${PLASMA_MS_SINCE_EPOCH})
add_definitions(-DPlasmaBranchName="${GIT_BRANCH}")
add_definitions(-DPlasmaRevisionId=${GIT_REV})
add_definitions(-DPlasmaShortChangeSet=${GIT_SHORT_CHANGELIST})
add_definitions(-DPlasmaChangeSet=${GIT_CHANGELIST})
add_definitions(-DPlasmaChangeSetDate=${GIT_CHANGESET_DATE})
add_definitions(-DPlasmaChangeSetDate=${GIT_CHANGESET_DATE})
add_definitions(-DPlasmaMajorVersion=${PLASMA_MAJOR_VERSION})
add_definitions(-DPlasmaMinorVersion=${PLASMA_MINOR_VERSION})
add_definitions(-DPlasmaPatchVersion=${PLASMA_PATCH_VERSION})
add_definitions(-DPlasmaConfigName="${PLASMA_CONFIG}")
add_definitions(-DPlasmaArchitectureName="x64")
add_definitions(-DPlasmaHostOs${CMAKE_SYSTEM_NAME}=1)
add_definitions(-DPlasmaHostOsName="${CMAKE_SYSTEM_NAME}")
add_definitions(-DPlasmaTargetOs${CMAKE_SYSTEM_NAME}=1)
add_definitions(-DPlasmaTargetOsName="${CMAKE_SYSTEM_NAME}")
add_definitions(-DPlasmaPlatform${CMAKE_SYSTEM_NAME}=1)
add_definitions(-DPlasmaPlatformName="${CMAKE_SYSTEM_NAME}")
add_definitions(-DPlasmaToolchain${CMAKE_CXX_COMPILER_ID}=1)
add_definitions(-DPlasmaToolchainName="${CMAKE_CXX_COMPILER_ID}")
add_definitions(-DPlasmaRendererName="${PLASMA_RENDERER}")

set(PLASMA_CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(PLASMA_CMAKE_DIR ${PLASMA_CORE_DIR}/CMakeFiles/)
set(PLASMA_TOOLCHAIN_DIR ${PLASMA_CMAKE_DIR}/Toolchain/)
set(PLASMA_TARGETOS_DIR ${PLASMA_CMAKE_DIR}/TargetOs/)
set(PLASMA_LIBRARIES_DIR ${PLASMA_CORE_DIR}/Source/)
set(PLASMA_PLATFORM_DATA_DIR ${PLASMA_CORE_DIR}/PlatformData/${PLASMA_TOOLCHAIN})

function(plasma_setup_library target directory internal)
  target_include_directories(${target} PUBLIC ${directory} ${PLASMA_LIBRARIES_DIR})
  if (${internal})
    #set_property(TARGET ${target} PROPERTY C_INCLUDE_WHAT_YOU_USE ${PLASMA_IWYU_PATH})
    #set_property(TARGET ${target} PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${PLASMA_IWYU_PATH})
  endif()
  plasma_toolchain_setup_library(${target})
endfunction()

function(plasma_get_all_dependencies outputList TARGET)
    list(APPEND visitedTargets ${TARGET})
    get_target_property(dependencies ${TARGET} INTERFACE_LINK_LIBRARIES)
    get_target_property(targetType ${TARGET} TYPE)
    if (NOT targetType STREQUAL "INTERFACE_LIBRARY")
      get_target_property(manualDependencies ${TARGET} MANUALLY_ADDED_DEPENDENCIES)
      list(APPEND dependencies ${manualDependencies})
    endif()
    set(allLinkedLibs "")
    foreach(dependency ${dependencies})
        if (TARGET ${dependency})
            list(FIND visitedTargets ${dependency} VISITED)
            if (${VISITED} EQUAL -1)
                plasma_get_all_dependencies(childLinkedLibs ${dependency})
                list(APPEND allLinkedLibs ${dependency} ${childLinkedLibs})
            endif()
        endif()
    endforeach()
    set(visitedTargets ${visitedTargets} PARENT_SCOPE)
    set(${outputList} ${allLinkedLibs} PARENT_SCOPE)
endfunction()

function(plasma_copy_from_linked_libraries target)
  plasma_get_all_dependencies(dependencies ${target})
  foreach(dependency ${dependencies})
    set(directory "${${dependency}CopyDirectories}")
    if(directory)
      add_custom_command(TARGET ${target} POST_BUILD COMMAND
        ${CMAKE_COMMAND} -E copy_directory ${directory} $<TARGET_FILE_DIR:${target}>)
    endif()
  endforeach()
endfunction()

function(plasma_target_includes target)
  set(multiValueArgs PUBLIC)
  cmake_parse_arguments(PLASMA "" "" "${multiValueArgs}" "${ARGN}")

  foreach(fromTarget ${PLASMA_PUBLIC})
  target_include_directories(${target} PUBLIC $<TARGET_PROPERTY:${fromTarget},INTERFACE_INCLUDE_DIRECTORIES>)
  target_compile_definitions(${target} PUBLIC $<TARGET_PROPERTY:${fromTarget},INTERFACE_COMPILE_DEFINITIONS>)
  endforeach()
endfunction()
 
#Emscripten
#MSVC
include(${PLASMA_TOOLCHAIN_DIR}/${CMAKE_CXX_COMPILER_ID}.cmake)
include(${PLASMA_TARGETOS_DIR}/${CMAKE_SYSTEM_NAME}.cmake)

set(CMAKE_C_FLAGS                             "${CMAKE_C_FLAGS}                             ${PLASMA_C_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS                           "${CMAKE_CXX_FLAGS}                           ${PLASMA_C_CXX_FLAGS}")

set(CMAKE_C_FLAGS_DEBUG                       "${CMAKE_C_FLAGS_DEBUG}                       ${PLASMA_C_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_DEBUG                     "${CMAKE_CXX_FLAGS_DEBUG}                     ${PLASMA_C_CXX_FLAGS_DEBUG}")

set(CMAKE_C_FLAGS_RELWITHDEBINFO              "${CMAKE_C_FLAGS_RELWITHDEBINFO}              ${PLASMA_C_CXX_FLAGS_RELWITHDEBINFO}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO            "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}            ${PLASMA_C_CXX_FLAGS_RELWITHDEBINFO}")

set(CMAKE_C_FLAGS_RELEASE                     "${CMAKE_C_FLAGS_RELEASE}                     ${PLASMA_C_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_RELEASE                   "${CMAKE_CXX_FLAGS_RELEASE}                   ${PLASMA_C_CXX_FLAGS_RELEASE}")

set(CMAKE_C_FLAGS_MINSIZEREL                  "${CMAKE_C_FLAGS_MINSIZEREL}                  ${PLASMA_C_CXX_FLAGS_MINSIZEREL}")
set(CMAKE_CXX_FLAGS_MINSIZEREL                "${CMAKE_CXX_FLAGS_MINSIZEREL}                ${PLASMA_C_CXX_FLAGS_MINSIZEREL}")

set(CMAKE_STATIC_LINKER_FLAGS                 "${CMAKE_STATIC_LINKER_FLAGS}                 ${PLASMA_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS                 "${CMAKE_SHARED_LINKER_FLAGS}                 ${PLASMA_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS                 "${CMAKE_MODULE_LINKER_FLAGS}                 ${PLASMA_LINKER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS                    "${CMAKE_EXE_LINKER_FLAGS}                    ${PLASMA_LINKER_FLAGS}")

set(CMAKE_STATIC_LINKER_FLAGS_DEBUG           "${CMAKE_STATIC_LINKER_FLAGS_DEBUG}           ${PLASMA_LINKER_FLAGS_DEBUG}")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG           "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}           ${PLASMA_LINKER_FLAGS_DEBUG}")
set(CMAKE_MODULE_LINKER_FLAGS_DEBUG           "${CMAKE_MODULE_LINKER_FLAGS_DEBUG}           ${PLASMA_LINKER_FLAGS_DEBUG}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG              "${CMAKE_EXE_LINKER_FLAGS_DEBUG}              ${PLASMA_LINKER_FLAGS_DEBUG}")

set(CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO  "${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO}  ${PLASMA_LINKER_FLAGS_RELWITHDEBINFO}")
set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO  "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO}  ${PLASMA_LINKER_FLAGS_RELWITHDEBINFO}")
set(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO  "${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO}  ${PLASMA_LINKER_FLAGS_RELWITHDEBINFO}")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO     "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}     ${PLASMA_LINKER_FLAGS_RELWITHDEBINFO}")

set(CMAKE_STATIC_LINKER_FLAGS_RELEASE         "${CMAKE_STATIC_LINKER_FLAGS_RELEASE}         ${PLASMA_LINKER_FLAGS_RELEASE}")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE         "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}         ${PLASMA_LINKER_FLAGS_RELEASE}")
set(CMAKE_MODULE_LINKER_FLAGS_RELEASE         "${CMAKE_MODULE_LINKER_FLAGS_RELEASE}         ${PLASMA_LINKER_FLAGS_RELEASE}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE            "${CMAKE_EXE_LINKER_FLAGS_RELEASE}            ${PLASMA_LINKER_FLAGS_RELEASE}")

set(CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL      "${CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL}      ${PLASMA_LINKER_FLAGS_MINSIZEREL}")
set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL      "${CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL}      ${PLASMA_LINKER_FLAGS_MINSIZEREL}")
set(CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL      "${CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL}      ${PLASMA_LINKER_FLAGS_MINSIZEREL}")
set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL         "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL}         ${PLASMA_LINKER_FLAGS_MINSIZEREL}")

#Windows
#Emscripten
#SDLSTDEmpty
#Stub
add_subdirectory(Source/Platform/${CMAKE_SYSTEM_NAME})

add_subdirectory(External)
add_subdirectory(Source)
