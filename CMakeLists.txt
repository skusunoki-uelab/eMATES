########################################################################
# Copyright 2014-2024 ADVENTURE Project
# All Rights Reserved
########################################################################
cmake_minimum_required(VERSION 3.5)
project(AdvMates_core)
set(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(CMAKE_CONFIG_DIR "${PROJECT_ROOT_DIR}/config/cmake")

#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

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

# make advmates-calc (default: true)
set(BUILD_MATES_CALC TRUE)

# make advmates-sim (default: true)
set(BUILD_MATES_SIM TRUE)
if(${PLATFORM} STREQUAL "novis")
  set(BUILD_MATES_SIM FALSE)
endif()

# using OpenMP (default: true w/o Cygwin, MinGW)
option(USE_OPENMP "Using OpenMP" ON)
if(${PLATFORM} STREQUAL "cygwin"
    OR ${PLATFORM} STREQUAL "mingw")
  set(USE_OPENMP OFF)
endif()

# additional build options for windows
BuildOptions_set_compile_options_for_windows()

#----------------------------------------------------------------------
# set path
#----------------------------------------------------------------------
include(${CMAKE_CONFIG_DIR}/SetPaths.cmake)

# set MATESUTL_(HEADER|LIBRARY)_DIR and LIBMATESUTL
SetPaths_set_matesutil_dirs()

# set AUTOGL_(HEADER|LIBRARY)_DIR and LIBAUTOGL
SetPaths_set_autogl_dirs()

# set XSADD_(HEADER|LIBRARY)_DIR and LIBXSADD
SetPaths_set_xsadd_dirs()

# install directory
SetPaths_set_install_dir()

#----------------------------------------------------------------------
# find packages
#----------------------------------------------------------------------
include(${CMAKE_CONFIG_DIR}/UseModules.cmake)

# zlib
#if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "winnative")
  UseModules_find_packages_for_zlib()
  UseModules_includes_for_zlib()
#endif()

# OpenMP
if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "winnative")
  UseModules_find_packages_for_openmp()
endif()

if(BUILD_MATES_SIM)
  # AutoGL
  # UseModules_includes_for_autogl() is called in autogl/CMaleLists.txt
  UseModules_find_packages_for_autogl()
  # OpenCV
  if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "winnative")
    UseModules_find_packages_for_opencv()
    UseModules_includes_for_opencv()
  endif()
endif()

#######################################################################
# simulation settings
#######################################################################

# set bidirectional traffic rule
# - which side of two-way roads does a car drive on?
# (default: left)
if(NOT DEFINED BIDIRECTIONAL_TRAFFIC_RULE)
  set(BIDIRECTIONAL_TRTAFFIC_RULE "left")
else()
  string(TOLOWER "${BIDIRECTIONAL_TRAFFIC_RULE}"
    BIDIRECTIONAL_TRAFFIC_RULE)
  if (NOT ${BIDIRECTIONAL_TRAFFIC_RULE} STREQUAL "right")
    set(BIDIRECTIONAL_TRAFFIC_RULE "left")
  else()
    add_definitions("-DRIGHT_HAND_TRAFFIC")
  endif()
endif()

# whether to include vehicles
# (default: ON(yes))
option(INCLUDE_VEHICLES
  "Define INCLUDE_VEHICLES macro" ON)
if(INCLUDE_VEHICLES)
  add_definitions("-DINCLUDE_VEHICLES")
endif()

# whether to include trams
# (default: OFF(no))
option(INCLUDE_TRAMS
  "Define INCLUDE_TRAMS macro" OFF)
if(INCLUDE_TRAMS)
  add_definitions("-DINCLUDE_TRAMS")
endif()

# whether to include pedestrians
# (default: OFF(no))
option(INCLUDE_PEDESTRIANS
  "Define INCLUDE_PEDESTRIANS macro" OFF)
if(INCLUDE_PEDESTRIANS)
  add_definitions("-DINCLUDE_PEDESTRIANS")
endif()

# whether to generate vehicles with the speed of 0 [km/h]
# (defaultlt: ON(yes))
option(GENERATE_VEHICLE_VELOCITY_ZERO
  "Define GENERATE_VEHICLE_VELOCITY_ZERO macro" ON)
if(GENERATE_VEHICLE_VELOCITY_ZERO)
  add_definitions("-DGENERATE_VEHICLE_VELOCITY_ZERO")
endif()

# whether to permit only one connected network
# (default: ON(yes))
option(UNIQUE_NETWORK
  "Define UNIQUE_NETWORK macro" ON)
if(UNIQUE_NETWORK)
  add_definitions("-DUNIQUE_NETWORK")
endif()

# whether to model branch lanes as polylines
# (default: OFF(no))
option(POLYLINE_BRANCH_LANE
  "Define POLYLINE_BRANCH_LANE" OFF)
if(POLYLINE_BRANCH_LANE)
  add_definitions("-DPOLYLINE_BRANCH_LANE")
endif()

# whether to measure calculation time for function groups
# (default: OFF(no))
option(MEASURE_TIME
  "Define MEASURE_TIME macro" ON)
if(MEASURE_TIME)
  add_definitions("-DMEASURE_TIME")
endif()

# whether to enable logger
# (default: OFF(no))
option(ENABLE_LOGGER
  "Define ENABLE_LOGGER macro" OFF)
if(ENABLE_LOGGER)
  add_definitions("-DENABLE_LOGGER")
endif()

#######################################################################
# source
#######################################################################
set(MATES_COMMON_SRCS
  AppMates.cpp
  Blinker.cpp
  Border.cpp
  Clocker.cpp
  ClockerManager.cpp
  ConvoyMonitor.cpp
  CustomMessage.cpp
  GeneratingTable.cpp
  GVInitializer.cpp
  GVManager.cpp
  FileManager.cpp
  InflowMonitor.cpp
  Intersection.cpp
  Lane.cpp
  LaneBundle.cpp
  LaneInIntersection.cpp
  LaneInSection.cpp
  LinkFlowRecord.cpp
  LinkFlowMonitor.cpp
  LocalLaneRouter.cpp
  LoggerManager.cpp
  ManagerBase.cpp
  ManagerPool.cpp
  ObjectManager.cpp
  ODNode.cpp
  ODNodeGroup.cpp
  RandomNumberGenerator.cpp
  RelativeDirectionTable.cpp
  RoadMap.cpp
  Route.cpp
  RouteCache.cpp
  RouteCacheContainer.cpp
  RouteKeyBase.cpp
  RouteKeyHierarchy.cpp
  RouterAStar.cpp
  RouterAStarHierarchy.cpp
  RouterBase.cpp
  RouterDijkstra.cpp
  RouterManager.cpp
  RoutingLink.cpp
  RoutingNetwork.cpp
  RoutingNode.cpp
  RoutingProbability.cpp
  RoutingRecorder.cpp
  ScheduleItemBase.cpp
  ScheduleManager.cpp
  Section.cpp
  Signal.cpp
  SignalAspect.cpp
  SignalCycle.cpp
  SignalSplit.cpp
  Simulator.cpp
  SpeedLimitItem.cpp
  SubIntersection.cpp
  SubLaneBundle.cpp
  SubSection.cpp
  TrafficCounter.cpp
  TrafficCounterComponent.cpp
  Vehicle.cpp
  VehicleActor.cpp
  VehicleBehavior.cpp
  VehicleBodyProperty.cpp
  VehicleDecision.cpp
  VehicleDeterminer.cpp
  VehicleGenerator.cpp
  VehicleGlobalRoute.cpp
  VehicleLaneChangeActor.cpp
  VehicleLaneChangeDeterminer.cpp
  VehicleLaneChangePerceiver.cpp
  VehicleLocalRoute.cpp
  VehicleLocation.cpp
  VehicleRestriction.cpp
  VehiclePerceiver.cpp
  VehicleScene.cpp
  VehicleTypeManager.cpp
  VirtualLeader.cpp
  io/ConvoyMonitorBuilder.cpp
  io/ConvoyMonitorWriter.cpp
  io/GVReader.cpp
  io/InflowMonitorBuilder.cpp
  io/InflowMonitorWriter.cpp
  io/IntersectionBuildDirector.cpp
  io/IntersectionBuilder.cpp
  io/IntersectionBuilder_Border.cpp
  io/IntersectionBuilder_Lane.cpp
  io/IntersectionBuilder_RDTable.cpp
  io/IntersectionBuilder_RoadwayVertex.cpp
  io/IntersectionBuilder_Subsection.cpp
  io/IntersectionBuilder_Vertex.cpp
  io/IntersectionPropertyReader.cpp
  io/LaneBundleBuildDirector.cpp
  io/LaneBundleBuilder.cpp
  io/LinkFlowMonitorBuilder.cpp
  io/LinkFlowMonitorWriter.cpp
  io/Logger.cpp
  io/ODNodeBuilder.cpp
  io/RoadMapBuilder.cpp
  io/RoadMapShapeWriter.cpp
  io/RouteCacheReader.cpp
  io/RouteCacheWriter.cpp
  io/RoutingNetworkBuilder.cpp
  io/SectionBuildDirector.cpp
  io/SectionBuilder.cpp
  io/SectionBuilder_Lane.cpp
  io/SectionBuilder_Subsection.cpp
  io/SectionBuilder_Vertex.cpp
  io/SectionPropertyReader.cpp
  io/SignalBuilder.cpp
  io/SignalTimeSeriesWriter.cpp
  io/TrafficCounterBuilder.cpp
  io/TrafficCounterWriter.cpp
  io/VehicleTimeSeriesWriter.cpp
  io/VehicleGeneratorBuilder.cpp
  io/VehicleTripWriter.cpp
  io/VehicleTypeBuilder.cpp
  io/VehicleTypeWriter.cpp
  ped/InflowPedestrianMonitor.cpp
  ped/InflowPedestrianMonitorBuilder.cpp
  ped/InflowPedestrianMonitorWriter.cpp
  ped/IntersectionPedExt.cpp
  ped/LanePedExt.cpp
  ped/ObjectManagerPedExt.cpp
  ped/Pedestrian.cpp
  ped/PedestrianActor.cpp
  ped/PedestrianBehavior.cpp
  ped/PedestrianBodyProperty.cpp
  ped/PedestrianDeterminer.cpp
  ped/PedestrianGenerator.cpp
  ped/PedestrianGeneratorBuilder.cpp
  ped/PedestrianLocation.cpp
  ped/PedestrianPerceiver.cpp
  ped/PedestrianScene.cpp
  ped/PedestrianTimeSeriesWriter.cpp
  ped/SimulatorPedExt.cpp
  ped/VehiclePedExt.cpp
  ped/Zebra.cpp
  ped/ZebraODEdge.cpp
  tram/BorderTram.cpp
  tram/IntersectionBuilderTramExt.cpp
  tram/IntersectionTramExt.cpp
  tram/ObjectManager.cpp
  tram/ODNodeBuilderTramExt.cpp
  tram/ODNodeTramExt.cpp
  tram/SectionBuilderTramExt.cpp
  tram/SectionTramExt.cpp
  tram/TramLaneInIntersection.cpp
  tram/TramLaneInSection.cpp
  tram/TramRoute.cpp
  tram/TramRouteManager.cpp
  tram/VehicleTram.cpp

  # EV-related [eMATES]
  VehicleEV.cpp
  VehicleEVActor.cpp
  Charger.cpp
  CSNodeBase.cpp
  CSNodeFast.cpp
  CSNodeNormal.cpp
  io/EVCommIO.cpp
  io/CSNodeFastBuilder.cpp
  io/CSNodeNormalBuilder.cpp
)

set(MATES_CALC_SRCS
  AppCalc.cpp
)

set(MATES_SIM_SRCS
  AppSim.cpp
  Clocker.cpp
  ClockerManager.cpp
  ped/DrawerForPedestrian.cpp
  ped/GuiPedestrianView.cpp
  vis/BgImage.cpp
  vis/DrawerForIntersection.cpp
  vis/DrawerForLane.cpp
  vis/DrawerForMonitor.cpp
  vis/DrawerForRouting.cpp
  vis/DrawerForSection.cpp
  vis/DrawerForSignal.cpp
  vis/DrawerForSubLaneBundle.cpp
  vis/DrawerForVehicle.cpp
  vis/GLColor.cpp
  vis/GuiBackgroundView.cpp
  vis/GuiMapView.cpp
  vis/GuiObjectSearch.cpp
  vis/GuiRoutingTest.cpp
  vis/GuiSimulationControl.cpp
  vis/GuiViewControl.cpp
  vis/GuiVehicleGeneration.cpp
  vis/GuiVehicleView.cpp
  vis/Visualizer.cpp
  vis/autogl_mates.c
  CSNodeBase.cpp # NOTE 25.03.27 by abe 入れないとリンカがエラーになる?
)

#######################################################################
# build libraries
#######################################################################
SetPaths_set_mates()

add_library(${LIBMATES}
  STATIC
  ${MATES_COMMON_SRCS}
  )

SetPaths_set_matesgl()

if(BUILD_MATES_SIM)
  add_library(${LIBMATESGL}
    STATIC
    ${MATES_SIM_SRCS}
    )
endif()

#######################################################################
# build advmates-calc
#######################################################################
set(MATES_CALC "advmates-calc")
set(MATES_CALC_MAIN_SRC mainCalc.cpp)
if(BUILD_MATES_CALC)
  add_executable(${MATES_CALC}
    ${MATES_CALC_SRCS}
    ${MATES_CALC_MAIN_SRC}
    )
  target_link_libraries(
    ${MATES_CALC}
    ${LIBMATES}
    m
    )
  UseModules_target_link_libraries_for_matesutil(${MATES_CALC})
  UseModules_target_link_libraries_for_xsadd(${MATES_CALC})
  UseModules_target_link_libraries_for_zlib(${MATES_CALC})
  install(TARGETS ${MATES_CALC} DESTINATION bin)
endif()

#######################################################################
# build advmates-sim
#######################################################################
set(MATES_SIM "advmates-sim")
set(MATES_SIM_MAIN_SRC  mainSim.cpp)
if(BUILD_MATES_SIM)
  add_executable(${MATES_SIM}
    ${MATES_SIM_MAIN_SRC}
  )
  target_link_libraries(
    ${MATES_SIM}
    ${LIBMATES}
    ${LIBMATESGL}
    m
  )
  UseModules_target_link_libraries_for_matesutil(${MATES_SIM})
  UseModules_target_link_libraries_for_xsadd(${MATES_SIM})
  UseModules_target_link_libraries_for_autogl(${MATES_SIM})
  UseModules_target_link_libraries_for_zlib(${MATES_SIM})
  UseModules_target_link_libraries_for_opencv(${MATES_SIM})
  install(TARGETS ${MATES_SIM} DESTINATION bin)
endif()

#----------------------------------------------------------------------
# phony targets
#----------------------------------------------------------------------
ProcessCMake_set_clean_target()
ProcessCMake_set_check_syntax_target()
ProcessCMake_set_cppcheck_target()

#----------------------------------------------------------------------
# for doxygen
#----------------------------------------------------------------------
set(MATES_DOXYGEN_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/../doc/doxygen")

UseModules_find_packages_for_doxygen()
if(DOXYGEN_FOUND)
  message(${DOXYGEN_EXECUTABLE})
  if(DOXYGEN_DOT_FOUND)
    set(MATES_DOXYGEN_CONF_FULL_JA
      "${MATES_DOXYGEN_DIR}/config/advmates-dox-full-ja"
      )
    set(MATES_DOXYGEN_CONF_FULL_EN
      "${MATES_DOXYGEN_DIR}/config/advmates-dox-full-en"
      )
  else()
    set(MATES_DOXYGEN_CONF_FULL_JA
      "${MATES_DOXYGEN_DIR}/config/advmates-dox-min-ja"
      )
    set(MATES_DOXYGEN_CONF_FULL_EN
      "${MATES_DOXYGEN_DIR}/config/advmates-dox-min-en"
      )
  endif()
  set(MATES_DOXYGEN_CONF_JA
    "${MATES_DOXYGEN_DIR}/config/advmates-dox-min-ja"
    )
  set(MATES_DOXYGEN_CONF_EN
    "${MATES_DOXYGEN_DIR}/config/advmates-dox-min-en"
    )
  add_custom_target(dox-full-ja
    COMMAND ${DOXYGEN_EXECUTABLE} ${MATES_DOXYGEN_CONF_FULL_JA}
    WORKING_DIRECTORY ${MATES_DOXYGEN_DIR}
    )
  add_custom_target(dox-full-en
    COMMAND ${DOXYGEN_EXECUTABLE} ${MATES_DOXYGEN_CONF_FULL_EN}
    WORKING_DIRECTORY ${MATES_DOXYGEN_DIR}
    )
  add_custom_target(dox-ja
    COMMAND ${DOXYGEN_EXECUTABLE} ${MATES_DOXYGEN_CONF_JA}
    WORKING_DIRECTORY ${MATES_DOXYGEN_DIR}
    )
  add_custom_target(dox-en
    COMMAND ${DOXYGEN_EXECUTABLE} ${MATES_DOXYGEN_CONF_EN}
    WORKING_DIRECTORY ${MATES_DOXYGEN_DIR}
    )
endif()

#----------------------------------------------------------------------
# print status
#----------------------------------------------------------------------
message("cmake status ...")
message("USE_OPENMP                     : ${USE_OPENMP}")
message("USE_OPENCV                     : ${USE_OPENCV}")
message("BIDIRECTIONAL_TRAFFIC_RULE     : ${BIDIRECTIONAL_TRAFFIC_RULE}")
message("INCLUDE_VEHICLES               : ${INCLUDE_VEHICLES}")
message("INCLUDE_TRAMS                  : ${INCLUDE_TRAMS}")
message("INCLUDE_PEDESTRIANS            : ${INCLUDE_PEDESTRIANS}")
message("GENERATE_VEHICLE_VELOCITY_ZERO : ${GENERATE_VEHICLE_VELOCITY_ZERO}")
message("UNIQUE_NETWORK                 : ${UNIQUE_NETWORK}")
message("POLYLINE_BRANCH_LANE           : ${POLYLINE_BRANCH_LANE}")
message("MEASURE_TIME                   : ${MEASURE_TIME}")
message("ENABLE_LOGGER                  : ${ENABLE_LOGGER}")
message("FM_FLAGS    : ${FM_FLAGS}")
message("FM_INCLUDES : ${FM_INCLUDES}")

#----------------------------------------------------------------------
# end cmake
#----------------------------------------------------------------------
ProcessCMake_end_cmake()
