/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleLaneChangeActor.hpp
 */
#ifndef __VEHICLE_LANE_CHANGE_ACTOR_HPP__
#define __VEHICLE_LANE_CHANGE_ACTOR_HPP__

class Vehicle;

struct VehicleBehavior;
struct VehicleBodyProperty;
struct VehicleDecision;
struct VehicleLocation;
struct VehicleScene;

//######################################################################
/**
 * @~japanese 自動車エージェントの車線変更に関する行動機能モジュール
 *
 * VehicleDecision をもとに行動し， VehicleBehavior , VehicleLocation を
 * 更新する．
 *
 * @~english  Action module for lane-change of car agent
 *
 * Act based on VehicleDecision and update VehicleBehavior and
 * VehicleLocation.
 *
 * @~ @ingroup Vehicle
 */
class VehicleLaneChangeActor
{
public:
    VehicleLaneChangeActor()
    {
        _vehicle        = nullptr;
        _body           = nullptr;
        _behavior       = nullptr;
        _decision       = nullptr;
        _location       = nullptr;
        _scene          = nullptr;
        _errorRemaining = 0.0;
    }
    ~VehicleLaneChangeActor() {};

    /**
     * @~japanese 所有者とその部分クラスオブジェクトを登録する
     * @~english  Set owner and its part class objects
     */
    void setVehicle(
        Vehicle* vehicle, VehicleBodyProperty* body, VehicleBehavior* behavior,
        VehicleDecision* decision, VehicleLocation* location,
        VehicleScene* scene);

public:
    /**
     * @~japanese 一連の車線変更動作を実行する
     * @~english  Perform a series of lane-change action
     */
    void act();

private:
    /**
     * @~japanese 車線変更を実行する
     * @~english  Perform lane-change
     */
    void _startLaneChange();

    /**
     * @~japanese error方向の変位を更新する
     * @~english  Update the displacement in the error direction
     */
    void _updateError();

    /**
     * @~japanese
     * error方向の変位の更新を終了する
     *
     * @~english
     * Finish updating the displacement in the error direction
     */
    void _finishLaneChange();

public:
    /**
     * @~japanese 車線変更を強制終了する
     * @~english  Force to finish lane-change
     */
    void abortLaneChange();

    //==================================================================
protected:
    /**
     * @~japanese 所有者
     * @~english  Owner
     */
    Vehicle* _vehicle;

    /*
     * @~japanese
     * 車線変更を終了するまでに残るerror方向の変位
     *
     * @~english
     * Displacement in error direction remaining before finishing
     * lane-change.
     */
    double _errorRemaining;

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
    const VehicleBodyProperty* _body;
    VehicleBehavior*           _behavior;
    VehicleDecision*           _decision;
    VehicleLocation*           _location;
    VehicleScene*              _scene;

    ///@}
};

#endif //__VEHICLE_LANE_CHANGE_ACTOR_HPP__
