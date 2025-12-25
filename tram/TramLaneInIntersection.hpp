/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file TramLaneInIntersection.hpp
 */
#ifdef INCLUDE_TRAMS
#ifndef __TRAM_LANE_IN_INTERSECTION_HPP__
#define __TRAM_LANE_IN_INTERSECTION_HPP__
#include "../LaneInIntersection.hpp"
#include <AmuLineSegment.hpp>
#include <string>

class LaneBundle;
class Connector;

//######################################################################
/**
 * @~japanese 交差点内路面電車レーン
 * @~english  Tram lane in intersection
 * @~ @ingroup TramSim
 */
class TramLaneInIntersection : public LaneInIntersection
{
public:
    TramLaneInIntersection(
        const std::string& id, const Connector* ptBegin,
        const Connector*               ptEnd,
        amu::geometry::AmuLineSegment* lineSegment, LaneBundle* parent);
    virtual ~TramLaneInIntersection();
};

#endif //__TRAM_LANE_IN_INTERSECTION_HPP__
#endif //INCLUDE_TRAMS
