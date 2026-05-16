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

project(profiling)


add_subdirectory(afperf)
add_subdirectory(source)

if (BUILD_SDK)

    install(DIRECTORY ${PROJECT_SOURCE_DIR}/
        DESTINATION sdk/include/tools/profiling
        FILES_MATCHING PATTERN "*.h"
                    PATTERN "*.hpp"
    )

endif()

