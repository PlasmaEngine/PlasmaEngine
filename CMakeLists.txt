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

project(Plasma)

set(PLASMA_RENDERER "OpenGL" CACHE STRING "Rendering backend used by the engine")
set_property(CACHE PLASMA_RENDERER PROPERTY STRINGS OpenGL DirectX12 Vulkan) 

add_definitions(-DPlasmaMsSinceEpoch=${PLASMA_MS_SINCE_EPOCH})
add_definitions(-DPlasmaBranchName="${PLASMA_BRANCH}")
add_definitions(-DPlasmaRevisionId=${PLASMA_REVISION})
add_definitions(-DPlasmaShortChangeSet=${PLASMA_SHORT_CHANGESET})
add_definitions(-DPlasmaChangeSet=${PLASMA_CHANGESET})
add_definitions(-DPlasmaChangeSetDate=${PLASMA_CHANGESET_DATE})
add_definitions(-DPlasmaChangeSetDate=${PLASMA_CHANGESET_DATE})
add_definitions(-DPlasmaMajorVersion=${PLASMA_MAJOR_VERSION})
add_definitions(-DPlasmaMinorVersion=${PLASMA_MINOR_VERSION})
add_definitions(-DPlasmaPatchVersion=${PLASMA_PATCH_VERSION})
add_definitions(-DPlasmaConfigName="${PLASMA_CONFIG}")
add_definitions(-DPlasmaArchitectureName="${PLASMA_ARCHITECTURE}")
add_definitions(-DPlasmaHostOs${PLASMA_HOSTOS}=1)
add_definitions(-DPlasmaHostOsName="${PLASMA_HOSTOS}")
add_definitions(-DPlasmaTargetOs${PLASMA_TARGETOS}=1)
add_definitions(-DPlasmaTargetOsName="${PLASMA_TARGETOS}")
add_definitions(-DPlasmaPlatform${PLASMA_PLATFORM}=1)
add_definitions(-DPlasmaPlatformName="${PLASMA_PLATFORM}")
add_definitions(-DPlasmaToolchain${PLASMA_TOOLCHAIN}=1)
add_definitions(-DPlasmaToolchainName="${PLASMA_TOOLCHAIN}")
add_definitions(-DPlasmaRendererName="${PLASMA_RENDERER}")
add_definitions(-DPlasmaRenderer${PLASMA_RENDERER})

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
include(${PLASMA_TOOLCHAIN_DIR}/${PLASMA_TOOLCHAIN}.cmake)
include(${PLASMA_TARGETOS_DIR}/${PLASMA_TARGETOS}.cmake)

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
add_subdirectory(Source/Platform/${PLASMA_PLATFORM})

add_subdirectory(External)
add_subdirectory(Source)
