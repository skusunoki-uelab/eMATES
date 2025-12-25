/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file DrawerForIntersection.hpp
 */
#ifndef __DRAWER_FOR_INTERSECTION_HPP__
#define __DRAWER_FOR_INTERSECTION_HPP__
#include "../Intersection.hpp"

//######################################################################
/**
 * @~japanese 交差点を描画する
 * @~english  Draw a intersection
 * @~ @ingroup Drawing
 */
class DrawerForIntersection
{
public:
    DrawerForIntersection() {};
    ~DrawerForIntersection() {};

    /**
     * @~japanese 交差点 @p inter を描画する
     * @~english  Draw @p inter
     */
    void draw(const Intersection& inter) const;

    /**
     * @~japanese 交差点 @p inter をサイズ @p size で簡易描画する
     * @~english  Simply draw @p inter with size @p size
     */
    void drawSimple(const Intersection& inter, double size) const;

private:
    /**
     * @~japanese サブセクションを描画する
     * @~english  Draw subsections
     */
    void _drawSubsections(const Intersection& inter) const;

    /**
     * @~japanese レーンを描画する
     * @~english  Draw lanes
     */
    void _drawLanes(const Intersection& inter) const;

    /**
     * @~japanese コネクタを描画する
     * @~english  Draw connectors
     */
    void _drawConnectors(const Intersection& inter) const;

    /**
     * @~japanese 信号機を描画する
     * @~english  Draw traffic light
     */
    void _drawSignals(const Intersection& inter) const;

    /**
     * @~japanese 識別番号を描画する
     * @~english  Draw ID number
     */
    void _drawId(const Intersection& inter) const;

    // --------------------------------------------------
    // CSの描画 [eMATES]

    /**
     * @~japanese CSの識別番号を描画する
     * @~english  Draw CS ID number
     */
    void _drawCSId(const Intersection& inter) const;

    /**
     * @~japanese CSの電力消費を描画する
     * @~english  Draw CS charge amount
     */
    void _drawCSCharge(const Intersection& inter) const;
};

#endif //__DRAWER_FOR_INTERSECTION_HPP__
