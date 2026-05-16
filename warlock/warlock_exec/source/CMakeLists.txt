project(warlock)
include(swdev_project)

if (MSVC)
   add_compile_options(-wd4251 -wd4286 -wd4275)
endif()

# *****************************************************************************
# Get the source files
file(GLOB SRCS *.hpp *.cpp)

# Include macros and configuration to support builds of Qt projects
if (WIN32)
   include(qt_project)
   configure_qt_project(MODULES WinExtras)
endif()

# *****************************************************************************
# The executable we are trying to build
try_add_subdirectory(${TOOLS_DIRECTORY}/utilqt/source utilqt)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../wkf wkf)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../warlock_core/source warlock_core)

add_executable(${PROJECT_NAME} WIN32 ${SRCS})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "applications")

# plugins
set(PLUGIN_ROOT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../plugins"
                     "${CMAKE_SOURCE_DIR}/../afsim_*shared"
                     "${WSF_ADD_EXTENSION_PATH}")
set(PLUGIN_INSTALL_DIR "${INSTALL_PLUGIN_PATH}/warlock_plugins")

include(vis_plugins)
use_vis_plugins(WARLOCK)

# *****************************************************************************
# Configure version header and resource files
include(GenerateVersionInfo)
generate_version_header(${PROJECT_NAME})
if(MSVC)
   generate_resource(${PROJECT_NAME} "warlock_icon.ico")
endif()

# for wsf_extensions.hpp
target_include_directories(${PROJECT_NAME} PRIVATE ${WSF_EXTENSION_HEADER_DIR})

# # 仅限 Windows
# if(WIN32)
#     # 获取 Windows SDK 版本（需要提前通过 CMake 或环境变量知道）
#     set(WINDOWS_SDK_VERSION "10.0.22621.0")  # ← 请根据你实际安装的版本修改！

#     set(WINDOWS_SDK_INCLUDE_DIR "C:/Program Files (x86)/Windows Kits/10/Include/${WINDOWS_SDK_VERSION}")
#     target_include_directories(warlock PRIVATE
#         "${WINDOWS_SDK_INCLUDE_DIR}/um"
#         "${WINDOWS_SDK_INCLUDE_DIR}/shared"
#         "${WINDOWS_SDK_INCLUDE_DIR}/winrt"
#         "${WINDOWS_SDK_INCLUDE_DIR}/ucrt"
#     )
# endif()

# *****************************************************************************
# Libraries
target_link_libraries(${PROJECT_NAME} PUBLIC wsf warlock_core wkf)
if (WIN32)
   target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::WinExtras)
endif()
configure_qt_conf(${PROJECT_NAME})
configure_bash_launcher(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN/lib")


