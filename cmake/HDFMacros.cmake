#-------------------------------------------------------------------------------
macro (DEFAULT_FOLDERS)
  #-----------------------------------------------------------------------------
  # Setup output Directories
  #-----------------------------------------------------------------------------
  if (NOT ${H5ZZFP_PACKAGE_NAME}_EXTERNALLY_CONFIGURED)
    set (CMAKE_RUNTIME_OUTPUT_DIRECTORY
        ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all Executables."
    )
    set (CMAKE_LIBRARY_OUTPUT_DIRECTORY
        ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all Libraries"
    )
    set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY
        ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all static libraries."
    )
    set (CMAKE_Fortran_MODULE_DIRECTORY
        ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all fortran modules."
    )
    get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if(_isMultiConfig)
      set (CMAKE_TEST_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
      set (CMAKE_PDB_OUTPUT_DIRECTORY
          ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all pdb files."
      )
    else ()
      set (CMAKE_TEST_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    endif ()
  else ()
    # if we are externally configured, but the project uses old cmake scripts
    # this may not be set
    if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
      set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})
    endif ()
  endif ()
endmacro ()

#-------------------------------------------------------------------------------
macro (HDF5_SUPPORT)
  if (NOT HDF5_HDF5_HEADER)
    set (FIND_HDF_COMPONENTS C shared)
    message(STATUS "HDF5 FORTRAN_INTERFACE ${FORTRAN_INTERFACE}" )
    if (FORTRAN_INTERFACE)
      set (FIND_HDF_COMPONENTS ${FIND_HDF_COMPONENTS} Fortran)
    endif ()
    message (STATUS "HDF5 find comps: ${FIND_HDF_COMPONENTS}")
    set (SEARCH_PACKAGE_NAME "HDF5")

    find_package (HDF5 NAMES ${SEARCH_PACKAGE_NAME} COMPONENTS ${FIND_HDF_COMPONENTS})
    message (STATUS "HDF5 C libs:${HDF5_FOUND} static:${HDF5_static_C_FOUND} and shared:${HDF5_shared_C_FOUND}")
    message (STATUS "HDF5 Fortran libs: static:${HDF5_static_Fortran_FOUND} and shared:${HDF5_shared_Fortran_FOUND}")
    if (HDF5_FOUND)
      if (HDF5_shared_C_FOUND)
        if (NOT TARGET ${HDF5_NAMESPACE}h5dump-shared)
          add_executable (${HDF5_NAMESPACE}h5dump-shared IMPORTED)
        endif ()
        set (HDF5_DUMP_EXECUTABLE $<TARGET_FILE:${HDF5_NAMESPACE}h5dump-shared>)

        if (NOT TARGET ${HDF5_NAMESPACE}h5diff-shared)
          add_executable (${HDF5_NAMESPACE}h5diff-shared IMPORTED)
        endif ()
        set (HDF5_DIFF_EXECUTABLE $<TARGET_FILE:${HDF5_NAMESPACE}h5diff-shared>)

        if (NOT TARGET ${HDF5_NAMESPACE}h5repack-shared)
          add_executable (${HDF5_NAMESPACE}h5repack-shared IMPORTED)
        endif ()
        set (HDF5_REPACK_EXECUTABLE $<TARGET_FILE:${HDF5_NAMESPACE}h5repack-shared>)
      endif()

      if (NOT HDF5_shared_C_FOUND)
        #find library from non-dual-binary package
        set (FIND_HDF_COMPONENTS C)
        if (FORTRAN_INTERFACE)
          set (FIND_HDF_COMPONENTS ${FIND_HDF_COMPONENTS} Fortran)
        endif ()
        message (STATUS "HDF5 find comps: ${FIND_HDF_COMPONENTS}")

        find_package (HDF5 NAMES ${SEARCH_PACKAGE_NAME} COMPONENTS ${FIND_HDF_COMPONENTS})
        message (STATUS "HDF5 libs:${HDF5_FOUND} C:${HDF5_C_FOUND} Fortran:${HDF5_Fortran_FOUND}")
        if (HDF5_BUILD_SHARED_LIBS)
          add_definitions (-DH5_BUILT_AS_DYNAMIC_LIB)
        else ()
          add_definitions (-DH5_BUILT_AS_STATIC_LIB)
        endif ()
        if (FORTRAN_INTERFACE AND ${HDF5_BUILD_FORTRAN})
          if (HDF5_shared_Fortran_FOUND)
            set (HDF5_FORTRAN_INCLUDE_DIRS ${HDF5_INCLUDE_DIR_FORTRAN})
            set (HDF5_FORTRAN_LIBRARIES ${HDF5_FORTRAN_SHARED_LIBRARY})
          else ()
            set (FORTRAN_INTERFACE OFF CACHE BOOL "Build FORTRAN support" FORCE)
            message (STATUS "HDF5 Fortran libs not found - disable build of Fortran support")
          endif ()
        endif ()
        if (WIN32)
          set_property (TARGET ${HDF5_NAMESPACE}h5dump PROPERTY IMPORTED_LOCATION "${HDF5_TOOLS_DIR}/h5dumpdll")
          set_property (TARGET ${HDF5_NAMESPACE}h5diff PROPERTY IMPORTED_LOCATION "${HDF5_TOOLS_DIR}/h5diffdll")
          set_property (TARGET ${HDF5_NAMESPACE}h5repack PROPERTY IMPORTED_LOCATION "${HDF5_TOOLS_DIR}/h5repackdll")
        endif ()
        set (HDF5_DUMP_EXECUTABLE $<TARGET_FILE:${HDF5_NAMESPACE}h5dump>)
        set (HDF5_DUMP_EXECUTABLE $<TARGET_FILE:${HDF5_NAMESPACE}h5diff>)
        set (HDF5_DUMP_EXECUTABLE $<TARGET_FILE:${HDF5_NAMESPACE}h5repack>)
      else ()
        if (HDF5_shared_C_FOUND)
          set (HDF5_LIBRARIES ${HDF5_C_SHARED_LIBRARY})
          set (HDF5_LIBRARY_PATH ${PACKAGE_PREFIX_DIR}/lib)
          set_property (TARGET ${HDF5_NAMESPACE}h5dump-shared PROPERTY IMPORTED_LOCATION "${HDF5_TOOLS_DIR}/h5dump-shared")
          set_property (TARGET ${HDF5_NAMESPACE}h5diff-shared PROPERTY IMPORTED_LOCATION "${HDF5_TOOLS_DIR}/h5diff-shared")
          set_property (TARGET ${HDF5_NAMESPACE}h5repack-shared PROPERTY IMPORTED_LOCATION "${HDF5_TOOLS_DIR}/h5repack-shared")
        else ()
          set (HDF5_FOUND 0)
        endif ()
        if (FORTRAN_INTERFACE AND ${HDF5_BUILD_FORTRAN})
          if (HDF5_shared_Fortran_FOUND)
            set (HDF5_FORTRAN_INCLUDE_DIRS ${HDF5_INCLUDE_DIR_FORTRAN})
            set (HDF5_FORTRAN_LIBRARIES ${HDF5_FORTRAN_SHARED_LIBRARY})
          else ()
            set (FORTRAN_INTERFACE OFF CACHE BOOL "Build FORTRAN support" FORCE)
            message (STATUS "HDF5 Fortran libs not found - disable build of Fortran support")
          endif ()
        else ()
          set (FORTRAN_INTERFACE OFF CACHE BOOL "Build FORTRAN support" FORCE)
          message (STATUS "HDF5 Fortran libs not found - disable build of Fortran support")
        endif ()
      endif ()
    else ()
     if (FORTRAN_INTERFACE)
        set(FORTRAN_COMP "Fortran")
      endif()
      find_package (HDF5 COMPONENTS ${FORTRAN_COMP}) # Legacy find
      if (FORTRAN_INTERFACE AND NOT HDF5_Fortran_FOUND)
        set (FORTRAN_INTERFACE OFF CACHE BOOL "Build FORTRAN support" FORCE)
        message (STATUS "HDF5 Fortran libs not found - disable build of Fortran support")
      endif ()
      #Legacy find_package does not set HDF5_TOOLS_DIR, so we set it here
      get_filename_component(HDF5_BIN_DIR ${HDF5_DIFF_EXECUTABLE} DIRECTORY)
      set(HDF5_TOOLS_DIR ${HDF5_BIN_DIR})
      if (HDF5_DUMP_EXECUTABLE AND NOT TARGET hdf5::h5dump)
        add_executable (${HDF5_NAMESPACE}h5dump IMPORTED)
        set_property (TARGET ${HDF5_NAMESPACE}h5dump PROPERTY IMPORTED_LOCATION "${HDF5_TOOLS_DIR}/h5dump")
        set (HDF5_DUMP_EXECUTABLE $<TARGET_FILE:${HDF5_NAMESPACE}h5dump>)
      endif ()

      if (HDF5_DIFF_EXECUTABLE AND NOT TARGET hdf5::h5diff)
        add_executable (${HDF5_NAMESPACE}h5diff IMPORTED)
        set_property (TARGET ${HDF5_NAMESPACE}h5diff PROPERTY IMPORTED_LOCATION "${HDF5_TOOLS_DIR}/h5diff")
        set (HDF5_DIFF_EXECUTABLE $<TARGET_FILE:${HDF5_NAMESPACE}h5diff>)
      endif ()

      if (HDF5_REPACK_EXECUTABLE AND NOT TARGET hdf5::h5repack)
        add_executable (${HDF5_NAMESPACE}h5repack IMPORTED)
        set_property (TARGET ${HDF5_NAMESPACE}h5repack PROPERTY IMPORTED_LOCATION "${HDF5_TOOLS_DIR}/h5repack")
        set (HDF5_REPACK_EXECUTABLE $<TARGET_FILE:${HDF5_NAMESPACE}h5repack>)
      endif ()
    endif ()

    set (HDF5_PACKAGE_NAME ${SEARCH_PACKAGE_NAME})

    if (HDF5_FOUND)
      set (HDF5_HAVE_H5PUBCONF_H 1)
      set (HDF5_HAVE_HDF5 1)
      set (HDF5_HDF5_HEADER "h5pubconf.h")
      set (HDF5_INCLUDE_DIRS ${HDF5_INCLUDE_DIR})
      message (STATUS "HDF5-${HDF5_VERSION_STRING} found: INC=${HDF5_INCLUDE_DIR} TOOLS=${HDF5_TOOLS_DIR}")
    else ()
      message (FATAL_ERROR " HDF5 shared is required for H5Z-ZFP")
    endif ()
  else ()
    # This project is being called from within another and HDF5 is already configured
    set (HDF5_HAVE_H5PUBCONF_H 1)
    set (HDF5_HAVE_HDF5 1)
    set (HDF5_LIBRARIES ${HDF5_LINK_LIBS})
    set (HDF5_INCLUDE_DIRS ${HDF5_INCLUDE_DIR})
  endif ()
  set (HDF5_LIBRARY_PATH ${PACKAGE_PREFIX_DIR}/lib)
  if (FORTRAN_INTERFACE)
    message (STATUS "HDF5 Fortran libs: include:${HDF5_FORTRAN_INCLUDE_DIRS} and shared:${HDF5_FORTRAN_LIBRARIES}")
  endif ()
  message (STATUS "HDF5 link libs: ${HDF5_LIBRARIES} Includes: ${HDF5_INCLUDE_DIRS}")
endmacro ()
