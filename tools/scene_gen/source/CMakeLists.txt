# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# Copyright (C) 2021 Stellar Science; U.S. Government has Unlimited Rights.
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

include(GenerateExportHeader)

FILE(GLOB SRCS *.cpp *.hpp)

add_executable(scene_generator ${SRCS})
target_include_directories(scene_generator PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
    
    $<INSTALL_INTERFACE:sdk/include/tools/scene_gen>
)
target_link_libraries(scene_generator PUBLIC util)

# Copies over required (invariant) files for the sample project to work in AFSIM.
# The scene generator creates the additional platform files to create a simulation with platforms
# add_custom_target(scene_gen_sample_project
#     COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/../projects/base_project ${PROJECT_BINARY_DIR}/projects/base_project
# )
# add_dependencies(scene_generator scene_gen_sample_project)
# set_target_properties(scene_generator scene_gen_sample_project PROPERTIES FOLDER "tools/${PROJECT_NAME}")

if (MSVC)
   target_compile_options(scene_generator PUBLIC -wd4251 -wd4286 -wd4275)
endif()

if (BUILD_SDK)

   install(TARGETS scene_generator
      EXPORT MiniSimSDK
      LIBRARY DESTINATION sdk/lib
      ARCHIVE DESTINATION sdk/lib
      RUNTIME DESTINATION sdk/bin
   )

   install(DIRECTORY ${PROJECT_SOURCE_DIR}/
      DESTINATION sdk/include/tools/scene_gen
      FILES_MATCHING PATTERN "*.h"
                     PATTERN "*.hpp"
   )

endif()
