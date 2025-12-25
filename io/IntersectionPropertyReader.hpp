/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file IntersectionPropertyReader.hpp
 */
#ifndef __INTERSECTION_PROPERTY_READER_HPP__
#define __INTERSECTION_PROPERTY_READER_HPP__
#include "../RoadMap.hpp"

//##############################################################################
/**
 * @~japanese ファイルを読み込み交差点の属性を設定する
 * @~english  Read file and set intersection properties
 * @~ @ingroup IO RoadNetwork Initialization
 */
class IntersectionPropertyReader
{
public:
    IntersectionPropertyReader(RoadMap* roadMap)
    {
        _roadMap = roadMap;
    }
    ~IntersectionPropertyReader() {}

    /**
     * @~japanese 通行規制をファイルから読み込む
     * @~english  Read traffic regulations from file
     */
    bool setTrafficControl();

private:
    /**
     * @~japanese 作成対象のRoadMapオブジェクト
     * @~english  RoadMap object to be built
     */
    RoadMap* _roadMap;
};


#endif //__INTERSECTION_PROPERTY_READER_HPP__