/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file PedestrianDecision.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __PEDESTRIAN_DECISION_HPP__
#define __PEDESTRIAN_DECISION_HPP__
#include <AmuPoint.hpp>

class Pedestrian;

//######################################################################
/**
 * @~japanese 歩行行動の種類
 * @~english  Types of walking behavior
 */
enum class WalkType : unsigned int
{
    FREEWALK,
    OVERTAKE,
    FOLLOW,
    AVOID,
    STOP,
    NONE,
};

//######################################################################
/**
 * @~japanese
 * 歩行者エージェントの行動生成に必要な意思決定結果を集約する構造体
 *
 * PedestrianDeterminer によって設定され，PedestrianActor が利用．
 *
 * @~english
 * Struct aggregating decision-making results necessary for behavior
 * generation of the pedestrian agent
 *
 * Set by PedestrianDeterminer , and used by VehicleActor .
 *
 * @~
 * @ingroup Pedestrian
 */
struct PedestrianDecision
{
public:
    PedestrianDecision()
    {
        _pedestrian = nullptr;
        _walkType   = WalkType::NONE;
    }
    ~PedestrianDecision() {}

private:
    /**
     * @~japanese 歩行行動
     * @~english  Walking behavior
     */
    WalkType _walkType;

    /**
     * @~japanese 目標位置
     * @~english  Target position
     */
    amu::geometry::AmuPoint _targetPosition;

private:
    /**
     * @~japanese 所有者
     * @~english  Owner
     */
    const Pedestrian* _pedestrian;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    void setPedestrian(const Pedestrian* ped)
    {
        _pedestrian = ped;
    }

    void setWalkType(WalkType type)
    {
        _walkType = type;
    }

    WalkType walkType() const
    {
        return _walkType;
    }

    const amu::geometry::AmuPoint& targetPosition() const
    {
        return _targetPosition;
    }

    void setTargetPosition(const amu::geometry::AmuPoint& point)
    {
        _targetPosition = point;
    }

    ///@}
};

#endif //__PEDESTRIAN_DECISION_HPP__
#endif //INCLUDE_PEDESTRIANS
