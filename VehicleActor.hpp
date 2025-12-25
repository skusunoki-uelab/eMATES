/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleActor.hpp
 */
#ifndef __VEHICLE_ACTOR_HPP__
#define __VEHICLE_ACTOR_HPP__

class Blinker;
class Intersection;
class Lane;
class LocalLaneRouter;
class Section;
class Vehicle;

struct VehicleLocalRoute;
struct VehicleBehavior;
struct VehicleBodyProperty;
struct VehicleDecision;
struct VehicleGlobalRoute;
struct VehicleLocation;
struct VehicleScene;

//######################################################################
/**
 * @~japanese 自動車エージェントの行動機能モジュール
 *
 * VehicleDecision をもとに行動し， VehicleBehavior , VehicleLocation を
 * 更新する．
 *
 * @~english  Action module of car agent
 *
 * Act based on VehicleDecision and update VehicleBehavior and
 * VehicleLocation.
 *
 * @~ @ingroup Vehicle
 */
class VehicleActor
{
public:
    VehicleActor()
    {
        _vehicle              = nullptr;
        _requiresReroute      = true;
        _requiresLocalReroute = true;
        _movesToNextLane      = false;

        _behavior    = nullptr;
        _body        = nullptr;
        _decision    = nullptr;
        _globalRoute = nullptr;
        _localRoute  = nullptr;
        _location    = nullptr;
        _scene       = nullptr;
        _localRouter = nullptr;
        _blinker     = nullptr;
    };

    virtual ~VehicleActor() {};

    /**
     * @~japanese 所有者とその部分クラスオブジェクトを登録する
     * @~english  Set owner and its part class objects
     */
    void setVehicle(
        Vehicle* vehicle, VehicleBehavior* behavior, VehicleBodyProperty* body,
        VehicleDecision* decision, VehicleGlobalRoute* globalRoute,
        VehicleLocalRoute* localRoute, VehicleLocation* location,
        VehicleScene* scene, LocalLaneRouter* localRouter, Blinker* blinker);

    //==================================================================
    /**
     * @~japanese @name 行動
     * @~english  @name Action
     */
    ///@{
public:
    /**
     * @~japanese 行動する
     * @~english  Perform action
     */
    virtual void act();

protected:
    /**
     * @~japanese 速度を更新する
     * @~english  Update velocity
     */
    void _renewVelocity();

    /**
     * @~japanese 速度履歴を更新する
     * @~english  Update velocity history
     */
    void _renewVelocityHistory();

    /**
     * @~japanese 交差点内で次の車線に移る
     * @~english  Move to the next lane in intersection
     */
    void _runIntersection2Intersection();

    /**
     * @~japanese
     * 交差点の車線から単路部の車線に移る
     *
     * @~english
     * Move from the lane in intersection to the lane in section
     */
    void _runIntersection2Section();

    /**
     * @~japanese 単路部内で次の車線に移る
     * @~english  Move to the next lane in section
     */
    void _runSection2Section();

    /**
     * @~japanese
     * 単路部の車線から交差点の車線に移る
     *
     * @~english
     * Move from the lane in section to the lane in intersection
     */
    void _runSection2Intersection();

    /**
     * @~japanese 一旦停止する
     *
     * 速度が閾値を下回り車両が停止していると判定されたときの処理を
     * 定義する．
     *
     * @~english  Pause
     *
     * Define the process to be performed when the speed is below the
     * threshold and it is determined that the vehicle stops.
     */
    void _pause();

protected:
    /**
     * @~japanese 停止時間および停止回数をリセットする
     *
     * @note
     * _run*2*関数の中で用いる
     *
     * @~english  Reset pause time and the number of pausing
     *
     * @note
     * Used in _run*2* functions
     */
    void _resetPauseState();

public:
    /**
     * @~japanese
     * 現在所属中の単路部に車線変更が開始したことを通知する
     *
     * @~english
     * Notify the current section that lane-change started.
     */
    void notify();

    /**
     * @~japanese
     * 現在所属中の単路部に車線変更が終了したことを通知する
     *
     * @~english
     * Notify the current section that lane-change has been completed.
     */
    void unnotify();

    ///@}

    //==================================================================
    /**
     * @~japanese @name 行動後処理
     * @~english  @name Post-action processing
     */
    ///@{
public:
    /**
     * @~japanese 行動後の処理をおこなう
     * @~english  Perform post-action processing
     */
    virtual void postact();

private:
    /**
     * @~japanese 感知器に車両の通過を通知する
     * @~english  Notify traffic counters of the passing of the vehicle
     */
    void _notifyTrafficCounter();

    ///@}

    //==================================================================
protected:
    /**
     * @~japanese 所有者
     * @~english  Owner
     */
    Vehicle* _vehicle;

    /**
     * @~japanese 行動後に経路探索の必要があるか
     * @~english  Whether routing required after action
     */
    bool _requiresReroute;

    /**
     * @~japanese 行動後にローカル経路探索の必要があるか
     * @~english  Whether local routing required after action
     */
    bool _requiresLocalReroute;

    /**
     * @~japanese 下流の車線に移動したかどうか
     * @~english  Whether the vehicle moved to the downstream lane
     */
    bool _movesToNextLane;

    //==================================================================
    /**
     * @~japanese
     * @name 所有者の部分クラスオブジェクトへのポインタ
     *
     * @~english
     * @name Pointers to part class objects of the owner
     */
    ///@{
protected:
    VehicleBehavior*           _behavior;
    const VehicleBodyProperty* _body;
    VehicleDecision*           _decision;
    VehicleGlobalRoute*        _globalRoute;
    const VehicleLocalRoute*   _localRoute;
    VehicleLocation*           _location;
    VehicleScene*              _scene;
    LocalLaneRouter*           _localRouter;
    Blinker*                   _blinker;

    ///@}

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    void setRequiresReroute(bool requiresReroute)
    {
        _requiresReroute = requiresReroute;
    }

    void setRequiresLocalReroute(bool requiresLocalReroute)
    {
        _requiresLocalReroute = requiresLocalReroute;
    }

    ///@}
};

#endif //__VEHICLE_ACTOR_HPP__
