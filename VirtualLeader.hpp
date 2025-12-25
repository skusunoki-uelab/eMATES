/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VirtualLeader.hpp
 */
#ifndef __VIRTUAL_LEADER_HPP__
#define __VIRTUAL_LEADER_HPP__
#include <iostream>
#include <string>
#include <vector>

//######################################################################
/**
 * @~japanese
 * 仮想先行エージェントの情報を格納するクラス
 *
 * エージェントのrecognize()関数の中で毎回更新される．先行車だけでなく
 * 信号などで停止すべき位置の情報も含む．自動車エージェントは
 * このクラスの情報をもとに加速度を決定する．
 * 
 * @~english
 * A class for storing information of the virtual preceding agent
 *
 * Updated every time in the agent's recognize() function. Include not
 * only the preceding car but also the information of the position
 * where the car should stop at a traffic light, etc. The car agent
 * determines the acceleration based on the information in this class.
 *
 * @~
 * @ingroup Vehicle
 */
class VirtualLeader
{
public:
    //==================================================================
    /**
     * @~japanese 仮想先行エージェントのタイプ
     *
     * @attention
     * 要素を変更する場合はvlTypeToStringの要素も同時に変更する．
     * VirtualLeader.cppで定義されている．順序も一致させること．
     *
     * @~english  Type of virtual preceding agent
     *
     * @attention
     * When changing the elements, change the elements of vlTypeToString
     * at the same time. It is defined in VirtualLeader.cpp.
     * Their order must be matched.
     */
    enum VLType
    {
        SPEED_LIMIT,
        TURNING_MIN_HEADWAY,
        TURNING_RIGHT,
        TURNING_LEFT,
        VEHICLE_FRONT,
        VEHICLE_TAIL,
        VEHICLE_CLINT,
        VEHICLE_CLINT2,
        VEHICLE_CLSEC,
        SIGNAL_RED,
        SIGNAL_REDBLINK_BEF,
        SIGNAL_REDBLINK_AFT,
        SIGNAL_YELLOWBLINK,
        LANE_INT,
        LC_ADJ_LEADER,
        LC_ADJ_SYNC,
        LC_SHORT_GAP,
        LC_SHORT_DIST,
        LC_INTERRUPT,
        LC_INTERRUPT_GAPGEN,
        PEDESTRIAN_ON_ZEBRA,
    };

    /**
     * @~japanese VLTypeを文字列に変換するときに用いる配列
     * @~english  Array used when converting VLType to string
     */
    static const std::vector<std::string> vlTypeToString;

    //==================================================================
public:
    VirtualLeader();

    VirtualLeader(double distance, double velocity, VLType type)
        : _annotation("")
    {
        _distance = distance;
        _velocity = velocity;
        _vlType   = type;
    }

    VirtualLeader(
        double distance, double velocity, VLType type,
        const std::string& annotation)
        : _annotation(annotation)
    {
        _distance = distance;
        _velocity = velocity;
        _vlType   = type;
    }

    ~VirtualLeader() {};

    /**
     * @~japanese 距離 @p distance と 速度 @p velocity を設定する
     * @~english  Set distance @p distance and velocity @p velocity
     */
    void setInformation(double distance, double velocity)
    {
        _distance = distance;
        _velocity = velocity;
    }

    /**
     * @~japanese VLTypeを文字列に変換して戻す
     * @~english  Return VLType converted to a string
     */
    const std::string vlTypeString() const
    {
        return vlTypeToString[_vlType];
    }

    /**
     * @~japanese
     * 仮想先行エージェントの情報を @p out に出力する
     *
     * @~english
     * Output information of virtual preceding agent to @p out
     */
    void print(std::ostream& out) const;

    //==================================================================
private:
    /**
     * @~japanese
     * この仮想先行車を認知したエージェントからの距離
     *
     * @~english
     *  Distance from the agent that recognized this virtual leader
     */
    double _distance;

    /**
     * @~japanese 速度 [m/ms]
     * @~english  Velocity [m/ms]
     */
    double _velocity;

    /**
     * @~japanese 仮想先行車の種類
     * @~english  Type of virtual leader
     */
    VLType _vlType;

    /**
     * @~japanese 注釈
     *
     * @note
     * おもに車両挙動のデバッグ用途で用いる
     *
     * @~english  Annotation
     *
     * @note
     * Mainly used for debugging vehicle behavior
     */
    std::string _annotation;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    double distance() const
    {
        return _distance;
    }

    double velocity() const
    {
        return _velocity;
    }

    VLType vlType() const
    {
        return _vlType;
    }

    std::string annotation() const
    {
        return _annotation;
    }

    ///@}
};

#endif //__VIRTUAL_LEADER_HPP__
