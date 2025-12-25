########################################################################
# Copyright 2014-2024 ADVENTURE Project
# All Rights Reserved
########################################################################

#**********************************************************************
# This configuration is only for creating check-syntax target
#**********************************************************************
cmake_minimum_required(VERSION 3.5)
project(AdvMates_core_pedestrian)
set(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(CMAKE_CONFIG_DIR "${PROJECT_ROOT_DIR}/config/cmake")

#----------------------------------------------------------------------
# start cmake
#----------------------------------------------------------------------
include(${CMAKE_CONFIG_DIR}/ProcessCMake.cmake)
ProcessCMake_start_cmake()

#----------------------------------------------------------------------
# platform, build type, compile option
#----------------------------------------------------------------------
include(${CMAKE_CONFIG_DIR}/BuildOptions.cmake)

# set PLATFORM, CMAKE_BUILD_TYPE
BuildOptions_set_platform()
BuildOptions_set_build_type()

# set CMAKE_(C|CXX)_COMPILER, CMAKE_(C|CXX)_FLAGS_$CMAKE_BUILD_TYPE
BuildOptions_set_compiler()
BuildOptions_set_compile_options()

# ----------------------------------------------------------------------
# load cache
load_cache(../../)
load_cache(../)

#----------------------------------------------------------------------
# phony targets
#----------------------------------------------------------------------
ProcessCMake_set_clean_target()
ProcessCMake_set_check_syntax_target()
ProcessCMake_set_cppcheck_target()

#----------------------------------------------------------------------
# print status
#----------------------------------------------------------------------
message("cmake status ...")
message("FM_FLAGS    : ${FM_FLAGS}")
message("FM_INCLUDES : ${FM_INCLUDES}")

