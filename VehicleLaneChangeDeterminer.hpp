/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleLaneChangeDeterminer.hpp
 */
#ifndef __VEHICLE_LANE_CHANGE_DETERMINER_HPP__
#define __VEHICLE_LANE_CHANGE_DETERMINER_HPP__

class Vehicle;

struct VehicleBehavior;
struct VehicleBodyProperty;
struct VehicleDecision;
struct VehicleLocation;
struct VehicleScene;

//######################################################################
/**
 * @~japanese 自動車エージェントの車線変更に関する意思決定機能モジュール
 *
 * VehicleScene をもとに判断し， VehicleDecision を更新する．
 *
 * @~english  Decision-making module for lane-change of car agent
 *
 * Make decision based on VehicleScene and update VehicleDecision.
 *
 * @~ @ingroup Vehicle
 */
class VehicleLaneChangeDeterminer
{
public:
    VehicleLaneChangeDeterminer()
    {
        _vehicle  = nullptr;
        _body     = nullptr;
        _behavior = nullptr;
        _decision = nullptr;
        _location = nullptr;
        _scene    = nullptr;
    };
    ~VehicleLaneChangeDeterminer() {};

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
     * @~japanese 車線変更の判断をおこなう
     * @~english  Make decision of lane-change
     */
    void determine();

private:
    /**
     * @~japanese gap acceptanceを判定する
     * @~english  Judge gap acceptance
     */
    void _judgeGapAcceptance();

public:
    /**
     * @~japanese 車間距離 @p gap を受け入れられるかどうか
     *
     * @note
     * 所有者以外のエージェントからも用される
     *
     * @~english  Whether the gap @p gap is acceptable
     *
     * @note
     * May be used by agents other than the subject car
     */
    bool isGapAcceptable(
        double gap, const Vehicle* leader, const Vehicle* follower) const;
protected:
    /**
     * @~japanese 車線変更中のerror方向の速度を更新する
     * @~english  Update speed in error direction during lane-change
     */
    void _renewErrorVelocity();

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
    const VehicleBodyProperty* _body;
    VehicleBehavior*           _behavior;
    VehicleDecision*           _decision;
    const VehicleLocation*     _location;
    VehicleScene*              _scene;

    ///@}
};

#endif //__VEHICLE_LANE_CHANGE_DETERMINER_HPP__
