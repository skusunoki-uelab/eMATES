/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RouteCacheReader.hpp
 */
#ifndef __ROUTE_CACHE_READER_HPP__
#define __ROUTE_CACHE_READER_HPP__
#include "../RoadMap.hpp"

//######################################################################
/// 経路に関する入出力を行うクラス
/**
 * @~japanese 経路探索結果のキャッシュをファイルから読み込む
 * @~english  Read route search result cache from file
 */
class RouteCacheReader
{
public:
    RouteCacheReader() {};
    ~RouteCacheReader() {};

    /**
     * @~japanese 経路探索結果をファイルから読み込む
     * @~english  Read route search result from file
     */
    void readRouteCache(RoadMap* roadMap);
};

#endif //__ROUTE_CACHE_READER_HPP__
