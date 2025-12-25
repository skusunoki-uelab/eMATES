/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file ObjectInLane.hpp
 */
#ifndef __OBJECT_IN_LANE_HPP__
#define __OBJECT_IN_LANE_HPP__
#include <AmuVector.hpp>
#include <string>

/**
 * @defgroup Agent
 * @~japanese エージェント
 * @~english  Agent
 */

//######################################################################
/**
 * @~japanese 車線に配置されるエージェントの純粋抽象クラス
 * @~english  Pure abstract class for agents placed in lanes
 * @~ @ingroup Agent 
 */
class ObjectInLane
{
public:
    virtual ~ObjectInLane() {};

    /**
     * @~japanese 識別番号を戻す
     * @~english  Return ID number
     */
    virtual const std::string& id() const = 0;

    /**
     * @~japanese 長さ [m] を戻す
     * @~english  Return its length [m]
     */
    virtual double bodyLength() const = 0;

    /**
     * @~japanese 幅 [m] を戻す
     * @~english  Return its width [m]
     */
    virtual double bodyWidth() const = 0;

    /**
     * @~japanese 高さ [m] を戻す
     * @~english  Return its height [m]
     */
    virtual double bodyHeight() const = 0;

    /**
     * @~japanese 車線の始点からの距離 [m] を戻す
     * @~english  Return distance from the lane starting point [m]
     */
    virtual double distance() const = 0;

    /**
     * @~japanese 単路部の始点からの距離 [m] を戻す
     * @~english  Return distance from the section starting point [m]
     */
    virtual double distanceFromInflowBorder() const = 0;

    /**
     * @~japanese 方向ベクトルを戻す
     * @~english  Return direction vector
     */
    virtual const amu::math::AmuVector directionVector() const = 0;

    /**
     * @~japanese 速度 [m/ms] を戻す
     * @~english  Return velocity [m/ms]
     */
    virtual double velocity() const = 0;

    /**
     * @~japanese 加速度 [m/(ms^2)] を戻す
     * @~english  Return acceleration [m/(ms^2)]
     */
    virtual double accel() const = 0;
};

#endif //__AGENT_IN_LANE_HPP__
