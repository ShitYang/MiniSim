# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
project(wsf)


IF(NOT TOOLS_LIBS)
   MESSAGE(FATAL_ERROR "TOOLS_LIBS must be defined.  Don't build this CMakeLists directly.")
ENDIF()

# WSF-based projects use this definition to locate files and libraries,
# either wsf/source must be added as a subdirectory first,
# or WSF_LIBS must be defined by the application
if(NOT WSF_LIBS)
   SET(WSF_LIBS wsf ${TOOLS_LIBS} wsf_util PARENT_SCOPE)
endif()

# Set the wsf source directory for other projects to use
# as a method to add/include other files
set(wsf_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source PARENT_SCOPE)

# Configuration for WsfEventPipe.
add_subdirectory(../wsf_util/ut_pack/pack_to_cpp pack_to_cpp)

add_subdirectory(source)


# Add some configuration files to the project for ease of editing
set(CONFIGURATION_FILES CMakeLists.txt optional_projects.cmake)
source_group(configuration FILES ${CONFIGURATION_FILES})
