# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
project(tracking_filters)

include(GenerateExportHeader)

FILE(GLOB SRCS *.cpp *.hpp)

add_library(${PROJECT_NAME} ${SRCS})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "tools")
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
    
    $<INSTALL_INTERFACE:sdk/include/tools/tracking_filters>
)
target_link_libraries(${PROJECT_NAME} PUBLIC util)
generate_export_header(${PROJECT_NAME})
if (MSVC)
   target_compile_options(${PROJECT_NAME} PUBLIC -wd4251 -wd4286 -wd4275)
endif()

if (BUILD_SDK)

   install(TARGETS ${PROJECT_NAME}
      EXPORT MiniSimSDK
      LIBRARY DESTINATION sdk/lib
      ARCHIVE DESTINATION sdk/lib
      RUNTIME DESTINATION sdk/bin
   )

   install(DIRECTORY ${PROJECT_SOURCE_DIR}/
      DESTINATION sdk/include/tools/tracking_filters
      FILES_MATCHING PATTERN "*.h"
                     PATTERN "*.hpp"
   )

endif()
