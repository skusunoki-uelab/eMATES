/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file ODNodeBuilderTramExt.cpp
 */
#ifdef INCLUDE_TRAMS
#include "ODNodeBuilderTramExt.hpp"
#include "BorderTram.hpp"
#include "TramLaneInIntersection.hpp"
#include "TramRouteManager.hpp"
#include "../GVManager.hpp"
#include "../ObjectManager.hpp"
#include "../Section.hpp"
#include "../io/RoadMapBuilder.hpp"

using namespace std;
using namespace amu::converter;
using namespace amu::math;
using TL = TramLaneSide;

//======================================================================
bool ODNodeBuilderTramExt::generateTramLanes()
{
    BorderTram* border0
        = dynamic_cast<BorderTram*>(const_cast<Border*>(_inter->border(0)));
    BorderTram* border1
        = dynamic_cast<BorderTram*>(const_cast<Border*>(_inter->border(1)));
    if (!border0 || !border1)
    {
        return true;
    }

    IntersectionTramExt* tramExt = _inter->tramExt();
    if (border0->inPointTram() && border1->outPointTram())
    {
        int idIntBegin = border0->numIn() + border0->numOut();
        int idIntEnd   = 100 + border1->numIn() + border1->numOut()
            + tramExt->numOutTramLanes(0, TL::Left)
            + tramExt->numOutTramLanes(0, TL::Center)
            + tramExt->numOutTramLanes(0, TL::Right);
        int idInt = idIntBegin * 10000 + idIntEnd;

        _generateTramLane(
            idInt, border0->inPointTram(), border1->outPointTram());
    }
    if (border1->inPointTram() && border0->outPointTram())
    {
        int idIntBegin = 100 + border1->numIn() + border1->numOut();
        int idIntEnd   = border0->numIn() + border0->numOut()
            + tramExt->numInTramLanes(0, TL::Left)
            + tramExt->numInTramLanes(0, TL::Center)
            + tramExt->numInTramLanes(0, TL::Right);
        int idInt = idIntBegin * 10000 + idIntEnd;

        _generateTramLane(
            idInt, border1->inPointTram(), border0->outPointTram());
    }
    return true;
}

#endif //INCLUDE_TRAMS
