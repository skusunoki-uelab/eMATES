/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file PedestrianGeneratorBuilder.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __PEDESTRIAN_GENERATOR_BUILDER_HPP__
#define __PEDESTRIAN_GENERATOR_BUILDER_HPP__

class RoadMap;
class PedestrianGenerator;

//######################################################################
/**
 * @~japanese 歩行者の発生に関する設定を入力する
 * @~english  Input pedestrian generation setting
 * @~ @ingroup IO PedSim
 */
class PedestrianGeneratorBuilder
{
public:
    PedestrianGeneratorBuilder(RoadMap* roadMap)
    {
        _roadMap   = roadMap;
        _generator = nullptr;
    };
    ~PedestrianGeneratorBuilder() {};

    /**
     * @~japanese 歩行者生成器を生成して戻す
     * @~english  Build and return pedestrian generator
     */
    PedestrianGenerator* buildPedestrianGenerator();

private:
    /**
     * @~japanese 歩行者の発生交通量を読み込む
     * @~english  Read pedestrian generation volume
     */
    void _readGeneratingVolume();

private:
    /**
     * @~japanese 歩行者生成器
     * @~english  Pedestrian generator
     */
    PedestrianGenerator* _generator;

    /**
     * @~japanese 地図オブジェクト
     * @~english  Road map object
     */
    RoadMap* _roadMap;
};

#endif //__PEDESTRIAN_GENERATOR_BUILDER_HPP__
#endif //INCLUDE_PEDESTRIANS
