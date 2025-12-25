/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file SectionBuilderTramExt.cpp
 */
#ifdef INCLUDE_TRAMS
#include "SectionBuilderTramExt.hpp"
#include "BorderTram.hpp"
#include "TramLaneInSection.hpp"
#include "../AppMates.hpp"
#include "../Border.hpp"
#include "../Connector.hpp"
#include "../GVManager.hpp"
#include "../Intersection.hpp"
#include <AmuConverter.hpp>

using namespace std;
using namespace amu::converter;
using namespace amu::geometry;

//======================================================================
bool SectionBuilderTramExt::generateTramLanes()
{
    for (int i = 0; i < 2; i++)
    {
        Intersection* from = _section->intersection(i);
        Intersection* to   = _section->intersection((i + 1) % 2);

        const Border* borderBegin = from->border(from->direction(to));
        const Border* borderEnd   = to->border(to->direction(from));

        BorderTram* borderTramBegin = dynamic_cast<BorderTram*>(
            const_cast<Border*>(borderBegin));
        BorderTram* borderTramEnd
            = dynamic_cast<BorderTram*>(const_cast<Border*>(borderEnd));

        if (!borderTramBegin || !borderTramEnd)
        {
            return false;
        }
        else if (
            !(borderTramBegin->outPointTram())
            || !(borderTramEnd->inPointTram()))
        {
            continue;
        }

        int idIntBegin
            = i * 100 + borderBegin->numIn() + borderBegin->numOut();
        int idIntEnd = ((i + 1) % 2) * 100 + borderEnd->numIn()
                       + borderEnd->numOut()
                       + ((dynamic_cast<BorderTram*>(
                               const_cast<Border*>(borderEnd)))
                                  ->inPointTram()
                              ? 1
                              : 0);
        int idInt = idIntBegin * 10000 + idIntEnd;
        _generateTramLane(
            idInt, borderTramBegin->outPointTram(),
            borderTramEnd->inPointTram());
    }

    return true;
}

//======================================================================
void SectionBuilderTramExt::_generateTramLane(
    int idInt, const Connector* pointBegin, const Connector* pointEnd)
{
    string id = formatId(to_string(idInt), NUM_FIGURE_FOR_LANE);

    AmuLineSegment* lineSegment
        = new AmuLineSegment(pointBegin->point(), pointEnd->point());
    lineSegment->setProperty();
    TramLaneInSection* lane = new TramLaneInSection(
        id, pointBegin, pointEnd, lineSegment, _section);
    lane->setSpeedLimit(
        AppMates::getGVManager().getNumeric("TRAM_SPEED_LIMIT"));
    _section->addLane(lane);
}

#endif //INCLUDE_TRAMS
