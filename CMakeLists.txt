cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)

#-----------------------------------------------------------------------
# CPM configuration
#-----------------------------------------------------------------------
set(CPM_MODULE_NAME "gl_batch_context")
set(CPM_LIB_TARGET_NAME ${CPM_MODULE_NAME})

if ((DEFINED CPM_DIR) AND (DEFINED CPM_UNIQUE_ID) AND (DEFINED CPM_TARGET_NAME))
  set(CPM_LIB_TARGET_NAME ${CPM_TARGET_NAME})
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CPM_DIR})
  include(CPM)
else()
  set (CPM_DIR "${CMAKE_CURRENT_BINARY_DIR}/cpm-packages" CACHE TYPE STRING)
  find_package(Git)
  if(NOT GIT_FOUND)
    message(FATAL_ERROR "CPM requires Git.")
  endif()
  if (NOT EXISTS ${CPM_DIR}/CPM.cmake)
    message(STATUS "Cloning repo (https://github.com/iauns/cpm)")
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" clone https://github.com/iauns/cpm ${CPM_DIR}
      RESULT_VARIABLE error_code
      OUTPUT_QUIET ERROR_QUIET)
    if(error_code)
      message(FATAL_ERROR "CPM failed to get the hash for HEAD")
    endif()
  endif()
  include(${CPM_DIR}/CPM.cmake)
endif()

#-----------------------------------------------------------------------
# CPM Modules
#-----------------------------------------------------------------------

# This call will ensure all include directories and definitions are present
# in the target. These correspond to the modules that we added above.
CPM_InitModule(${CPM_MODULE_NAME})

#-----------------------------------------------------------------------
# Module specific CMake options
#-----------------------------------------------------------------------
# Find OpenGL
find_package(OpenGL REQUIRED)

# Option for using an OS mesa context.
option(USE_OS_MESA "Use an OS mesa context." OFF)
if (USE_OS_MESA)
  add_definitions(-DUSE_OS_MESA)
  set(MESA_LIBRARIES "OSMesa32")
endif()

# Compiler options
if (UNIX)
  if (APPLE)
    # Apple specific sources
    set (SpecificSources
      gl-batch-context/src/CGLContext.cpp
      gl-batch-context/src/CGLContext.h
      gl-batch-context/src/NSContext.mm
      gl-batch-context/src/NSContext.h
      )
    add_definitions(-DGL_CONTEXT_USING_OSX)
  else ()
    # Linux specific sources
    if (USE_OS_MESA)
      set (SpecificSources
        gl-batch-context/src/OSMesaContext.cpp
        gl-batch-context/src/OSMesaContext.h
        )
    else()
      set (SpecificSources
        gl-batch-context/src/GLXContext.cpp
        gl-batch-context/src/GLXContext.h
        )
    endif()
    add_definitions(-DGL_CONTEXT_USING_LINUX)
  endif ()
else()
  # TODO: Windows won't work until we get a glew module that clones
  #       glew from https://github.com/nigels-com/glew and adds the appropriate
  #       sources.
  if (WIN32)
    # Since we are building a static library, we need to define GLEW_STATIC
    # <url:http://glew.sourceforge.net/install.html#^define the GLEW_STATIC>
    add_definitions(-DGLEW_STATIC)

    # GLEW include directories and source.
    include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/glew/include)
    set(Source_GLEW_for_Windows 3rdParty/glew/src/glew.c)

    # Windows specific sources
    set (SpecificSources
      gl-batch-context/src/WGLContext.cpp
      gl-batch-context/src/WGLContext.h
      )

    add_definitions(-DGL_CONTEXT_USING_WIN)
  endif(WIN32)
endif ()


#-----------------------------------------------------------------------
# Source
#-----------------------------------------------------------------------

# Globbing has some downsides, but the advantages outweigh the
# disadvantages.
file (GLOB Sources
  "gl-batch-context/*.cpp"
  "gl-batch-context/*.h"
  )

#-----------------------------------------------------------------------
# Library setup
#-----------------------------------------------------------------------

# Build the library.
add_library(${CPM_LIB_TARGET_NAME} ${Sources})
target_link_libraries(${CPM_LIB_TARGET_NAME} ${CPM_LIBRARIES})

