/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file PedestrianScene.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __PEDESTRIAN_SCENE_HPP__
#define __PEDESTRIAN_SCENE_HPP__
#include <iostream>

class Lane;
class Pedestrian;
class Vehicle;

//######################################################################
/**
 * @~japanese
 * 歩行者エージェントの意思決定に必要な局所環境を集約する構造体
 *
 * PedestrianPerceiver によって設定され，PedestrianDeterminer が利用．
 *
 * @~english
 * Struct aggregating local environment necessary for decision-making
 * of the pedestrian agent
 *
 * Set by PedestrianPerceiver , and used by PedestrianDeterminer .
 *
 * @~
 * @ingroup Pedestrian
 */
struct PedestrianScene
{
public:
    PedestrianScene()
    {
        _pedestrian = nullptr;
        clear();
    }

    ~PedestrianScene() {};

    //==================================================================
    /**
     * @~japanese 保存した情報をクリアする
     * @~english  Clear stored information
     */
    void clear()
    {
        _nearestPedestrian                = nullptr;
        _laneToStop                       = nullptr;
        _nearestVehicle                   = nullptr;
        _distanceToNearestPedestrian      = 0.0;
        _frontDistanceToNearestPedestrian = 0.0;
        _sideDistanceToNearestPedestrian  = 0.0;
        _existsOncomingPedestrian         = false;
    }

    /**
     * @~japanese 距離情報を設定する
     * @~english  Set distance information
     */
    void setDistance(double distance, double frontDistance, double sideDistance)
    {
        _distanceToNearestPedestrian      = distance;
        _frontDistanceToNearestPedestrian = frontDistance;
        _sideDistanceToNearestPedestrian  = sideDistance;
    }

    /**
     * @~japanese 認知結果を @p out に出力する
     * @~english  Output the recognized result to @p out
     */
    void print(std::ostream& out) const;

    //==================================================================
private:
    /**
     * @~japanese 所有者
     * @~english  Owner
     */
    const Pedestrian* _pedestrian;

    /**
     * @~japanese 最近接歩行者
     * @~english  Nearest pedestrian
     */
    const Pedestrian* _nearestPedestrian;

    /**
     * @~japanese 最近接歩行者までの距離
     * @~english  Distance to the nearest pedestrian
     */
    double _distanceToNearestPedestrian;

    /**
     * @~japanese 最近接歩行者までの前方距離
     * @~english  Front distance to the nearest pedestrian
     */
    double _frontDistanceToNearestPedestrian;

    /**
     * @~japanese 最近接歩行者までの側方距離
     *
     * @note
     * 基準となる歩行者の歩行方向右側が正
     *
     * @~english  Side distance to the nearest pedestrian
     *
     * @note
     * The right side of the reference pedestrian's walking direction
     * is positive. 
     */
    double _sideDistanceToNearestPedestrian;

    /**
     * @~japanese
     * 視野内に対向歩行者がいるかどうか
     *
     * @~english
     * Whether there is an oncoming pedestrian within the visual field
     */
    bool _existsOncomingPedestrian;


    const Lane* _laneToStop;

    /**
     * @~japanese 最近接車両
     * @~english  Nearest car
     */
    const Vehicle* _nearestVehicle;

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

    const Pedestrian* nearestPedestrian() const
    {
        return _nearestPedestrian;
    }

    void setNearestPedestrian(const Pedestrian* ped)
    {
        _nearestPedestrian = ped;
    }

    double distanceToNearestPedestrian() const
    {
        return _distanceToNearestPedestrian;
    }

    double frontDistanceToNearestPedestrian() const
    {
        return _frontDistanceToNearestPedestrian;
    }

    double sideDistanceToNearestPedestrian() const
    {
        return _sideDistanceToNearestPedestrian;
    }

    bool existsOncomingPedestrian() const
    {
        return _existsOncomingPedestrian;
    }

    void setExistsOncomingPedestrian(bool flag)
    {
        _existsOncomingPedestrian = flag;
    }

    const Lane* laneToStop() const
    {
        return _laneToStop;
    }

    void setLaneToStop(const Lane* lane)
    {
        _laneToStop = lane;
    }

    const Vehicle* nearestVehicle() const
    {
        return _nearestVehicle;
    }

    void setNearestVehicle(const Vehicle* vehicle)
    {
        _nearestVehicle = vehicle;
    }

    ///@}
};

#endif //__PEDESTRIAN_SCENE_HPP__
#endif //INCLUDE_PEDESTRIANS
