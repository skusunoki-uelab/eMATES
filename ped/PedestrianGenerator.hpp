/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file PedestrianGenerator.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __PEDESTRIAN_GENERATOR_HPP__
#define __PEDESTRIAN_GENERATOR_HPP__
#include "../Config.hpp"
#include "../RandomNumberGenerator.hpp"

class RoadMap;
class Intersection;
class Zebra;
class Pedestrian;

//######################################################################
/**
 * @~japanese 歩行者の発生を制御する
 *
 * @note
 * 車両とは異なり，現段階の実装では，各ステップにおいて歩行者の発生を
 * 確率的に決定している．またこのクラスは GeneratingTable に相当する
 * ものを持たず，各 Zebra がそこに適用される発生確率を持つ．
 *
 * @~english  Control pedestrian generation
 *
 * @note
 * Unlike cars, in the current implementation, the generation of
 * pedestrians is determined stochastically at each step. And this
 * class does not have something equivalent to GeneratingTable , but
 * each Zebra object has an generation probability applied to itself.
 *
 * @~ @ingroup PedSim Running
 */
class PedestrianGenerator
{
public:
    PedestrianGenerator(RoadMap* roadMap);
    ~PedestrianGenerator();

    /**
     * @~japanese 歩行者を発生させる
     * @~english  Generate pedestrians
     */
    void generatePedestrians();

private:
    /**
     * @~japanese
     * 歩行者を確率的に生成して @p zebra の _waitingPedestrians に
     * 追加する
     *
     * @~english
     * Generate a pedestrian stochastically and add it to @p zebra 's
     * _waitingPedestrians . 
     */
    void _generateWaitingPedestrian(Zebra* zebra);

    /**
     * @~japanese
     * @p zebra の @p dir 方向に歩行者を発生させるかどうかを戻す
     *
     * @~english
     * Return whether to generate a pedestrian in direction @p dir of
     * @p zebra
     */
    bool _canGeneratePedestrian(Zebra* zebra, int dir);

private:
    /**
     * @~japanese 地図オブジェクト
     * @~english  Road map object
     */
    RoadMap* _roadMap;

    /**
     * @~japanese 乱数生成器
     * @~english  Random number generator
     */
    RandomNumberGenerator _rng;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // to be re-implemented
    // 発生歩行者データを格納する構造体
    struct GeneratedPedestrianData
    {
        Pedestrian*         pedestrian;
        const Intersection* intersection;
        const Zebra*        zebra;
        ulint               startTime;
    };

    // 発生歩行者データ
    //std::vector<GeneratedPedestrianData> _nodeGvd;

    // 発生車両データを出力するかどうか
    bool _isOutputGeneratePedestrianData;


    // 発生歩行者データ出力フラグを設定する
    void setOutputGeneratePedestrianDataFlag(bool flag)
    {
        _isOutputGeneratePedestrianData = flag;
    }
};

#endif //__PEDESTRIAN_GENERATOR_HPP__
#endif //INCLUDE_PEDESTRIANS
