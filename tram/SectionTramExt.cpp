/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file SectionTramExt.cpp
 */
#ifdef INCLUDE_TRAMS
#include "SectionTramExt.hpp"
#include "BorderTram.hpp"
#include "TramLaneInSection.hpp"
#include "../Intersection.hpp"
#include "../Section.hpp"
#include "../Border.hpp"

using namespace std;

//======================================================================
SectionTramExt::SectionTramExt(Section* section)
{
    _section = section;
}

//======================================================================
void SectionTramExt::getTramLanesFrom(
    vector<const Lane*>& result_lanes, const Intersection* inter)
{
    BorderTram* borderTram = dynamic_cast<BorderTram*>(
        const_cast<Border*>(inter->border(inter->direction(_section))));

    if (!borderTram)
    {
        return;
    }

    const Connector* connector = borderTram->outPointTram();
    if (!connector)
    {
        return;
    }

    vector<const Lane*> lanes = _section->lanesFromConnector(connector);
    result_lanes.insert(result_lanes.end(), lanes.begin(), lanes.end());
}

//======================================================================
void SectionTramExt::getTramLanesTo(
    vector<const Lane*>& result_lanes, const Intersection* inter)
{
    BorderTram* borderTram = dynamic_cast<BorderTram*>(
        const_cast<Border*>(inter->border(inter->direction(_section))));

    if (!borderTram)
    {
        return;
    }

    const Connector*    connector = borderTram->inPointTram();
    vector<const Lane*> lanes = _section->lanesToConnector(connector);
    result_lanes.insert(result_lanes.end(), lanes.begin(), lanes.end());
}

#endif //INCLUDE_TRAMS
