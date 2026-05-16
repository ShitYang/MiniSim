# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
# ****************************************************************************
# Updated by Infoscitex, a DCS Company.
# ****************************************************************************
include(GenerateExportHeader)

SET(UT_SCRIPT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

if(MSVC)
   add_compile_options(-wd4251 -wd4286 -wd4275)
endif()

file(GLOB SRCS *.cpp *.hpp)

add_library(${PROJECT_NAME} SHARED ${SRCS})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "tools")

if(BUILD_SHARED_LIBS)
  add_definitions(-DUT_SCRIPT_BUILD_DLL)
endif(BUILD_SHARED_LIBS)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/source>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
    
    # 安装后，所有头文件都在 <prefix>/include 下
    # 所以只需要 include 和 include/source
    $<INSTALL_INTERFACE:sdk/include/tools/util_script>
)

target_link_libraries(${PROJECT_NAME} PRIVATE util)
generate_export_header(${PROJECT_NAME})
if(MSVC)
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
      DESTINATION sdk/include/tools/util_script
      FILES_MATCHING PATTERN "*.h"
                     PATTERN "*.hpp"
   )

endif()
