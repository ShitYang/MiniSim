include(GenerateExportHeader)
project(wkf)

if (MSVC)
   add_compile_options(-wd4251 -wd4286 -wd4275)
endif()

# *****************************************************************************
# Get the source files
file(GLOB SRCS *.hpp *.cpp)

# Include macros and configuration to support builds of Qt projects
if (WIN32)
   include(qt_project)
   configure_qt_project(MODULES WinExtras Widgets Gui)
endif()

add_library(${PROJECT_NAME} SHARED ${SRCS})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "applications/wkf")

# *****************************************************************************
# Configure version header and resource files
generate_export_header(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${WSF_EXTENSION_HEADER_DIR}>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    
    $<INSTALL_INTERFACE:sdk/include/warlock/wkf>
)

# *****************************************************************************
# Libraries
target_link_libraries(${PROJECT_NAME} PUBLIC utilqt Qt5::Gui Qt5::Widgets Qt5::Core)
if (WIN32)
   target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::WinExtras)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN/lib")

install(DIRECTORY sprites DESTINATION resources)
install(DIRECTORY tiffs DESTINATION resources)
install(DIRECTORY projects DESTINATION .)

if (BUILD_SDK)

   install(TARGETS ${PROJECT_NAME}
      EXPORT MiniSimSDK
      LIBRARY DESTINATION sdk/lib
      ARCHIVE DESTINATION sdk/lib
      RUNTIME DESTINATION sdk/bin
   )

   install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
      DESTINATION sdk/include/warlock/wkf
      FILES_MATCHING PATTERN "*.h"
                     PATTERN "*.hpp"
   )

endif()
