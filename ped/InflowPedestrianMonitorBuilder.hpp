/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file InflowPedestrianMonitorBuilder.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __INFLOW_PEDESTRIAN_MONITOR_BUILDER_HPP__
#define __INFLOW_PEDESTRIAN_MONITOR_BUILDER_HPP__
#include "../RoadMap.hpp"
#include <vector>

class ZebraODEdge;

//##############################################################################
/**
 * @~japanese ファイルを読み込み流入歩行者検出器を生成する
 * @~english  Read file and generate inflow pedestrian monitor
 * @~ * @ingroup IO Initialization Monitoring PedSim
 */
class InflowPedestrianMonitorBuilder
{
public:
    InflowPedestrianMonitorBuilder() {};
    ~InflowPedestrianMonitorBuilder() {};

    /**
     * @~japanese ファイルを読み込み流入歩行者検出器を生成する
     * @~english  Read file and generate inflow pedestrian monitors
     */
    void buildInflowPedestrianMonitors(RoadMap* roadMap);
};

#endif //__INFLOW_PEDESTRIAN_MONITOR_BUILDER_HPP__
#endif //INCLUDE_PEDESTRIANS
