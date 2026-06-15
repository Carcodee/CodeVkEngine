
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was CodeCudaEngineConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

include(CMakeFindDependencyMacro)

set(_CodeCudaEngine_CUDAToolkit_ROOT "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.8")
file(TO_CMAKE_PATH "${_CodeCudaEngine_CUDAToolkit_ROOT}" _CodeCudaEngine_CUDAToolkit_ROOT)
set(CUDAToolkit_ROOT "${_CodeCudaEngine_CUDAToolkit_ROOT}" CACHE PATH "CUDA toolkit used by CodeCudaEngine" FORCE)
find_dependency(CUDAToolkit REQUIRED)
unset(_CodeCudaEngine_CUDAToolkit_ROOT)

include("${CMAKE_CURRENT_LIST_DIR}/CodeCudaEngineTargets.cmake")
