# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# Copyright (C) 2022 Stellar Science; U.S. Government has Unlimited Rights.
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
include(GenerateExportHeader)

set(library_name artificer)

file(GLOB SRCS *.cpp *.hpp)
add_library(${library_name} SHARED ${SRCS})

generate_export_header(${library_name})
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    
    $<INSTALL_INTERFACE:sdk/include/tools/artificer>
)

set_property(TARGET ${library_name} PROPERTY FOLDER "tools/artificer")
target_link_libraries(${library_name} PUBLIC util)

if (BUILD_SDK)

    install(TARGETS ${PROJECT_NAME}
        EXPORT MiniSimSDK
        LIBRARY DESTINATION sdk/lib
        ARCHIVE DESTINATION sdk/lib
        RUNTIME DESTINATION sdk/bin
    )

    install(DIRECTORY ${PROJECT_SOURCE_DIR}/
        DESTINATION sdk/include/tools/artificer
        FILES_MATCHING PATTERN "*.h"
                    PATTERN "*.hpp"
    )

endif()