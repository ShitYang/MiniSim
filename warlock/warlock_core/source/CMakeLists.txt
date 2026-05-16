# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
# *****************************************************************************
# CMAKE file: warlock_core
# *****************************************************************************
include(GenerateExportHeader)
project(warlock_core)

if (MSVC)
   add_compile_options(-wd4251 -wd4286 -wd4275)
endif()

# *****************************************************************************
# Get the source files
file(GLOB SRCS *.hpp *.cpp)
file(GLOB QRC ${CMAKE_CURRENT_SOURCE_DIR}/../ui/resources/*.qrc)
file(GLOB UIS ${CMAKE_CURRENT_SOURCE_DIR}/../ui/*.ui)

include(qt_project)
configure_qt_project(UI UI_HEADERS ${UIS})

# Add in event pipe
include(${WSF_ROOT}/wsf/event_pipe.cmake)
set(EVENT_PIPE_INCLUDES)
set(EVENT_PIPE_SOURCES)
add_event_pipe_schema(${CMAKE_CURRENT_SOURCE_DIR}/../wk_events.utpack
                      "${wsf_SOURCE_DIR}/../event_pipe/wsf.utpack"
                      ""
                      "WsfEventPipeClasses.hpp"
                      "WsfEventPipe"
                      "WkEventPipe"
                      EVENT_PIPE_INCLUDES
                      EVENT_PIPE_SOURCES)
set(SRCS ${SRCS} ${EVENT_PIPE_INCLUDES})

# *****************************************************************************
# The library we are trying to build
add_library(${PROJECT_NAME} ${SRCS} ${UI_HEADERS} ${QRC})

generate_export_header(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
    
    $<INSTALL_INTERFACE:sdk/include/warlock/warlock_core>
)


# *****************************************************************************
# Libraries
# wsf_mil is required for Warlock's script interface to process scripts with
# param types of WsfWeapon
target_link_libraries(${PROJECT_NAME} PUBLIC wsf wsf_mil util Qt5::Core Qt5::Widgets Qt5::Gui wkf)
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "warlock")

if (BUILD_SDK)

   install(TARGETS ${PROJECT_NAME}
      EXPORT MiniSimSDK
      LIBRARY DESTINATION sdk/lib
      ARCHIVE DESTINATION sdk/lib
      RUNTIME DESTINATION sdk/bin
   )

   install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
      DESTINATION sdk/include/warlock/warlock_core/source
      FILES_MATCHING PATTERN "*.h"
                     PATTERN "*.hpp"
   )

endif()

