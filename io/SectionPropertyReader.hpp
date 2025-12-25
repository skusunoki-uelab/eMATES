/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file SectionPropertyBuilder.hpp
 */
#ifndef __SECTION_PROPERTY_READER_HPP__
#define __SECTION_PROPERTY_READER_HPP__
#include "../RoadMap.hpp"

//##############################################################################
/**
 * @~japanese ファイルを読み込み単路部の属性を設定する
 * @~english  Read file and set section properties
 * @~ @ingroup IO RoadNetwork Initialization
 */
class SectionPropertyReader
{
public:
    SectionPropertyReader(RoadMap* roadMap)
    {
        _roadMap = roadMap;
    };
    ~SectionPropertyReader() {};

    /**
     * @~japanese 制限速度をファイルから読み込む
     * @~english  Read speed limit from file
     */
    bool setSpeedLimit();

    /**
     * @~japanese 通行規制をファイルから読み込む
     * @~english  Read traffic regulations from file
     */
    bool setTrafficControl();

    /**
     * @~japanese 選択確率をファイルから読み込む
     * @~english  Read search probability from file
     */
    bool setRoutingProbability();

private:
    /**
     * @~japanese 作成対象のRoadMapオブジェクト
     * @~english  RoadMap object to be built
     */
    RoadMap* _roadMap;
};

#endif //__SECTION_PROPERTY_READER_HPP__
