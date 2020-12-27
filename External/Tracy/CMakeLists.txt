option(TRACY_ENABLE "Profiling is enabled" ON)
option(TRACY_ON_DEMAND "Profiling enabled on demand" ON)
option(TRACY_NO_EXIT "Client executable does not exit until all profile data is sent to server" OFF)
option(TRACY_NO_BROADCAST "Disable client discovery by broadcast to local network" OFF)

add_library(tracy Repo/TracyClient.cpp)

macro(set_option OPTION)
    if(${OPTION})
        message(STATUS "${OPTION}: ON")
        target_compile_definitions(tracy PUBLIC ${OPTION})
    else()
        message(STATUS "${OPTION}: OFF")
    endif()
endmacro()

set_option(TRACY_ENABLE)
set_option(TRACY_ON_DEMAND)
set_option(TRACY_NO_EXIT)
set_option(TRACY_NO_BROADCAST)
set_option(TRACY_USE_CONSTEXPR_VARIABLES)

if(NOT TRACY_PORT)
    set(TRACY_PORT 8086)
endif()

message(STATUS "TRACY_PORT: ${TRACY_PORT}")
target_compile_definitions(tracy PUBLIC TRACY_PORT=${TRACY_PORT})

target_include_directories(tracy PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Repo")

  set_target_properties( tracy PROPERTIES FOLDER "External/Tracy" )
