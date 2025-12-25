/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleLaneChangePerceiver.hpp
 */
#ifndef __VEHICLE_LANE_CHANGE_PERCEIVER_HPP__
#define __VEHICLE_LANE_CHANGE_PERCEIVER_HPP__

class Lane;
class LocalLaneRouter;
class Vehicle;

struct VehicleBehavior;
struct VehicleBodyProperty;
struct VehicleDecision;
struct VehicleLocalRoute;
struct VehicleLocation;
struct VehicleScene;

//######################################################################
/**
 * @~japanese 自動車エージェントの車線変更に関する認知機能モジュール
 *
 * VehicleScene を更新する．
 *
 * @~english  Recognition module for lane-change of car agent
 *
 * Update VehicleScene.
 *
 * @~ @ingroup Vehicle
 */
class VehicleLaneChangePerceiver
{
public:
    VehicleLaneChangePerceiver()
    {
        _vehicle     = nullptr;
        _behavior    = nullptr;
        _body        = nullptr;
        _decision    = nullptr;
        _localRoute  = nullptr;
        _location    = nullptr;
        _scene       = nullptr;
        _localRouter = nullptr;
    };
    ~VehicleLaneChangePerceiver() {};

    /**
     * @~japanese 所有者とその部分クラスオブジェクトを登録する
     * @~english  Set owner and its part class objects
     */
    void setVehicle(
        Vehicle* vehicle, VehicleBehavior* behavior, VehicleBodyProperty* body,
        VehicleDecision* decision, VehicleLocalRoute* localRoute,
        VehicleLocation* location, VehicleScene* scene,
        LocalLaneRouter* localRouter);

    //==================================================================
    /**
     * @~japanese @name 認知前処理
     * @~english  @name Pre-recognition processing
     */
    ///@{
public:
    /**
     * @~japanese 認知前の処理を行う
     * @~english  Perform pre-recognition processing
     */
    void preperceive();

    ///@}

    //==================================================================
    /**
     * @~japanese @name 認知
     * @~english  @name Recognition
     */
    ///@{
public:
    /**
     * @~japanese 車線変更のために周囲の状況を認識する
     * @~english  Recognize surroundings for lane-change
     */
    virtual void perceive();

protected:
    /**
     * @~japanese
     * 車線変更先の車線の先行車と後続車を探索する
     *
     * @~english
     * Search for preceding and following cars on the lane to change to
     */
    void _searchAdjLeaderAndFollower();

    /**
     * @~japanese
     * 自分の前方に割り込もうとする車両を探索する
     *
     * @attention
     * すべての車両の preperceive() が終わってから呼び出される必要がある
     *
     * @~english
     * Search for the car trying to cut interrupting into the front of
     * the subject car
     *
     * @attention
     * Must be called after preperceive() on all cars
     */
    void _searchInterruptingLeader();

    /**
     * @~japanese 車線変更用希望ヘッドウェイを更新する
     * @~english  Update desired headway for lane-change
     */
    void _renewDesiredHeadwayForLaneChange();

    /**
     * @~japanese 仮想先行車をセットする
     * @~english  Set virtual leader for lane-change
     */
    void _setVirtualLeadersForLaneChange();

    ///@}

    //==================================================================
protected:
    /**
     * @~japanese 所有者
     * @~english  Owner
     */
    Vehicle* _vehicle;

    //==================================================================
    /**
     * @~japanese
     * @name 所有者の部分クラスオブジェクトへのポインタ
     *
     * @~english
     * @name Pointers to the part class objects of the owner
     */
    ///@{
protected:
    VehicleBehavior*           _behavior;
    const VehicleBodyProperty* _body;
    VehicleDecision*           _decision;
    const VehicleLocalRoute*   _localRoute;
    const VehicleLocation*     _location;
    VehicleScene*              _scene;
    LocalLaneRouter*           _localRouter;

    ///@}
};

//######################################################################
/**
 * @~japanese
 * @name
 * LMRS (Lane-change Model with Relaxation and Synchronization) 用
 * パラメータ
 *
 * @~english
 * @name
 * Parameters for LMRS (Lane-change Model with Relaxation and
 * Synchronization)
 */
///@{

/**
 * @~japanese 車線変更の閾値となる推定距離 [m]
 * @~english  Estimated distance to threshold for lane change [m]
 */
constexpr double ANTICIPATION_DISTANCE = 300;

/**
 * @~japanese 車線変更の閾値となる推定時間 [s]
 * @~english  Estimated time to threshold for lane change [s]
 */
constexpr double ANTICIPATION_TIME = 67;

/**
 * @~japanese 速度インセンティブの正規化係数 [m/s]
 * @~english  Normalization factor for speed incentive [m/s]
 */
constexpr double VELOCITY_GAIN = 19.444444;

/**
 * @~japanese 希望ヘッドウェイの最大値 [s]
 * @~english  Maximum desired headway [s]
 */
constexpr double MAX_DESIRED_HEADWAY = 1.4;

/**
 * @~japanese 希望ヘッドウェイの最小値 [s]
 * @~english  Minimum desired headway [s]
 */
constexpr double MIN_DESIRED_HEADWAY = 0.7;

/**
 * @~japanese 希望ヘッドウェイの緩和時間 [s]
 * @~english  Relaxation time of desired headway [s]
 */
constexpr double DESIRED_HEADWAY_RELAXATION = 20;

/**
 * @~japanese 同期型車線変更の閾値
 * @~english  Desire threshold for synchronized lane change
 */
constexpr double DESIRE_THRESHOLD_SYNCHRONIZATION = 0.5;

/**
 * @~japanese 協調型車線変更の閾値
 * @~english Desire threshold for cooperative lane change
 */
constexpr double DESIRE_THRESHOLD_COOPERATION = 0.75;

///@}

#endif //__VEHICLE_LANE_CHANGE_PERCEIVER_HPP__
