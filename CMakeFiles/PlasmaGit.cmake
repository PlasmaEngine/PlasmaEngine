execute_process(COMMAND git rev-list --count HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE PLASMA_REVISION_ID
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if ("${PLASMA_REVISION_ID}" STREQUAL "")
    # We don't have any source control so just hard-code all of this information to empty
    set (PLASMA_REVISION_ID 0)
    set (PLASMA_SHORT_CHANGE_SET 0)
    set (PLASMA_CHANGE_SET 0)
    set (PLASMA_CHANGE_SET_DATE "")
else ()
    execute_process(COMMAND git rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE PLASMA_SHORT_CHANGE_SET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(COMMAND git rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE PLASMA_CHANGE_SET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(COMMAND git log -1 --abbrev=12 --date=format:%Y-%m-%d --pretty=format:%cd
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE PLASMA_CHANGE_SET_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif ()