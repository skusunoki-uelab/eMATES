/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file InflowMonitorBuilder.hpp
 */
#ifndef __INFLOW_MONITOR_BUILDER_HPP__
#define __INFLOW_MONITOR_BUILDER_HPP__
#include "../RoadMap.hpp"
#include "../ODNode.hpp"
#include <vector>

//##############################################################################
/**
 * @~japanese ファイルを読み込み流入車両検知器を生成する
 * @~english  Read file and generate inflow vehicle monitor
 * @~ @ingroup Initialization IO Monitoring
 */
class InflowMonitorBuilder
{
public:
    InflowMonitorBuilder() {};
    ~InflowMonitorBuilder() {};

    /**
     * @~japanese ファイルを読み込み流入車両検出器を生成する
     * @~english  Read file and generate inflow vehicle monitor
     */
    void buildInflowMonitors(RoadMap* roadMap);
};

#endif //__INFLOW_MONITOR_BUILDER_HPP__
