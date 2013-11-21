
if(APPLE)
  cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
else()
  cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
endif()

include(ExternalProject)
project(SpireBatchRenderer)

# Base tests directory.
set (BASE_TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../tests)

option(USE_OS_MESA "Use an OS mesa context." OFF)
if (USE_OS_MESA)
  add_definitions(-DUSE_OS_MESA)
  set(MESA_LIBRARIES "OSMesa32")
endif()

#------------------------------------------------------------------------------
# Required CPM Setup - See: http://github.com/iauns/cpm
#------------------------------------------------------------------------------
set(CPM_DIR "${CMAKE_CURRENT_BINARY_DIR}/cpm-packages" CACHE TYPE STRING)
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

CPM_AddModule("spire" SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

CPM_Finish()

#-----------------------------------------------------------------------
# Configure OpenGL
#-----------------------------------------------------------------------
find_package(OpenGL REQUIRED)

#-----------------------------------------------------------------------
# Grab Regression Data From GitHub
#-----------------------------------------------------------------------

set(REGRESSION_DATA_DIR "${BASE_TESTS_DIR}/RegressionData")

CPM_EnsureRepoIsCurrent(
  TARGET_DIR ${REGRESSION_DATA_DIR}
  GIT_REPOSITORY "git://github.com/SCIInstitute/SpireRegression"
  GIT_TAG "origin/master"
  )

#-----------------------------------------------------------------------
# Regression directory setup
#-----------------------------------------------------------------------

# TODO: Make all of these options.
add_definitions(-DTEST_OUTPUT_IMAGES)
add_definitions(-DTEST_IMAGE_OUTPUT_DIR="${REGRESSION_DATA_DIR}/Output")
add_definitions(-DTEST_IMAGE_COMPARE_DIR="${REGRESSION_DATA_DIR}/Verified")
add_definitions(-DTEST_PERCEPTUAL_COMPARE)
add_definitions(-DTEST_PERCEPTUAL_COMPARE_BINARY="perceptualdiff")

#-----------------------------------------------------------------------
# Compiler settings
#-----------------------------------------------------------------------

if (CMAKE_BUILD_TYPE MATCHES DEBUG)
  add_definitions(-DSPIRE_DEBUG)
endif()

if (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

  if (APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")

    # Find necessary apple libraries (cocoa).
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    set(OS_SPECIFIC_LIBS ${COCOA_LIBRARY})

    # Apple specific sources
    set (SpecificSources
      CGLContext.cpp
      CGLContext.h
      NSContext.mm
      NSContext.h
      )
  else ()
    # Linux specific sources
    if (USE_OS_MESA)
      set (SpecificSources
        OSMesaContext.cpp
        OSMesaContext.h
        )
    else()
      set (SpecificSources
        GLXContext.cpp
        GLXContext.h
        )
    endif()
  endif ()
else()
  if (WIN32)
    # Since we are building a static library, we need to define GLEW_STATIC
    # <url:http://glew.sourceforge.net/install.html#^define the GLEW_STATIC>
    add_definitions(-DGLEW_STATIC)

    # GLEW include directories and source.
    include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/glew/include)
    set(Source_GLEW_for_Windows 3rdParty/glew/src/glew.c)

    # Windows specific sources
    set (SpecificSources
      WGLContext.cpp
      WGLContext.h
      )
  endif(WIN32)
endif ()

#-----------------------------------------------------------------------
# External dependencies
#-----------------------------------------------------------------------

# Note: we need to add gtest before we set strict warnings, but after we define
# C++11 settings.

# Gtest
add_definitions(-DGTEST_USE_OWN_TR1_TUPLE=1)
add_subdirectory(${BASE_TESTS_DIR}/3rdParty/gtest ${CMAKE_CURRENT_BINARY_DIR}/gtest)
include_directories(SYSTEM ${BASE_TESTS_DIR}/3rdParty/gtest/include)

# CImg
INCLUDE_DIRECTORIES(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/CImg" )

#-----------------------------------------------------------------------
# Setup strict warnings and werror
#-----------------------------------------------------------------------

if(APPLE)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wconversion -Wshadow")
endif()

#-----------------------------------------------------------------------
# Setup source
#-----------------------------------------------------------------------

# Setup spire test sources. Even though this is a batch renderer, this is
# a good test of concept.
set(CommonSources
  BatchContext.cpp
  BatchContext.h
  main.cpp)

file(GLOB TestSources
  "${BASE_TESTS_DIR}/*.cpp"
  "${BASE_TESTS_DIR}/*.h"
  )

########################################################################
# Copy shaders to the target directory

# Copy shaders.
file(COPY ${BASE_TESTS_DIR}/shaders/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/
     FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
     DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                           GROUP_READ
                           WORLD_READ WORLD_EXECUTE)

# Exported meshes. These are used to test rendering.
file(COPY ${BASE_TESTS_DIR}/assets/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Assets
     FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
     DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                           GROUP_READ
                           WORLD_READ WORLD_EXECUTE)

########################################################################
# Setup executable

add_executable(spirebatch
  ${CommonSources}
  ${TestSources}
  ${SpecificSources}
  ${Source_GLEW_for_Windows})

target_link_libraries(spirebatch 
  gtest
  ${CPM_LIBRARIES}
  ${MESA_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${OS_SPECIFIC_LIBS})

