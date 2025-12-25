/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file IntersectionBuilderTramExt.cpp
 */
#ifdef INCLUDE_TRAMS
#include "IntersectionBuilderTramExt.hpp"
#include "BorderTram.hpp"
#include "TramLaneInIntersection.hpp"
#include "TramRouteManager.hpp"
#include "../AppMates.hpp"
#include "../Config.hpp"
#include "../GVManager.hpp"
#include "../ObjectManager.hpp"
#include "../Section.hpp"
#include "../io/RoadMapBuilder.hpp"
#include "../io/SectionBuilder.hpp"

using namespace std;
using namespace amu::converter;
using namespace amu::geometry;
using namespace amu::math;
using TL = TramLaneSide;

//======================================================================
void IntersectionBuilderTramExt::generateConnectorsOnLine(
    AmuLineSegment line, int numIn, int numOut, int borderId)
{
    int sum = numIn + numOut + _inter->tramExt()->numTotalTramLanes(borderId);
    SectionBuilder* incSection
        = _roadMapBuilder->sectionBuilder(_inter->nextSection(borderId)->id());

    AmuVector vec = line.directionVector();
    vec.normalize();
    AmuLineSegment newLine(
        line.pointBegin() + vec * incSection->roadsideWidth(),
        line.pointEnd() - vec * incSection->roadsideWidth());

    int idInt = 0;

    // 自動車レーン用コネクタ
    // Connectors for car lanes
    _generateVehicleConnectorsOnLine(
        newLine, numIn, numOut, sum, borderId, idInt);

    // 路面電車レーン用コネクタ
    // Connectors for tram lanes
    _generateTramConnectorsOnLine(newLine, numIn, numOut, sum, borderId, idInt);
}

//======================================================================
void IntersectionBuilderTramExt::_generateVehicleConnectorsOnLine(
    AmuLineSegment& line, int numIn, int numOut, int sum, int borderId,
    int& idInt)
{
    ObjectManager&       obj     = AppMates::getObjectManager();
    IntersectionTramExt* tramExt = _inter->tramExt();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 流入コネクタ
    // Inflow connectors
    for (int i = 0; i < numIn; i++)
    {
        string id = formatId(
            to_string(borderId * 100 + idInt), NUM_FIGURE_FOR_CONNECTOR_LOCAL);
        idInt++;
#ifdef RIGHT_HAND_TRAFFIC
        int n = tramExt->numTotalTramLanes(borderId, TL::Left) + numOut
            + tramExt->numTotalTramLanes(borderId, TL::Center) + i;
#else
        int n = tramExt->numTotalTramLanes(borderId, TL::Left) + i;
#endif
        AmuPoint tmpPoint
            = line.createInteriorPoint(2 * n + 1, 2 * (sum - n) - 1);
        _inter->addInternalConnector(
            id, obj.createConnector(tmpPoint.x(), tmpPoint.y(), tmpPoint.z()));
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 流出コネクタ
    // Outflow connectors
    for (int i = 0; i < numOut; i++)
    {
        string id = formatId(
            to_string(borderId * 100 + idInt), NUM_FIGURE_FOR_CONNECTOR_LOCAL);
        idInt++;
#ifdef RIGHT_HAND_TRAFFIC
        int n = tramExt->numTotalTramLanes(borderId, TL::Left) + i;
#else
        int n = tramExt->numTotalTramLanes(borderId, TL::Left) + numIn
            + tramExt->numTotalTramLanes(borderId, TL::Center) + i;
#endif
        AmuPoint tmpPoint
            = line.createInteriorPoint(2 * n + 1, 2 * (sum - n) - 1);
        _inter->addInternalConnector(
            id, obj.createConnector(tmpPoint.x(), tmpPoint.y(), tmpPoint.z()));
    }
}

//======================================================================
void IntersectionBuilderTramExt::_generateTramConnectorsOnLine(
    AmuLineSegment& line, int numIn, int numOut, int sum, int borderId,
    int& idInt)
{
    ObjectManager&       obj     = AppMates::getObjectManager();
    IntersectionTramExt* tramExt = _inter->tramExt();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 流入コネクタ
    // Inflow connectors
    if (tramExt->numInTramLanes(borderId, TL::Left) > 0)
    {
        string id = formatId(
            to_string(borderId * 100 + idInt), NUM_FIGURE_FOR_CONNECTOR_LOCAL);
        idInt++;
#ifdef RIGHT_HAND_TRAFFIC
        int n = tramExt->numOutTramLanes(borderId, TL::Left);
#else
        int n = 0;
#endif
        AmuPoint tmpPoint
            = line.createInteriorPoint(2 * n + 1, 2 * (sum - n) - 1);
        tramExt->addInternalTramConnector(
            id, obj.createConnector(tmpPoint.x(), tmpPoint.y(), tmpPoint.z()));
    }
    //------------------------------------------------------------------
    else if (tramExt->numInTramLanes(borderId, TL::Center) > 0)
    {
        string id = formatId(
            to_string(borderId * 100 + idInt), NUM_FIGURE_FOR_CONNECTOR_LOCAL);
        idInt++;
#ifdef RIGHT_HAND_TRAFFIC
        int n = numOut + tramExt->numOutTramLanes(borderId, TL::Center);
#else
        int n = numIn;
#endif
        AmuPoint tmpPoint
            = line.createInteriorPoint(2 * n + 1, 2 * (sum - n) - 1);
        tramExt->addInternalTramConnector(
            id, obj.createConnector(tmpPoint.x(), tmpPoint.y(), tmpPoint.z()));
    }
    //------------------------------------------------------------------
    else if (tramExt->numInTramLanes(borderId, TL::Right) > 0)
    {
        string id = formatId(
            to_string(borderId * 100 + idInt), NUM_FIGURE_FOR_CONNECTOR_LOCAL);
        idInt++;
#ifdef RIGHT_HAND_TRAFFIC
        int n = numIn + numOut + tramExt->numOutTramLanes(borderId, TL::Right);
#else
        int n = numIn + numOut;
#endif
        AmuPoint tmpPoint
            = line.createInteriorPoint(2 * n + 1, 2 * (sum - n) - 1);
        tramExt->addInternalTramConnector(
            id, obj.createConnector(tmpPoint.x(), tmpPoint.y(), tmpPoint.z()));
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 流出コネクタ
    // Outflow connectors
    if (tramExt->numOutTramLanes(borderId, TL::Left) > 0)
    {
        string id = formatId(
            to_string(borderId * 100 + idInt), NUM_FIGURE_FOR_CONNECTOR_LOCAL);
        idInt++;
#ifdef RIGHT_HAND_TRAFFIC
        int n = 0;
#else
        int n = tramExt->numInTramLanes(borderId, TL::Left);
#endif
        AmuPoint tmpPoint
            = line.createInteriorPoint(2 * n + 1, 2 * (sum - n) - 1);
        tramExt->addInternalTramConnector(
            id, obj.createConnector(tmpPoint.x(), tmpPoint.y(), tmpPoint.z()));
    }
    //------------------------------------------------------------------
    else if (tramExt->numOutTramLanes(borderId, TL::Center) > 0)
    {
        string id = formatId(
            to_string(borderId * 100 + idInt), NUM_FIGURE_FOR_CONNECTOR_LOCAL);
        idInt++;
#ifdef RIGHT_HAND_TRAFFIC
        int n = numOut;
#else
        int n = numIn + tramExt->numInTramLanes(borderId, TL::Center);
#endif
        AmuPoint tmpPoint
            = line.createInteriorPoint(2 * n + 1, 2 * (sum - n) - 1);
        tramExt->addInternalTramConnector(
            id, obj.createConnector(tmpPoint.x(), tmpPoint.y(), tmpPoint.z()));
    }
    //------------------------------------------------------------------
    else if (tramExt->numOutTramLanes(borderId, TL::Right) > 0)
    {
        string id = formatId(
            to_string(borderId * 100 + idInt), NUM_FIGURE_FOR_CONNECTOR_LOCAL);
        idInt++;
#ifdef RIGHT_HAND_TRAFFIC
        int n = numIn + numOut;
#else
        int n = numIn + numOut + tramExt->numInTramLanes(borderId, TL::Right);
#endif
        AmuPoint tmpPoint
            = line.createInteriorPoint(2 * n + 1, 2 * (sum - n) - 1);
        tramExt->addInternalTramConnector(
            id, obj.createConnector(tmpPoint.x(), tmpPoint.y(), tmpPoint.z()));
    }
}

//======================================================================
bool IntersectionBuilderTramExt::generateTramLanes()
{
    for (int i = 0; i < _inter->numNexts(); i++)
    {
        BorderTram* borderFrom
            = dynamic_cast<BorderTram*>(const_cast<Border*>(_inter->border(i)));
        if (!borderFrom)
        {
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 中心サブセクション
        // Central subsection
        for (int j = 0; j < _inter->numNexts(); j++)
        {
            if (i == j || !(borderFrom->inPointTram()))
            {
                continue;
            }
            _generateDefaultCentralTramLanes(i, j);
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 横断歩道
        // Crosswalk subsection
        if (_inter->crosswalkWidth(i) != 0)
        {
            _generateCrosswalkTramLanes(i);
        }
    }

    return true;
}

//======================================================================
void IntersectionBuilderTramExt::_generateDefaultCentralTramLanes(
    int from, int to)
{
    BorderTram* borderTo
        = dynamic_cast<BorderTram*>(const_cast<Border*>(_inter->border(to)));
    if (borderTo == NULL || borderTo->outPointTram() == NULL)
    {
        return;
    }

    TramRouteManager&    tramRouteManager = AppMates::getTramRouteManager();
    IntersectionTramExt* tramExt          = _inter->tramExt();

    if (tramRouteManager.hasTramRoute(_inter, from, to))
    {
        int idIntBegin
            = from * 100 + _inter->numIn(from) + _inter->numOut(from);
        int idIntEnd = to * 100 + _inter->numIn(to) + _inter->numOut(to)
            + tramExt->numInTramLanes(to, TL::Left)
            + tramExt->numInTramLanes(to, TL::Center)
            + tramExt->numInTramLanes(to, TL::Right);
        int idInt = idIntBegin * 10000 + idIntEnd;

        _generateTramLane(
            idInt, tramExt->edgeTramConnector(idIntBegin),
            tramExt->edgeTramConnector(idIntEnd));
    }
}

//======================================================================
void IntersectionBuilderTramExt::_generateCrosswalkTramLanes(int dir)
{
    BorderTram* border
        = dynamic_cast<BorderTram*>(const_cast<Border*>(_inter->border(dir)));
    IntersectionTramExt* tramExt = _inter->tramExt();

    int idInt1 = _inter->numIn(dir) + _inter->numOut(dir);
    if (border->inPointTram())
    {
        int idIntBegin = 1 * 1000 + dir * 100 + idInt1;
        int idIntEnd   = dir * 100 + idInt1;
        int idInt      = idIntBegin * 10000 + idIntEnd;
        idInt1++;
        _generateTramLane(
            idInt, border->inPointTram(),
            tramExt->internalTramConnector(
                formatId(to_string(idIntEnd), NUM_FIGURE_FOR_CONNECTOR_LOCAL)));
    }
    if (border->outPointTram())
    {
        int idIntBegin = dir * 100 + idInt1;
        int idIntEnd   = 1 * 1000 + dir * 100 + idInt1;
        int idInt      = idIntBegin * 10000 + idIntEnd;
        _generateTramLane(
            idInt,
            tramExt->internalTramConnector(formatId(
                to_string(idIntBegin), NUM_FIGURE_FOR_CONNECTOR_LOCAL)),
            border->outPointTram());
    }
}

//======================================================================
void IntersectionBuilderTramExt::_generateTramLane(
    int idInt, const Connector* begin, const Connector* end)
{
    string id;
    id = formatId(to_string(idInt), NUM_FIGURE_FOR_LANE);

    AmuLineSegment* lineSegment
        = new AmuLineSegment(begin->point(), end->point());
    lineSegment->setProperty();
    TramLaneInIntersection* lane
        = new TramLaneInIntersection(id, begin, end, lineSegment, _inter);
    lane->setSpeedLimit(
        AppMates::getGVManager().getNumeric("TRAM_SPEED_LIMIT"));
    _inter->addLane(lane);
}

#endif //INCLUDE_TRAMS
