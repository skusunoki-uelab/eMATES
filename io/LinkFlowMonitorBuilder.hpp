/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file LinkFlowMonitorBuilder.hpp
 */
#ifndef __LINK_FLOW_MONITOR_BUILDER_HPP__
#define __LINK_FLOW_MONITOR_BUILDER_HPP__
#include "../RoadMap.hpp"

//######################################################################
/**
 * @~japanese ファイルを読み込みリンク交通流観測器を生成する
 * @~english  Read file and generate link traffic flow monitors
 * @~ @ingroup Initialization IO Monitoring
 */
class LinkFlowMonitorBuilder
{
public:
    LinkFlowMonitorBuilder() {}
    ~LinkFlowMonitorBuilder() {}

    /**
     * @~japanese ファイルを読み込みリンク旅行時間観測器を生成する
     * @~english  Read file and generate link travel time monitors
     */
    void buildLinkFlowMonitors(RoadMap* roadMap);
};

#endif //__LINK_FLOW_MONITOR_BUILDER_HPP__
