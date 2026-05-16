# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

include(GenerateExportHeader)

project(dis)

FILE(GLOB SRC *.cpp *.c)
FILE(GLOB HDR *.hpp *.h)

IF(CMAKE_COMPILER_IS_GNUCXX)
   # This argument causes the linker to exit if the library has unresolved
   # symbols.  Do not disable this option.
   SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
ENDIF()

# Calls a swdev_project.cmake macro to set large source file options for MSVC.
# These options tell the compiler to use 32-bit addressing and enable inline
# expansion, which cuts down on the symbol count about 30%
set(LARGE_SOURCES ${LARGE_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/DisAppearanceEnums.cpp)
large_source_files(${LARGE_SOURCES})

ADD_LIBRARY(${PROJECT_NAME} ${SRC} ${HDR})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "tools")
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
    
    $<INSTALL_INTERFACE:sdk/include/tools/dis>
)
generate_export_header(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PUBLIC genio util util_script)
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
      DESTINATION sdk/include/tools/dis
      FILES_MATCHING PATTERN "*.h"
                     PATTERN "*.hpp"
   )

endif()