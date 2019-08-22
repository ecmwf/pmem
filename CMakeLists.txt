cmake_minimum_required( VERSION 3.1 FATAL_ERROR )

project( pmem C CXX )

set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../ecbuild/cmake")

include( ecbuild_system NO_POLICY_SCOPE )

ecbuild_requires_macro_version( 2.9 )

ecbuild_declare_project()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

### dependencies

ecbuild_use_package( PROJECT eckit VERSION  0.21  REQUIRED )

find_package(pmemio REQUIRED)

### export package to other ecbuild packages

set( PMEM_INCLUDE_DIRS       ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/pmem ${CMAKE_CURRENT_BINARY_DIR}/src/tests/pmem ${PMEMIO_INCLUDE_DIRS} )
set( PMEM_LIBRARIES          pmempp ${PMEMIO_OBJ_LIBRARY} )

get_directory_property( PMEM_DEFINITIONS COMPILE_DEFINITIONS )

### include directories

include_directories( ${PMEM_INCLUDE_DIRS} ${PMEMIO_INCLUDE_DIRS} )

add_subdirectory( src )

### finalize project

ecbuild_add_resources( TARGET ${PROJECT_NAME}_top_files SOURCES README.md)

ecbuild_pkgconfig(NAME pmem
                  DESCRIPTION "Use the persistent memory libs"
                  LIBRARIES pmempp )

ecbuild_install_project( NAME ${PROJECT_NAME} )

ecbuild_print_summary()
