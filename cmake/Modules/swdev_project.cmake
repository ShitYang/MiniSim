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



# Microsoft's compilers need to use the /bigobj flag for source files containing many symbols
# This macro marks a list of sources as large (> 65,279 symbols.)
# Without it, especially Debug builds in older MSVC versions (2015, 2017), report:
#   error: number of sections exceeded object file format limit: compile with /bigobj
# /bigobj increases .obj file sizes by 2-3% so we use it only where needed.
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


macro(try_add_subdirectory DIR TARGET_NAME)
   if(NOT TARGET ${TARGET_NAME})
      add_subdirectory(${DIR} ${TARGET_NAME})
   endif(NOT TARGET ${TARGET_NAME})
endmacro(try_add_subdirectory DIR TARGET_NAME)

macro(link_sockets TARGET)
   if(WIN32)
      target_link_libraries(${TARGET} PUBLIC ws2_32)
   endif(WIN32)
endmacro(link_sockets TARGET)

# Generates a bash script to launch an application (TARGET_NAME) from outside of the install directory
function(configure_bash_launcher TARGET_NAME)
   if (NOT WIN32)
      configure_file(${CMAKE_SOURCE_DIR}/cmake/Modules/BashLauncher.sh.in ${CMAKE_BINARY_DIR}/${TARGET_NAME}.sh @ONLY)
      install(FILES ${CMAKE_BINARY_DIR}/${TARGET_NAME}.sh DESTINATION bin COMPONENT Runtime
         PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
   endif()
endfunction()

macro(swdev_absolute_paths VARNAME)
   set(${VARNAME})
   foreach(ARG ${ARGN})
      if (IS_ABSOLUTE "${ARG}")
         GET_FILENAME_COMPONENT(ARG_ABS_PATH "${ARG}" ABSOLUTE)
         set(${VARNAME} ${${VARNAME}} "${ARG_ABS_PATH}")
      elseif(EXISTS "${ARG}")
         GET_FILENAME_COMPONENT(ARG_ABS_PATH "${ARG}" ABSOLUTE)
         set(${VARNAME} ${${VARNAME}} "${ARG_ABS_PATH}")
      elseif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${ARG}")
         GET_FILENAME_COMPONENT(ARG_ABS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${ARG}" ABSOLUTE)
         set(${VARNAME} ${${VARNAME}} "${ARG_ABS_PATH}")
      endif()
   endforeach()
endmacro()