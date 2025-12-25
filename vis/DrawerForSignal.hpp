/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file DrawerForSignal.hpp
 */
#ifndef __DRAWER_FOR_SIGNAL_HPP__
#define __DRAWER_FOR_SIGNAL_HPP__
#include "../Border.hpp"
#include "../Intersection.hpp"

//######################################################################
/**
 * @~japanese 信号機を描画する
 * @~english  Draw a traffic light
 * @~ @ingroup Drawing
 */
class DrawerForSignal
{
public:
    DrawerForSignal() {};
    ~DrawerForSignal() {};

    /**
     * @~japanese 交差点 @p inter に設置された信号機を描画する
     * @~english  Draw a traffic light at the intersection @p inter
     */
    void draw(const Intersection& inter) const;

private:
    /**
     * @~japanese 境界方向 @p dir のメイン灯火を描画する
     * @~english  Draw the main light with border direction @p dir
     */
    void _drawMainLight(const Intersection& inter, int dir) const;

    /**
     * @~japanese 境界方向 @p dir のサブ灯火を描画する
     * @~english  Draw the sub light with border direction @p dir
     */
    void _drawSubLight(const Intersection& inter, int dir) const;
};

#endif //__DRAWER_FOR_SIGNAL_HPP__
