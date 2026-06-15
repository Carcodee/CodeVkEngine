#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "CodeCudaEngine::codeCudaLib" for configuration "Debug"
set_property(TARGET CodeCudaEngine::codeCudaLib APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(CodeCudaEngine::codeCudaLib PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CUDA"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/codeCudaLib.lib"
  )

list(APPEND _cmake_import_check_targets CodeCudaEngine::codeCudaLib )
list(APPEND _cmake_import_check_files_for_CodeCudaEngine::codeCudaLib "${_IMPORT_PREFIX}/lib/codeCudaLib.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
