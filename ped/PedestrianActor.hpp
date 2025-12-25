/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file PedestrianActor.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __PEDESTRIAN_ACTOR_HPP__
#define __PEDESTRIAN_ACTOR_HPP__

class Pedestrian;
class PedestrianDeterminer;

struct PedestrianBehavior;
struct PedestrianDecision;
struct PedestrianLocation;

//######################################################################
/**
 * @~japanese 歩行者エージェントの行動機能モジュール
 *
 * PedestrianDeterminer の結果をもとに行動し， PedestrianBehavior ,
 * PedestrianLocation を更新する．
 *
 * @~english  Action module of car agent
 *
 * Act based on the result of PedestrianDeterminer and update
 * PedestrianBehavior and  PedestrianLocation.
 *
 * @~ @ingroup Pedestrian
 */
class PedestrianActor
{
public:
    PedestrianActor()
    {
        _pedestrian = nullptr;
        _behavior   = nullptr;
        _decision   = nullptr;
        _location   = nullptr;
    }
    ~PedestrianActor() {}

    /**
     * @~japanese 所有者とその部分クラスオブジェクトを登録する
     * @~english  Set owner and its part class objects
     */
    void setPedestrian(
        Pedestrian* ped, PedestrianBehavior* behavior,
        PedestrianDecision* decision, PedestrianLocation* location);

    //==================================================================
    /**
     * @~japanese @name 行動
     * @~english  @name Action
     */
    ///@{
public:
    /**
     * @~japanese 行動する
     * @~english  act
     */
    void act();

private:
    /**
     * @~japanese 速度を更新する
     * @~english  Update velocity
     */
    void _renewVelocity();

    ///@}

    //==================================================================
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
    PedestrianBehavior*       _behavior;
    const PedestrianDecision* _decision;
    PedestrianLocation*       _location;

    ///@}
};

#endif //__PEDESTRIAN_ACTOR_HPP__
#endif //INCLUDE_PEDESTRIANS
