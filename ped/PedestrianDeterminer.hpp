/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file PedestrianDeterminer.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __PEDESTRIAN_DETERMINER_HPP__
#define __PEDESTRIAN_DETERMINER_HPP__
#include <AmuPoint.hpp>

class Pedestrian;
class PedestrianPerceiver;
class Vehicle;

struct PedestrianBehavior;
struct PedestrianBodyProperty;
struct PedestrianDecision;
struct PedestrianLocation;
struct PedestrianScene;

//######################################################################
/**
 * @~japanese 歩行者エージェントの意思決定機能モジュール
 *
 * PedestrianPerceiver の結果をもとに判断する．
 *
 * @~english  Decision-making module of pedestrian agent
 *
 * Make decision based on the result of PedestrianPerceiver .
 *
 * @~ @ingroup Pedestrian
 */
class PedestrianDeterminer
{
public:
    PedestrianDeterminer()
    {
        _pedestrian = nullptr;
        _body       = nullptr;
        _behavior   = nullptr;
        _decision   = nullptr;
        _location   = nullptr;
        _perceiver  = nullptr;
        _scene      = nullptr;
    }
    ~PedestrianDeterminer() {}

    /**
     * @~japanese 所有者とその部分クラスオブジェクトを登録する
     * @~english  Set owner and its part class objects
     */
    void setPedestrian(
        Pedestrian* ped, PedestrianBehavior* behavior,
        PedestrianBodyProperty* body, PedestrianDecision* decision,
        PedestrianLocation* location, PedestrianPerceiver* perceiver,
        PedestrianScene* scene);

    /**
     * @~japanese 意思決定する
     * @~english  Make decision
     */
    void determine();

private:
    /**
     * @~japanese 最近接歩行者に対する行動を決定する
     * @~english  Decide action against the nearest pedestrian
     */
    void _determineBehaviorAgainstPedestrian();

    /**
     * @~japanese このステップにおける目標位置を定める
     *
     * @param ped 最近接歩行者
     * @param longitudinalGap @p ped と目標位置の縦方向距離 (後ろが正)
     * @param lateralGap @p ped と目標位置の側方距離 (右が正)
     *
     * @~english  Determine the target position for this step
     *
     * @param ped Nearest pedestrian
     * @param longitudinalGap Longitudinal gap between @p ped and target
     * position (rear is positive)
     * @param lateralGap Lateral gap between @p ped and target position
     * (right is positive)
     */
    void _determineTargetPosition(
        const Pedestrian* ped, double longitudinalGap, double lateralGap);

private:
    /**
     * @~japanese 所有者
     * @~english  Owner
     */
    Pedestrian* _pedestrian;

    //==================================================================
    /**
     * @~japanese
     * @name 所有者の部分クラスオブジェクトへのポインタ
     *
     * @~english
     * @name Pointers to part class objects of the owner
     */
    ///@{
private:
    const PedestrianBodyProperty* _body;
    const PedestrianBehavior*     _behavior;
    PedestrianDecision*           _decision;
    const PedestrianLocation*     _location;
    PedestrianPerceiver*          _perceiver;
    const PedestrianScene*        _scene;

    ///@}
};

#endif //__PEDESTRIAN_H__
#endif //INCLUDE_PEDESTRIANS
