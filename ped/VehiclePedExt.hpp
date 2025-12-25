/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehiclePedExt.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __VEHICLE_PED_EXT_HPP__
#define __VEHICLE_PED_EXT_HPP__
#include "../Vehicle.hpp"
#include <AmuPoint.hpp>
#include <AmuVector.hpp>

class Route;
class VehicleBehavior;
class Intersection;
class Pedestrian;
class Zebra;
struct VehicleBodyProperty;
struct VehicleLocation;
struct VehicleScene;

//##############################################################################
/**
 * @~japanese 自動車エージェントクラスの歩行者拡張
 * @~english  Pedestrian extension of car agent class
 * @~ @ingroup PedSim Vehicle
 */
class VehiclePedExt
{
public:
    VehiclePedExt()
    {
        _vehicle = nullptr;

        _requiresNotification = false;
        _laneToBeNotified     = nullptr;
        _timeToEnterLane      = 0;

        _behavior    = nullptr;
        _body        = nullptr;
        _globalRoute = nullptr;
        _location    = nullptr;
        _scene       = nullptr;
    };

    ~VehiclePedExt() {};

    /**
     * @~japanese 所有者とその部分クラスオブジェクトを登録する
     * @~english  Set owner and its part class objects
     */
    void setVehicle(
        Vehicle* vehicle, VehicleBehavior* behavior, VehicleBodyProperty* body,
        VehicleGlobalRoute* globalRoute, VehicleLocation* location,
        VehicleScene* scene);

    //==========================================================================
    /**
     * @~japanese @name 認知機能の拡張
     * @~english  @name Extension of recognition
     */
    ///@{
public:
    /**
     * @~japanese 前ステップでの認知結果をクリアする
     * @~english  Clear the recognition results from the previous step
     */
    void clearStatus()
    {
        _requiresNotification = false;
        _laneToBeNotified     = nullptr;
    }

    /**
     * @~japanese 歩行者を認知する
     * 
     * 厳密には，歩行者の接近するレーンを認知する
     *
     * @~english  Recognize pedestrian
     *
     * Specifically, perceive the lane in which pedestrians approach.
     */
    void searchPedestrian();

private:
    /**
     * @~japanese
     * 距離 @p distance だけ離れた レーン @p lane に進入するまでの
     * 時間を求める
     *
     * 求めた時間が閾値を下回る場合はレーンへの通知を準備する．通知
     * されたレーンには歩行者の接近を認めない．
     *
     * @~english
     * Find the time until entering @p lane, which is @p distance away
     *
     * If the calculated time is less than the threshold, prepare to
     * notify to the lane. Pedestrians are not allowed to approach the
     * notified lane.
     */
    void _calcTimeToEnterCrosswalkLane(const Lane* lane, double distance);

    ///@}

    //==========================================================================
    /**
     * @~japanese @name 行動機能の拡張
     * @~english  @name Extension of action
     */
    ///@{
public:
    /**
     * @~japanese レーンにこの車両の接近を通知する
     * @~english  Notify the lane of this vehicle's approach
     */
    void notifyCrosswalkLane();

    ///@}

private:
    /**
     * @~japanese 所有者
     * @~english  Owner
     */
    Vehicle* _vehicle;

    /**
     * @~japanese
     * レーンにこの車両の接近を通知する必要があるかどうか
     *
     * @~english
     * Whether lanes should be notified of this vehicle's approach
     */
    bool _requiresNotification;

    /**
     * @~japanese 通知する対象のレーン
     * @~english  Lane to be notified
     */
    Lane* _laneToBeNotified;

    /**
     * @~japanese 横断歩道レーンに進入するまでの時間
     * @~english  Time until entering the crosswalk lane
     */
    ulint _timeToEnterLane; //[ms]

    //==========================================================================
    /**
     * @~japanese
     * @name 所有者の部分クラスオブジェクトへのポインタ
     *
     * @~english
     * @name Pointers to part class objects of the owner
     */
    ///@{
private:
    VehicleBehavior*           _behavior;
    const VehicleBodyProperty* _body;
    const VehicleGlobalRoute*  _globalRoute;
    const VehicleLocation*     _location;
    VehicleScene*              _scene;

    ///@}

    //==========================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    bool requiresNotification() const
    {
        return _requiresNotification;
    }

    double timeToEnterLane() const
    {
        return _timeToEnterLane;
    }

    ///@}
};

#endif //__VEHICLE_PED_EXT_HPP__
#endif //INCLUDE_PEDESTRIANS
