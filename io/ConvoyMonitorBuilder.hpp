/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file ConvoyMonitorReader.hpp
 */
#ifndef __CONVOY_MONITOR_BUILDER_HPP__
#define __CONVOY_MONITOR_BUILDER_HPP__
#include "../RoadMap.hpp"

//######################################################################
/**
 * @~japanese ファイルを読み込み車列観測器を生成する
 * @~english  Read file and generate vehicle convoy monitor
 * @~ @ingroup Initialization IO Monitoring
 */
class ConvoyMonitorBuilder
{
public:
    ConvoyMonitorBuilder() {};
    ~ConvoyMonitorBuilder() {};

    /**
     * @~japanese ファイルを読み込み車列観測器を生成する
     * @~english  Read file and generate vehicle convoy monitors
     */
    void buildConvoyMonitors(RoadMap* roadMap);
};

#endif //__CONVOY_MONITOR_BUILDER_HPP__
