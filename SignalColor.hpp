/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file SignalColor.hpp
 */
#ifndef __SIGNAL_COLOR_HPP__
#define __SIGNAL_COLOR_HPP__

//######################################################################
/**
 * @~japanese 信号の色を定義する
 * @note      名前空間でも問題ないが...
 *
 * @~english  Define traffic light colors
 * @note      Namespace are also fine, ...
 *
 * @~ @ingroup Signal
 */
struct SignalColor
{
public:
    /**
     * @~japanese メイン灯火色
     * @~english  Main light color
     */
    enum class MainState : int
    {
        BLUE        = 1,
        RED         = 2,
        YELLOW      = 3,
        REDBLINK    = 4,
        YELLOWBLINK = 5,
        DUMMY       = -1,
    };

    /**
     * @~japanese 矢印灯火
     * @~english  Arrow light
     */
    enum class SubState : int
    {
        NONE          = 0,
        ALL           = 123,
        STRAIGHT      = 1,
        LEFT          = 2,
        RIGHT         = 3,
        STRAIGHTLEFT  = 12,
        STRAIGHTRIGHT = 13,
        LEFTRIGHT     = 23,
        DUMMY         = -1,
    };

    /**
     * @~japanese 歩行者用信号
     * @~english  Pedestrian light
     */
    enum class WalkerState : int
    {
        NONE   = 0,
        BLUE   = 1,
        RED    = 2,
        YELLOW = 3,
        DUMMY  = -1,
    };
};

#endif // __SIGNAL_COLOR_HPP__
