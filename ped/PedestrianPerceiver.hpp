/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file PedestrianPerceiver.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __PEDESTRIAN_PERCEIVER_HPP__
#define __PEDESTRIAN_PERCEIVER_HPP__

class Pedestrian;
class Vehicle;

struct PedestrianLocation;
struct PedestrianScene;

//######################################################################
/**
 * @~japanese 歩行者エージェントの認知機能モジュール
 *
 * PedestrianScene を更新する．
 *
 * @~english  Recognition module of pedestrian agent
 *
 * Update PedestrianScene .
 *
 * @~ @ingroup Pedestrian
 */
class PedestrianPerceiver
{
public:
    PedestrianPerceiver()
    {
        _pedestrian = nullptr;
        _location   = nullptr;
        _scene      = nullptr;
    }

    ~PedestrianPerceiver() {}

    /**
     * @~japanese 所有者とその部分クラスオブジェクトを登録する
     * @~english  Set owner and its part class objects
     */
    void setPedestrian(
        Pedestrian* ped, const PedestrianLocation* location,
        PedestrianScene* scene);

    /**
     * @~japanese 周囲の状況を認識する
     * @~english  Recognize surroundings
     */
    void perceive();

private:
    /**
     * @~japanese 最近傍歩行者を探す
     * @~english  Find the nearest pedestrian
     */
    void _searchNearestPedestrian();
    /**
     * @~japanese 歩行者が停止すべきレーンを探す
     * @~english  Find lane where the pedestrian should stop 
     */
    void _searchLaneToStop();

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
    const PedestrianLocation* _location;
    PedestrianScene*          _scene;

    ///@}
};

#endif //__PEDESTRIAN_PERCEIVER_HPP__
#endif //INCLUDE_PEDESTRIANS
