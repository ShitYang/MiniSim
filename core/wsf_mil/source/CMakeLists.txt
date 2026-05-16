# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
include (GenerateExportHeader)


if (MSVC)
   add_compile_options(-wd4251 -wd4286 -wd4275)
endif()

FILE(GLOB SRCS *.?pp)
set(WSF_MIL_COMP_SUB comm ew mover processor observer script sensor weapon)
foreach(SUB ${WSF_MIL_COMP_SUB})
   FILE(GLOB SUB_SRCS  ${SUB}/*.?pp)
   set(SRCS ${SRCS} ${SUB_SRCS})
   source_group(${SUB} FILES ${SUB_SRCS})
   set(WSF_MIL_COMP_INCLUDES "${WSF_MIL_COMP_INCLUDES}" "${CMAKE_CURRENT_SOURCE_DIR}/${SUB}")
endforeach(SUB ${WSF_MIL_COMP_SUB})

#if(WSF_USE_DIS)
   set(WSF_MIL_SUB ${WSF_MIL_SUB} dis)
#endif()
#if(WSF_USE_XIO)
   set(WSF_MIL_SUB ${WSF_MIL_SUB} xio)
#endif()

foreach(SUB ${WSF_MIL_SUB})
   FILE(GLOB SUB_SRCS ${SUB}/*.?pp)
   set(SRCS ${SRCS} ${SUB_SRCS})
   source_group(${SUB} FILES ${SUB_SRCS})
endforeach(SUB ${WSF_MIL_SUB})
FILE(GLOB SUB_SRCS *.?pp)
set(SRCS ${SRCS} ${SUB_SRCS})

# Add in event pipe
include(${WSF_ROOT}/wsf/event_pipe.cmake)
set(EVENT_PIPE_INCLUDES)
set(EVENT_PIPE_SOURCES)
add_event_pipe_schema(${CMAKE_CURRENT_SOURCE_DIR}/../wsf_mil_events.utpack
                      "${wsf_SOURCE_DIR}/../event_pipe/wsf.utpack"
                      ""
                      "WsfEventPipeClasses.hpp"
                      "WsfEventPipe"
                      "WsfMilEventPipe"
                      EVENT_PIPE_INCLUDES
                      EVENT_PIPE_SOURCES)
set(SRCS ${SRCS} ${EVENT_PIPE_INCLUDES})

add_library(${PROJECT_NAME} ${SRCS})

generate_export_header(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/source>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    
    $<INSTALL_INTERFACE:sdk/include/core/wsf_mil>
)
foreach(SUB ${WSF_MIL_COMP_SUB})
   target_include_directories(${PROJECT_NAME} PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${SUB}>
      $<INSTALL_INTERFACE:sdk/include/core/wsf_mil>
   )
endforeach(SUB ${WSF_MIL_COMP_SUB})

target_link_libraries(${PROJECT_NAME} PUBLIC wsf)

# Set the subfolder that the target will appear under in an IDE's solution explorer.
# This isn't actually related to any file system folders.
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "core")


if (BUILD_SDK)

   install(TARGETS ${PROJECT_NAME}
      EXPORT MiniSimSDK
      LIBRARY DESTINATION sdk/lib
      ARCHIVE DESTINATION sdk/lib
      RUNTIME DESTINATION sdk/bin
   )

   install(DIRECTORY ${PROJECT_SOURCE_DIR}/
      DESTINATION sdk/include/core/wsf_mil
      FILES_MATCHING PATTERN "*.h"
                     PATTERN "*.hpp"
   )

endif()