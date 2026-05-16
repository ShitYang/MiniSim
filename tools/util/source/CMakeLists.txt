# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
# ****************************************************************************
# Updated by DCS Corporation.
# ****************************************************************************
project(util)
include(GenerateExportHeader)

file(GLOB SRCS *.cpp *.hpp)

file(GLOB CT_SRCS ../codetimers/*.cpp ../codetimers/*.hpp)
set(SRCS ${SRCS} ${CT_SRCS})

# Ut code not currently used.  We may be able to move these to util_legacy or some other project
file(GLOB PURGE_SRCS
      # Replaced by WsfAttributeContainer
      UtAttribute*
      # Unit classes, should be using classes in UtUnitTypes.hpp instead ( they match UtInput )
      UtLength.* UtLLAPos.* UtSpeed.* UtAccel.* UtAngleR.* UtAngleRate.* UtDataSize.*

      UcavUtils.*
      # Compatibility headers, no longer necessary
      default_defs.hpp)
#list(REMOVE_ITEM SRCS ${PURGE_SRCS})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/UtBuildInfoMacros.hpp.in ${PROJECT_BINARY_DIR}/source/UtBuildInfoMacros.hpp)
set(SRCS ${SRCS} ${PROJECT_BINARY_DIR}/source/UtBuildInfoMacros.hpp)

IF(CMAKE_COMPILER_IS_GNUCXX)
   # This argument causes the linker to exit if the library has unresolved
   # symbols.  Do not disable this option.
   SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
ENDIF()


ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRCS})
generate_export_header(${PROJECT_NAME} BASE_NAME ut)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "tools")

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/source>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../codetimers>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
    
    $<INSTALL_INTERFACE:sdk/include/tools/util>
)


if (MSVC)
   target_compile_options(${PROJECT_NAME} PUBLIC -wd4251 -wd4286 -wd4275)
   add_compile_options(/wd4996)
endif()

if (BUILD_SDK)

   install(TARGETS ${PROJECT_NAME}
      EXPORT MiniSimSDK
      LIBRARY DESTINATION sdk/lib
      ARCHIVE DESTINATION sdk/lib
      RUNTIME DESTINATION sdk/bin
   )

   install(DIRECTORY ${PROJECT_SOURCE_DIR}/
      DESTINATION sdk/include/tools/util
      FILES_MATCHING PATTERN "*.h"
                     PATTERN "*.hpp"
   )

endif()