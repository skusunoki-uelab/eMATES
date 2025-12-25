/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RouteCacheWriter.hpp
 */
#ifndef __ROUTE_CACHE_WRITER_HPP__
#define __ROUTE_CACHE_WRITER_HPP__
#include "../RoadMap.hpp"

//######################################################################
/**
 * @~japanese 経路探索結果のキャッシュをファイルに書き込む
 * @~english  Write route search result cache from file
 */
class RouteCacheWriter
{
public:
    RouteCacheWriter() {};
    ~RouteCacheWriter() {};

    /**
     * @~japanese 経路探索結果をファイルに書き込む
     * @~english  Write route search result to file
     */
    void writeRouteCache(RoadMap* roadMap);
};

#endif //__ROUTE_CACHE_WRITER_HPP__
