add_library(SpirvHeaders)

plasma_setup_library(SpirvHeaders ${CMAKE_CURRENT_LIST_DIR} FALSE)

target_sources(SpirvHeaders
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Repo/include/spirv/unified1/spirv.h
    ${CMAKE_CURRENT_LIST_DIR}/Symbols.cpp
)

target_include_directories(SpirvHeaders
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Repo/include/
)

set_target_properties( SpirvHeaders PROPERTIES FOLDER "External/SpirvHeaders" )
