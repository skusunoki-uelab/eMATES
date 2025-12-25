/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file TramLaneInSection.hpp
 */
#ifdef INCLUDE_TRAMS
#ifndef __TRAM_LANE_IN_SECTION_HPP__
#define __TRAM_LANE_IN_SECTION_HPP__
#include "../LaneInSection.hpp"
#include <AmuLineSegment.hpp>
#include <string>

class LaneBundle;
class Connector;

//######################################################################
/**
 * @~japanese 単路部内路面電車レーン
 * @~english  Tram lane in section
 * @~ @ingroup TramSim
 */
class TramLaneInSection : public LaneInSection
{
public:
    TramLaneInSection(
        const std::string& id, const Connector* ptBegin,
        const Connector*               ptEnd,
        amu::geometry::AmuLineSegment* lineSegment, LaneBundle* parent);
    virtual ~TramLaneInSection();
};

#endif //__TRAM_LANE_IN_SECTION_HPP__
#endif //INCLUDE_TRAMS
