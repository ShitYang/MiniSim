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
project(wsf_util)


file(GLOB SRCS *.?pp)

add_library(${PROJECT_NAME} ${SRCS})


target_link_libraries(${PROJECT_NAME} util)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    
    $<INSTALL_INTERFACE:sdk/include/core/wsf_util>
)

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
    DESTINATION sdk/include/core/wsf_util
    FILES_MATCHING PATTERN "*.h"
                    PATTERN "*.hpp"
    )

endif()
