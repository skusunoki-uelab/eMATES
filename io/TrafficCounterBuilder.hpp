/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file TrafficCounterBuilder.hpp
 */
#ifndef __TRAFFIC_COUNTER_BUILDER_HPP__
#define __TRAFFIC_COUNTER_BUILDER_HPP__
#include "../RoadMap.hpp"
#include "../TrafficCounter.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

//######################################################################
/**
 * @~japanese ファイルを読み込み車両感知器を生成する
 * @~english  Read file and generate traffic counter
 * @~ @ingroup Initialization IO Monitoring 
 */
class TrafficCounterBuilder
{
public:
    TrafficCounterBuilder() {};
    ~TrafficCounterBuilder() {};

    /**
     * @~japanese ファイルを読み込み車両感知器を生成する
     * @~english  Read file and generate traffic counter
     */
    void buildTrafficCounters(RoadMap* roadMap);
};

#endif //__TRAFFIC_COUNTER_BUILDER_HPP__
