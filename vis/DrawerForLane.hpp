/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file DrawerForLane.hpp
 */
#ifndef __DRAWER_FOR_LANE_HPP__
#define __DRAWER_FOR_LANE_HPP__
#include "../Lane.hpp"

//######################################################################
/**
 * @~japanese レーンを描画する
 * @~english  Draw a lane
 * @~ @ingroup Drawing
 */
class DrawerForLane
{
public:
    DrawerForLane() {};
    ~DrawerForLane() {};

    /**
     * @~japanese レーン @p lane を描画する
     * @~english  Draw @p lane
     */
    void draw(const Lane& lane, double zMargin, double width) const;

private:
    /**
     * @~japanese レーンを構成する線分 @p line を描画する
     * @~english  Draw the line segment @p line that make up the lane
     */
    void _drawLine(
        const amu::geometry::AmuLineSegment& line, double height,
        double zMargin, bool isArrow) const;
};

#endif //__DRAWER_FOR_LANE_HPP__
