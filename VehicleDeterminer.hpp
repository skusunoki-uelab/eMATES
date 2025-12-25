/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleDeterminer.hpp
 */
#ifndef __VEHICLE_DETERMINER_HPP__
#define __VEHICLE_DETERMINER_HPP__

class Vehicle;

struct VehicleBodyProperty;
struct VehicleBehavior;
struct VehicleDecision;
struct VehicleLocation;
struct VehicleScene;

//######################################################################
/**
 * @~japanese 自動車エージェントの意思決定機能モジュール
 *
 * VehicleScene をもとに判断し， VehicleDecision を更新する．
 *
 * @~english  Decision-making module of car agent
 *
 * Make decision based on VehicleScene and update VehicleDecision.
 *
 * @~ @ingroup Vehicle
 */
class VehicleDeterminer
{
public:
    VehicleDeterminer()
    {
        _vehicle  = nullptr;
        _body     = nullptr;
        _behavior = nullptr;
        _decision = nullptr;
        _location = nullptr;
        _scene    = nullptr;
    };
    virtual ~VehicleDeterminer() {};

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
     * @~japanese 意思決定する
     * @~english  Make decision
     */
    virtual void determine();

protected:
    /**
     * @~japanese VirtualLeader をもとに加速度を決定する
     * @~english  Determine acceleration based on VirtualLeader
     */
    void _determineAcceleration();

public:
    /**
     * @~japanese
     * Intelligent Driver Model+によって加速度を算出する
     *
     * @note
     * 同時に安全距離を求める
     *
     * @~english
     * Calculate acceleration by the Intelligent Driver Model+
     *
     * @note
     * Find safe distance simultaneously.
     */
    double calcAcceleration(
        double distance, double relVelocity, double desiredHeadway) const;

    /**
     * @~japanese
     * Intelligent Driver Model+によって加速度を算出する
     *
     * @note
     * 別に求めた安全距離 @p safetyDistance を用いる
     *
     * @~english
     * Calculate acceleration by the Intelligent Driver Model+
     *
     * @note
     * Use safety distance @p safetyDistance obtained separately.
     */
    double calcAcceleration(double distance, double safetyDistance) const;

    /**
     * @~japanese
     * Intelligent Driver Model+で用いられる安全距離を算出する
     *
     * @~english
     * Calculate safety distance used in Intelligent Driver Model+
     */
    double calcSafetyDistance(
        double velocity, double relVelocity, double desiredHeadway) const;

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
     * @name Pointers to part class objects of the owner
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

#endif //__VEHICLE_DETERMINER_HPP__
