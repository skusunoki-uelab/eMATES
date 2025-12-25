/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file PedestrianBehavior.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __PEDESTRIAN_BEHAVIOR_HPP__
#define __PEDESTRIAN_BEHAVIOR_HPP__
#include <AmuVector.hpp>
#include <cassert>
#include <iostream>

class Pedestrian;

//######################################################################
/**
 * @~japanese
 * 歩行者エージェントの局所的な挙動に関する変数を集約する構造体
 *
 * おもに Pedestrian によって更新される．
 *
 * @~english
 * Struct aggregating variables related to the local behavior of the
 * pedestrian agent
 *
 * Mainly updated by PedestrianActor .
 *
 * @~
 * @ingroup Vehicle
 */
struct PedestrianBehavior
{
public:
    PedestrianBehavior() : _velocity(0, 0, 0), _desiredDirection(0, 0, 0)
    {
        _pedestrian        = nullptr;
        _crossingDirection = 0;
        _maxSpeed          = 1.3 / 1000.0;
        _accelFactor       = 1.3;
    };
    ~PedestrianBehavior() {};

    /**
     * @~japanese 速度ベクトルを正規化する
     * @~english  Normalize velocity vector
     */
    void normalizeVelocity()
    {
        _velocity.normalize();
    }

    /**
     * @~japanese ほとんど止まっているかどうかを戻す
     *
     * 速度が閾値以下になっているかどうかによって判定する．
     *
     * @~english  Return whether the pedestrian is almost stopped
     *
     * Judged by whether the speed is below the threshold
     */
    bool isAlmostStopped() const
    {
        return (_velocity.size() < 1e-6);
    }

    /**
     * @~japanese 行動に関する変数を @p out に出力する
     * @~english  Display behavior variables to @p out
     */
    void print(std::ostream& out) const;

    //==================================================================
private:
    /**
     * @~japanese 所有者
     * @~english  Owner
     */
    Pedestrian* _pedestrian;

    /**
     * @~japanese 横断方向 
     *
     * 0 or 1 で横断歩道をどの方向に歩行しようとするのか定める．
     * 辺 ( _vertexes[3] , _vertexes[0] ) から 辺 ( _vertexes[1] ,
     * _vertexes[2] ) へ歩行する方向を0 ， その逆を 1 とする．
     *
     * @~english  Crossing direction
     *
     * Use 0 or 1 to determine which direction the pedestrian wants to
     * walk on the crosswalk. Let 0 be the direction of walking from
     * edge ( _vertexes[3], _vertexes[0] ) to edge ( _vertexes[1],
     * _vertexes[2]) , and 1 be the opposite direction.  
     */
    int _crossingDirection;

    /**
     * @~japanese 速度ベクトル [m/ms]
     * @~english  Velocity vector
     */
    amu::math::AmuVector _velocity;

    /**
     * @~japanese 希望歩行方向
     * @~english  Desired walking direction
     */
    amu::math::AmuVector _desiredDirection;

    /**
     * @~japanese 通常時の最高歩行速さ [m/ms]
     * @~english  Maximum normal walking speed [m/ms]
     */
    double _maxSpeed;

    /**
     * @~japanese 加速係数
     * @~english  Acceleration factor
     */
    double _accelFactor;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    void setPedestrian(Pedestrian* ped);

    int crossingDirection() const
    {
        return _crossingDirection;
    }

    void setCrossingDirection(int direction)
    {
        assert(direction == 0 || direction == 1);
        _crossingDirection = direction;
    }

    const amu::math::AmuVector& velocity() const
    {
        return _velocity;
    }

    void setVelocity(const amu::math::AmuVector& velocity)
    {
        _velocity = velocity;
    }

    const amu::math::AmuVector& desiredDirection() const
    {
        return _desiredDirection;
    }

    void setDesiredDirection(const amu::math::AmuVector& direction)
    {
        _desiredDirection = direction;
    }

    double maxSpeed() const
    {
        return _maxSpeed;
    }

    double accelFactor() const
    {
        return _accelFactor;
    }

    ///@}
};

#endif //__PEDESTRIAN_BEHAVIOR_HPP__
#endif //INCLUDE_PEDESTRIANS
