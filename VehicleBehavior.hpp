/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleBehavior.hpp
 */
#ifndef __VEHICLE_BEHAVIOR_HPP__
#define __VEHICLE_BEHAVIOR_HPP__
#include "Config.hpp"
#include <cstdint>
#include <deque>
#include <iostream>

class Lane;
class Vehicle;

//######################################################################
/**
 * @~japanese
 * 自動車エージェントの局所的な挙動に関する変数を集約する構造体
 *
 * おもに VehicleActor ， VehicleLaneChangeActor によって更新される．
 *
 * @~english
 * Struct aggregating variables related to the local behavior of the
 * car agent
 *
 * Mainly updated by VehicleActor and VehicleLaneChangeActor.
 *
 * @~
 * @ingroup Vehicle
 */
struct VehicleBehavior
{
public:
    VehicleBehavior();
    ~VehicleBehavior() {};

    /**
     * @~japanese 単路部内における平均速度を戻す
     *
     * 単路部内での走行距離を単路部内の滞在時間で除した値を計算する
     *
     * @~english  Return the average speed on section
     *
     * Calculate the distance traveled at the section divided by the
     * time spent within the section.
     */
    double aveVelocityInSection() const;

    /**
     * @~japanese 希望速度に対する走行速度の比 @p rate を保存する
     * @~english  Save @p rate of driving speed to desired speed
     */
    void addVelocityRateHistory(double rate);

    /**
     * @~japanese
     * 希望速度に対する走行速度の比の時間平均を返す
     *
     * @~english
     * Return temporal average of the rate of driving speed to desired
     * speed
     */
    double aveVelocityRate() const;

    /**
     * @~japanese 一時停止開始時刻をクリアする
     *
     * @note
     * 現在時刻との差が閾値を超えているかどうかによって一時停止を判定
     * することが多いので， 停止していない間は取りうる値の最大値を代入
     * しておく．
     * 
     * @~english  Clear pause start time
     *
     * @note
     * Since pausing is often determined based on whether the difference
     * from the current time exceeds a threshold, the maximum possible
     * value is assigned while the vehicle is moving.
     */
    void clearPauseStartTime()
    {
        _pauseStartTime = UINT32_MAX;
    }

    /**
     * @~japanese 行動に関する変数を @p out に出力する
     * @~english  Display behavior variables to @p out
     */
    void print(std::ostream& out) const;

    //====================================================================
private:
    /**
     * @~japanese 持ち主
     * @~english  Owner
     */
    const Vehicle* _vehicle;

    /**
     * @~japanese 速度 [m/ms]
     * @~english  Velocity [m/ms]
     */
    double _velocity;

    /**
     * @~japanese 速度比の履歴
     *
     * @note
     * VehicleScene::_vMax に対する _velocity の比率を保持する
     *
     * @~english  History of velocity rate
     *
     * @note
     * Store rate of _velocity to VehicleScene::_vMax . 
     */
    std::deque<double> _velocityRateHistory;

    /**
     * @~japanese error方向の速度 [m/ms]
     * @~english  Velocity in error direction [m/ms]
     */
    double _errorVelocity;

    /**
     * @~japanese 加速度 [m/(ms^2)]
     * @~english  Acceleration [m/(ms^2)]
     */
    double _accel;

    /**
     * @~japanese 変更先車線
     *
     * @note
     * 他者から認知される場合に利用される．車線変更開始前の
     * 自車の意思決定には VehicleScene::_desiredLaneTo を用いる
     *
     * @~english  Lane to change to
     *
     * @note
     * Used when recognized by others. For the subject car's decision
     * before lane-change starts, use VehicleScene::_desiredLaneTo .
     */
    const Lane* _laneTo;

    /**
     * @~japanese 変更先車線における位置
     *
     * @note
     * 他者から認知される場合に利用される．車線変更開始前の
     * 自車の意思決定には VehicleScene::_desiredDistanceTo を用いる
     * 
     * @~english  Position in the lane to change to
     *
     * @note
     * Used when recognized by others. For the subject car's decision
     * before lane-change starts, use VehicleScene::_desiredDistanceTo .
     */
    double _distanceTo;

    /**
     * @~japanese
     * 着目される行動（車線変更）をとっているかどうか
     *
     * @~english
     * Whether taking actions that attract attention (lane-change)
     */
    bool _isNotifying;

    /**
     * @~japanese 一時停止中であるか
     * @~english  Whether pausing
     */
    bool _isPausing;

    /**
     * @~japanese 停止回数
     * @~english  The number of pausing
     */
    int _numPausing;

    /**
     * @~japanese 一時停止開始時刻 [ms]
     *
     * @attention
     * 車両が一時停止しているか否かの判定に用いるが，_isPausingと併用
     * すること．
     *
     * @~english  Start time of pausing [ms]
     *
     * @attention
     * Used to determine whether the vehicle is pausing or not, but
     * should be used in conjunction with _isPausing.
     */
    ulint _pauseStartTime;

    /**
     * @~japanese 停止時間 [ms]
     *
     * 休止状態（sleep状態）かどうかによらず，交差点や単路部内における
     * 停止時間を記録する
     *
     * @~english  Pause time [ms]
     *
     * Record pause time at intersections and sections regardless of
     * whether inactive (sleep state) or not
     */
    int _pauseDuration;

    /**
     * @~japanese 休止時間 [ms]
     *
     * 休止状態の車両はすべての通行優先権を失う．逆に，他車は休止状態の
     * 車両を判断の材料としない．
     *
     * @~english  Inactive duration [ms]
     *
     * An inactive car loses all right of way. Conversely, cars do not
     * recognize inactive cars as the basis for decision-making. 
     */
    int _sleepDuration;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    void setVehicle(const Vehicle* vehicle)
    {
        _vehicle = vehicle;
    }

    double velocity() const
    {
        return _velocity;
    }

    double errorVelocity() const
    {
        return _errorVelocity;
    }

    void setErrorVelocity(double errorVelocity)
    {
        _errorVelocity = errorVelocity;
    }

    void setVelocity(double velocity)
    {
        _velocity = velocity;
    }

    double accel() const
    {
        return _accel;
    }

    void setAccel(double accel)
    {
        _accel = accel;
    }

    const Lane* laneTo() const
    {
        return _laneTo;
    }

    void setLaneTo(const Lane* lane)
    {
        _laneTo = lane;
    }

    double distanceTo() const
    {
        return _distanceTo;
    }

    void setDistanceTo(double distance)
    {
        _distanceTo = distance;
    }

    bool isNotifying() const
    {
        return _isNotifying;
    }

    void setIsNotifying(bool isNotifying)
    {
        _isNotifying = isNotifying;
    }

    bool isPausing() const
    {
        return _isPausing;
    }

    void setIsPausing(bool isPausing)
    {
        _isPausing = isPausing;
    }

    int numPausing() const
    {
        return _numPausing;
    }

    void incrementNumPausing()
    {
        _numPausing++;
    }

    void setNumPausing(int numPausing)
    {
        _numPausing = numPausing;
    }

    ulint pauseStartTime() const
    {
        return _pauseStartTime;
    }

    void setPauseStartTime(ulint time)
    {
        _pauseStartTime = time;
    }

    int pauseDuration() const
    {
        return _pauseDuration;
    }

    void setPauseDuration(int duration)
    {
        _pauseDuration = duration;
    }

    void addPauseDuration(int duration)
    {
        _pauseDuration += duration;
    }

    int sleepDuration() const
    {
        return _sleepDuration;
    }

    void setSleepDuration(int duration)
    {
        _sleepDuration = duration;
    }

    void addSleepDuration(int duration)
    {
        _sleepDuration += duration;
    }

    ///@}
};

#endif //__VEHICLE_BEHAVIOR_HPP__
