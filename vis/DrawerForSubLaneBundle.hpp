/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file DrawerForSubLaneBundle.hpp
 */
#ifndef __DRAWER_FOR_SUB_LANE_BUNDLE_HPP__
#define __DRAWER_FOR_SUB_LANE_BUNDLE_HPP__
#include "../SubLaneBundle.hpp"

//######################################################################
/**
 * @~japanese サブセクションを描画する
 * @~english  Draw a subsection
 * @~ @ingroup Drawing
 */
class DrawerForSubLaneBundle
{
public:
    DrawerForSubLaneBundle() {};
    ~DrawerForSubLaneBundle() {};

    /**
     * @~japanese サブセクション @p subsec を描画する
     * @~english  Draw @p subsec
     */
    void draw(const SubLaneBundle& subsec) const;

private:
    /**
     * @~japanese サブセクション @p subsec の形状を描画する
     * @~english  Draw the shape of @p subsec
     */
    void _drawShape(const SubLaneBundle& subsec) const;

    /**
     * @~japanese サブセクション @p subsec のサブネットワークを描画する
     * @~english  Draw subnetwork of @p subsec
     */
    void _drawSubnetwork(
        const SubLaneBundle& subsec, double zMargin,
        double width) const;

    /**
     * @~japanese サブセクション @p subsec の信号を描画する
     * @~english  Draw the traffic signals of @p subsec
     */
    void _drawSignals(const SubLaneBundle& subsec) const;

    /**
     * @~japanese サブセクション @p subsec の識別番号を描画する
     * @~english  Draw the ID number of @p subsec
     */
    void _drawId(const SubLaneBundle& subsec) const;
};

#endif //__DRAWER_FOR_SUB_LANE_BUNDLE_HPP__
