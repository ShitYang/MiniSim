include(GenerateExportHeader)
set(library_name afperf)

add_library(${library_name} SHARED
   afperf.cpp
   "${CMAKE_CURRENT_LIST_DIR}/../impl/ProfilingCsvUtil.cpp"
   "${CMAKE_CURRENT_LIST_DIR}/../impl/ProfilingTimeUtil.cpp"
   # impl headers included in order for headers to be present in IDE folders.
   "${CMAKE_CURRENT_LIST_DIR}/../impl/ProfilingCsvUtil.hpp"
   "${CMAKE_CURRENT_LIST_DIR}/../impl/ProfilingTimeUtil.hpp"
)
set_property(TARGET ${library_name} PROPERTY FOLDER "tools/profiling")
# generate_export_header(${library_name})

target_include_directories(afperf
   PRIVATE
   $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../source>
   $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../common>
   $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../impl>
   $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/${library_name}>

   $<INSTALL_INTERFACE:sdk/include/tools/profiling>
)

generate_export_header(${library_name})
   
target_link_libraries(${library_name} PUBLIC util)

if (BUILD_SDK)

   install(TARGETS afperf
      EXPORT MiniSimSDK
      LIBRARY DESTINATION sdk/lib
      ARCHIVE DESTINATION sdk/lib
      RUNTIME DESTINATION sdk/bin
   )

endif()