cmake_minimum_required(VERSION 3.9)

# Fail immediately if not using an out-of-source build
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR
    "In-source builds are not supported.  Please create a build directory "
    "separate from the source directory")
endif ()

if (POLICY CMP0074)
  # find_package() uses <PackageName>_ROOT variables.
  cmake_policy (SET CMP0074 NEW)
endif ()

if (POLICY CMP0083)
  # To control generation of Position Independent Executable (PIE) or not,
  # some flags are required at link time.
  cmake_policy (SET CMP0083 NEW)
endif ()

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

#------------------------------------------------------------------------------#
# Parse version number from H5Zzfp_version.h
#------------------------------------------------------------------------------#
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/src/H5Zzfp_version.h" H5Z_ZFP_H REGEX "^\#define H5Z_FILTER_ZFP_VERSION_MAJOR")
string(REGEX REPLACE "^.*MAJOR " "" H5Z_ZFP_VERSION_MAJOR "${H5Z_ZFP_H}")
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/src/H5Zzfp_version.h" H5Z_ZFP_H REGEX "^\#define H5Z_FILTER_ZFP_VERSION_MINOR")
string(REGEX REPLACE "^.*MINOR " "" H5Z_ZFP_VERSION_MINOR "${H5Z_ZFP_H}")
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/src/H5Zzfp_version.h" H5Z_ZFP_H REGEX "^\#define H5Z_FILTER_ZFP_VERSION_PATCH")
string(REGEX REPLACE "^.*PATCH " "" H5Z_ZFP_VERSION_PATCH "${H5Z_ZFP_H}")
set(H5Z_ZFP_VERSION "${H5Z_ZFP_VERSION_MAJOR}.${H5Z_ZFP_VERSION_MINOR}.${H5Z_ZFP_VERSION_PATCH}")

project(H5Z_ZFP VERSION ${H5Z_ZFP_VERSION} LANGUAGES C)

#------------------------------------------------------------------------------#
# Compile options.
#------------------------------------------------------------------------------#
option(FORTRAN_INTERFACE "Enable the Fortran interface" ON)
if (FORTRAN_INTERFACE)
  enable_language(Fortran)
endif ()

#------------------------------------------------------------------------------#
# Some boilerplate to setup nice output directories
#------------------------------------------------------------------------------#
set(CMAKE_INSTALL_INCLUDEDIR include)
set(CMAKE_INSTALL_CMAKEDIR lib/cmake/h5z_zfp)

#------------------------------------------------------------------------------#
# Required packages
#------------------------------------------------------------------------------#
# Find HDF5, relies on HDF5_DIR or HDF5_ROOT being set in environment.
set(COMPONENTS C shared)
if (FORTRAN_INTERFACE)
  list(APPEND COMPONENTS Fortran)
endif ()
# By default static will be returned unless only shared is available
find_package(HDF5 REQUIRED COMPONENTS ${COMPONENTS} CONFIG)
if (NOT HDF5_FOUND)
  message (FATAL_ERROR " HDF5 is Required for h5z_zfp")
endif ()
set (HDF5_LIBRARIES ${HDF5_C_SHARED_LIBRARY})
set (HDF5_LIBRARY_PATH ${PACKAGE_PREFIX_DIR}/lib)
set (HDF5_INCLUDE_DIRS ${HDF5_INCLUDE_DIR})
message (STATUS "HDF5 C libs:${HDF5_FOUND} include:${HDF5_INCLUDE_DIRS} and shared:${HDF5_LIBRARIES}")
if (FORTRAN_INTERFACE)
  set (HDF5_FORTRAN_INCLUDE_DIRS ${HDF5_INCLUDE_DIR_FORTRAN})
  set (HDF5_FORTRAN_LIBRARIES ${HDF5_FORTRAN_SHARED_LIBRARY})
  message (STATUS "HDF5 Fortran libs: include:${HDF5_FORTRAN_INCLUDE_DIRS} and shared:${HDF5_FORTRAN_LIBRARIES}")
endif ()

# Find MPI depending on if HDF5 needs MPI.
if (HDF5_IS_PARALLEL AND ("${HDF5_VERSION}" VERSION_EQUAL "1.14.0"))
  find_package(MPI REQUIRED COMPONENTS C)
endif()

# Find ZFP, relies on ZFP_DIR or ZFP_ROOT being set in environment.
find_package(ZFP REQUIRED CONFIG)
if (NOT ZFP_FOUND)
  message (FATAL_ERROR " ZFP is Required for h5z_zfp")
endif ()

#-----------------------------------------------------------------------------
# Dashboard and Testing Settings
#-----------------------------------------------------------------------------
option (BUILD_TESTING "Build h5z-zfp Unit Testing" OFF)
if (BUILD_TESTING)
  set (DART_TESTING_TIMEOUT 1200 CACHE STRING
       "Timeout in seconds for each test (default 1200=20minutes)")
  enable_testing ()
  include (CTest)
endif ()

#------------------------------------------------------------------------------#
# Add source
#------------------------------------------------------------------------------#
add_subdirectory(${CMAKE_SOURCE_DIR}/src)

if (BUILD_TESTING)
  add_subdirectory(${CMAKE_SOURCE_DIR}/test)
endif ()

#------------------------------------------------------------------------------#
# Packaging
#------------------------------------------------------------------------------#
# Install h5z_zfp-config.cmake and h5z_zfp-config-version.cmake
set(H5Z_ZFP_CONFIG_IN  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/h5z_zfp-config.cmake.in)
set(H5Z_ZFP_CONFIG_OUT ${CMAKE_CURRENT_BINARY_DIR}/cmake/h5z_zfp-config.cmake)
configure_file(${H5Z_ZFP_CONFIG_IN} ${H5Z_ZFP_CONFIG_OUT} @ONLY)
set(H5Z_ZFP_CONFIG_VERSION_IN  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/h5z_zfp-config-version.cmake.in)
set(H5Z_ZFP_CONFIG_VERSION_OUT ${CMAKE_CURRENT_BINARY_DIR}/cmake/h5z_zfp-config-version.cmake)
configure_file(${H5Z_ZFP_CONFIG_VERSION_IN} ${H5Z_ZFP_CONFIG_VERSION_OUT} @ONLY)
install(FILES ${H5Z_ZFP_CONFIG_OUT}
        DESTINATION ${CMAKE_INSTALL_CMAKEDIR})
install(FILES ${H5Z_ZFP_CONFIG_VERSION_OUT}
        DESTINATION ${CMAKE_INSTALL_CMAKEDIR})
