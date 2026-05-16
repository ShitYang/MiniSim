cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif()

project(MiniSim)

# Setup versioning information
set(AFSIM_VERSION_MAJOR   "2")
set(AFSIM_VERSION_MINOR   "9")
set(AFSIM_VERSION_PATCH   "0")
set(AFSIM_VERSION_BUILD   "0")
set(AFSIM_VERSION         "${AFSIM_VERSION_MAJOR}.${AFSIM_VERSION_MINOR}.${AFSIM_VERSION_PATCH}")
set(AFSIM_RELEASE_DATE    "02-25-2022")
set(AFSIM_RELEASE_VERSION "${AFSIM_VERSION} ${AFSIM_RELEASE_DATE}")

if(MSVC)
    add_compile_options(/Zi)
endif()

add_definitions(-DNOMINMAX)

option(BUILD_SDK "Build and install the MiniSim SDK" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/bin CACHE PATH "Installation path" FORCE)

MACRO(large_source_files)
   # Setting compile flags per file disables unity builds for those files.
   # Fortunately when CMAKE_UNITY_BUILD is enabled, afsim/CMakeLists.txt already
   # adds "/bigobj" to everything, so we can skip adding it again.
   IF(MSVC AND NOT CMAKE_UNITY_BUILD)
      FOREACH(SRC ${ARGV})
         set_source_files_properties(${SRC} PROPERTIES COMPILE_FLAGS "/bigobj")
      ENDFOREACH()
   ENDIF()
ENDMACRO()

if(CMAKE_UNITY_BUILD)
  # By default ensure code *compiles* with all files combined. This ensures CI
  # jobs or developers can tailor the value as needed for build performance.
  if(NOT CMAKE_UNITY_BUILD_BATCH_SIZE)
    set(CMAKE_UNITY_BUILD_BATCH_SIZE 0)
  endif()
  if(MSVC)
    # Allow > 65279 symbols in a single .obj file
    add_compile_options(/bigobj)
  endif()
endif()

# If we are doing a shared/plug-in/DLL build set those options
# otherwise set options for a static build

set(BUILD_SHARED_LIBS TRUE)
set(INSTALL_LIB_PATH "bin/lib")
if (NOT WIN32)
   ADD_DEFINITIONS("-fPIC")
endif()

if(WIN32)
   if(MSVC)
      add_definitions("-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS")
      add_compile_options(/MP)

      if(PROMOTE_HARDWARE_EXCEPTIONS)
         add_compile_options(/EHa)
      else()
         add_compile_options(/EHsc)
      endif()

      if (MSVC_VERSION GREATER 1900)
         # Acknowledge MSVC 2017 std::tr1 deprecation and silence output warning.
         # Only required for googletest usage, tracked at https://github.com/google/googletest/issues/1111
         add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
      endif()
   endif()
   if(BUILD_SHARED_LIBS)
      add_definitions("-DSWDEV_ALL_USE_DLL")
   endif(BUILD_SHARED_LIBS)
   # On windows, the dll's must reside in the same path as the .exe
   if("${INSTALL_DLL_PATH}" STREQUAL "")
      set(INSTALL_DLL_PATH .)
   endif("${INSTALL_DLL_PATH}" STREQUAL "")
   # Don't use the WinDef.h macros for min and max, conflicts with std::min/max
   add_definitions(-DNOMINMAX)
else(WIN32)
   # On linux, we can use rpath to locate so's in a subdirectory
   if("${INSTALL_DLL_PATH}" STREQUAL "")
      set(INSTALL_DLL_PATH ./lib)
   endif("${INSTALL_DLL_PATH}" STREQUAL "")
   # disable strict aliasing optimization (for UtFunction)
   add_definitions("-fno-strict-aliasing")
   
   if(PROMOTE_HARDWARE_EXCEPTIONS)
      add_compile_options(-fnon-call-exceptions)
   endif()
   
endif(WIN32)

if (PROMOTE_HARDWARE_EXCEPTIONS)
   add_definitions(-DPROMOTE_HARDWARE_EXCEPTIONS)
endif()

if(MSVC)
   # Set Stack size from default 1MB in Windows. If this value
   # is too large it can limit the number of threads we can spawn.
   # Default value used by Windows is 1MB which is not good enough either,
   # so set a value of 2MB for now until more issues arise.
   set(CMAKE_CXX_STACK_SIZE "2000000")

   # enable msvc conformance flag (for cross compilation compatibility with gcc) to
   # disable being able to pass an rvalue to a non-const reference parameter.
   add_compile_options(/Zc:referenceBinding)
else()
   # Offer usage of ccache if the program is available.
   find_program(CCACHE_EXECUTABLE ccache)
   if(CCACHE_EXECUTABLE)
      option(AFSIM_USE_CCACHE "Use ccache to accelerate rebuilds" OFF)
      if(AFSIM_USE_CCACHE)
         set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
         set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
      endif()
   endif()
endif()

# Use -O3 instead of -O2 for release
if(NOT WIN32)
   # Since 3rd_party sources are built with GCC 4.8, the default ABI is not the CXX11 ABI
   # that is default in GCC >= 5. Newer compilers will fail to build without this line
   # unless we rebuild 3rd_party with newer versions of GCC
   add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)
   # Add linker flags to export function names
   set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-rdynamic -export-dynamic")
   set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} "-rdynamic -export-dynamic")
   set(CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS} "-rdynamic -export-dynamic")
endif()


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules")
set(WSF_ROOT ${PROJECT_SOURCE_DIR}/core)
set(WSF_EXTENSION_HEADER_DIR ${PROJECT_BINARY_DIR}/include)
set(TOOLS_DIRECTORY ${PROJECT_SOURCE_DIR}/tools)

set(Qt5_SIM_DIR "${CMAKE_SOURCE_DIR}/3rd_party/qt-5.12.5-x64-vs2017")
set(Qt5Core_DIR "${Qt5_SIM_DIR}/lib/cmake/Qt5Core")
set(Qt5WinExtras_DIR "${Qt5_SIM_DIR}/lib/cmake/Qt5WinExtras")
set(Qt5Widgets_DIR "${Qt5_SIM_DIR}/lib/cmake/Qt5Widgets")
set(Qt5OpenGL_DIR "${Qt5_SIM_DIR}/lib/cmake/Qt5OpenGL")
set(Qt5Network_DIR "${Qt5_SIM_DIR}/lib/cmake/Qt5Network")
set(Qt5Gui_DIR "${Qt5_SIM_DIR}/lib/cmake/Qt5Gui")

add_subdirectory(tools/util util)
add_subdirectory(tools/util_script util_script)
add_subdirectory(tools/geodata geodata)
add_subdirectory(tools/packetio/source packetio)
add_subdirectory(tools/dis/source dis)
add_subdirectory(tools/tracking_filters/source tracking_filters)
add_subdirectory(tools/genio/source genio)
add_subdirectory(tools/scene_gen scene_gen)
add_subdirectory(tools/profiling profiling)
add_subdirectory(tools/artificer artificer)
set(TOOLS_LIBS util util_script geodata packetio dis tracking_filters genio profiling)
# set(TOOLS_LIBS util util_script geodata packetio dis genio profiling)
add_subdirectory(core/wsf_util/source wsf_util)
add_subdirectory(core/wsf wsf)
add_subdirectory(core/wsf_mil wsf_mil)

add_subdirectory(warlock)


if (BUILD_SDK)
   install(EXPORT MiniSimSDK
      FILE MiniSimSDK.cmake
      NAMESPACE MiniSim::
      DESTINATION sdk/cmake/MiniSim
   )
endif()
