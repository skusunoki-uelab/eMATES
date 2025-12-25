/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file BorderTram.cpp
 */
#ifdef INCLUDE_TRAMS
#include "BorderTram.hpp"
#include "IntersectionTramExt.hpp"
#include "../Connector.hpp"
#include "../Intersection.hpp"
#include <AmuVector.hpp>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;
using TL = TramLaneSide;

//======================================================================
const Connector* BorderTram::tramConnector(int idInt) const
{
    switch (idInt % 100)
    {
    case 0:
        if (_inPointTram)
        {
            return _inPointTram;
        }
    case 1:
        return _outPointTram;
    default:
        break;
    }

    return nullptr;
}

//======================================================================
void BorderTram::createConnectors(
    int numIn, int numOut, IntersectionTramExt* tramExt, int dir)
{
    _inPoints.resize(numIn);
    _outPoints.resize(numOut);

    AmuVector vec = _lineSegment.directionVector();
    vec.normalize();
    AmuLineSegment line(
        _lineSegment.pointBegin() + vec * _roadsideWidth,
        _lineSegment.pointEnd() - vec * _roadsideWidth);

    // 自動車レーン用コネクタ
    // Connectors for car lane
    _createVehicleConnectors(line, numIn, numOut, tramExt, dir, false);

    // 路面電車レーン用コネクタ
    // Connectors for tram lane
    _createTramConnectors(line, numIn, numOut, tramExt, dir, false);
}

//======================================================================
void BorderTram::createConnectorsReverse(
    int numIn, int numOut, IntersectionTramExt* tramExt, int dir)
{
    _inPoints.resize(numIn);
    _outPoints.resize(numOut);

    AmuVector vec = _lineSegment.directionVector();
    vec.normalize();
    AmuLineSegment line(
        _lineSegment.pointBegin() + vec * _roadsideWidth,
        _lineSegment.pointEnd() - vec * _roadsideWidth);

    // 自動車レーン用コネクタ
    // Connectors for car lane
    _createVehicleConnectors(line, numIn, numOut, tramExt, dir, true);

    // 路面電車レーン用コネクタ
    // Connectors for tram lane
    _createTramConnectors(line, numIn, numOut, tramExt, dir, true);
}

//======================================================================
void BorderTram::_createVehicleConnectors(
    AmuLineSegment& line, int numIn, int numOut,
    IntersectionTramExt* tramExt, int dir, bool isReversed)
{
    /* *****************************************************************

# LEFT_HAND_TRAFFIC

        (inter)          (section)
             _borders[dir]
                  |
      (end point side of the border) / (end if isReversed==true)
      ----------  +  -----------------------------------------------
                  |
                 --> _outPointTram (TL:Right, if it exists)
                 <-- _inPointTram  (TL:Right, if it exists)
                  |
                 --> _outPoints[numOut-1]
                 -->  ...
                 --> _outPoints[0]
                  |
                 --> _outPointTram (TL::Center, if it exists)
                 <-- _inPointTram  (TL::Center, if it exists)
                  |
                 <-- _inPoints[numIn-1]
                 <--  ...
                 <-- _inPoints[0]
                  |
                 --> _outPointTram (TL::Left, if it exists)
                 <-- _inPointTram  (TL::Left, if it exists)
                  |
      ----------  +  -----------------------------------------------
      (start point side of the border) / (end if isReversed==true)
                  |


# RIGHT_HAND_TRAFFIC

        (inter)          (section)
             _borders[dir]
                  |
      (end point side of the border) / (start if isReversed=~true)
      ----------  +  -----------------------------------------------
                  |
                 <-- _inPointTram  (TL::Right, if it exists)
                 --> _outPointTram (TL::Right, if it exists)
                  |
                 <-- _inPoints[numIn-1]
                 <--  ...
                 <-- _inPoints[0]
                  |
                 <-- _inPointTram  (TL::Center, if it exists)
                 --> _outPointTram (TL::Center, if it exists)
                  |
                 --> _outPoints[numOut-1]
                 -->  ...
                 --> _outPoints[0]
                  |
                 <-- _inPointTram  (TL::Left, if it exists)
                 --> _outPointTram (TL::Left, if it exists)
                  |
      ----------  +  -----------------------------------------------
      (start point side of the border) / (end if isReversed==true)
                  |

    ***************************************************************** */

    int numTramLeft   = tramExt->numTotalTramLanes(dir, TL::Left);
    int numTramCenter = tramExt->numTotalTramLanes(dir, TL::Center);
    int numTramRight  = tramExt->numTotalTramLanes(dir, TL::Right);
    int sum = numIn + numOut + tramExt->numTotalTramLanes(dir);
    int n;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 流入コネクタ
    // Inflow connectors
    if (!isReversed)
    {
#ifdef RIGHT_HAND_TRAFFIC
        n = numTramRight + numOut + numTramCenter;
#else
        n = numTramLeft;
#endif
    }
    else
    {
#ifdef RIGHT_HAND_TRAFFIC
        n = numTramLeft + numOut + numTramCenter;
#else
        n = numTramRight;
#endif
    }
    for (int i = 0; i < numIn; i++)
    {
        _inPoints[i] = _createInnerConnector(
            line, 2 * n + 1, 2 * sum - (2 * n + 1));
        n++;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 流出コネクタ
    // Outflow connectors
#ifdef RIGHT_HAND_TRAFFIC
    if (!isReversed)
    {
        n = numTramRight;
    }
    else
    {
        n = numTramLeft;
    }
#else
    if (!isReversed)
    {
        n = numTramLeft + numIn + numTramCenter;
    }
    else
    {
        n = numTramRight + numIn + numTramCenter;
    }
#endif
    for (int i = 0; i < numOut; i++)
    {
        _outPoints[i] = _createInnerConnector(
            line, 2 * n + 1, 2 * sum - (2 * n + 1));
        n++;
    }
}

//======================================================================
void BorderTram::_createTramConnectors(
    AmuLineSegment& line, int numIn, int numOut,
    IntersectionTramExt* tramExt, int dir, bool isReversed)
{
    int sum = numIn + numOut + tramExt->numTotalTramLanes(dir);
    int n;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 流入コネクタ
    // Inflow connectors
    if (tramExt->numInTramLanes(dir, TL::Left) > 0)
    {
        if (!isReversed)
        {
#ifdef RIGHT_HAND_TRAFFIC
            n = tramExt->numOutTramLanes(dir, TL::Left);
#else
            n = 0;
#endif
        }
        else
        {
#ifdef RIGHT_HAND_TRAFFIC
            n = numIn + numOut + tramExt->numInTramLanes(dir, TL::Left);
#else
            n = numIn + numOut;
#endif
        }
    }
    //------------------------------------------------------------------
    else if (tramExt->numInTramLanes(dir, TL::Center) > 0)
    {
        if (!isReversed)
        {
#ifdef RIGHT_HAND_TRAFFIC
            n = numOut + tramExt->numOutTramLanes(dir, TL::Center);
#else
            n = numIn;
#endif
        }
        else
        {
#ifdef RIGHT_HAND_TRAFFIC
            n = numOut + tramExt->numInTramLanes(dir, TL::Center);
#else
            n = numIn;
#endif
        }
    }
    //------------------------------------------------------------------
    else if (tramExt->numInTramLanes(dir, TL::Right) > 0)
    {
        if (!isReversed)
        {
#ifdef RIGHT_HAND_TRAFFIC
            n = numIn + numOut
                + tramExt->numOutTramLanes(dir, TL::Right);
#else
            n = numIn + numOut;
#endif
        }
        else
        {
#ifdef RIGHT_HAND_TRAFFIC
            n = tramExt->numInTramLanes(dir, TL::Right);
#else
            n = 0;
#endif
        }
    }
    _inPointTram
        = _createInnerConnector(line, 2 * n + 1, 2 * sum - (2 * n + 1));

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 流出コネクタ
    // Outflow connectors
    if (tramExt->numOutTramLanes(dir, TL::Left) > 0)
    {
        if (!isReversed)
        {
#ifdef RIGHT_HAND_TRAFFIC
            n = 0;
#else
            n = tramExt->numInTramLanes(dir, TL::Left);
#endif
        }
        else
        {
#ifdef RIGHT_HAND_TRAFFIC
            n = numIn + numOut;
#else
            n = numIn + numOut
                + tramExt->numOutTramLanes(dir, TL::Left);
#endif
        }
    }
    //------------------------------------------------------------------
    else if (tramExt->numOutTramLanes(dir, TL::Center) > 0)
    {
        if (!isReversed)
        {
#ifdef RIGHT_HAND_TRAFFIC
            n = numOut;
#else
            n = numIn + tramExt->numInTramLanes(dir, TL::Center);
#endif
        }
        else
        {
#ifdef RIGHT_HAND_TRAFFIC
            n = numOut;
#else
            n = numIn + tramExt->numOutTramLanes(dir, TL::Center);
#endif
        }
    }
    //------------------------------------------------------------------
    else if (tramExt->numOutTramLanes(dir, TL::Right) > 0)
    {
        if (!isReversed)
        {
#ifdef RIGHT_HAND_TRAFFIC
            n = numIn + numOut;
#else
            n = numIn + numOut
                + tramExt->numInTramLanes(dir, TL::Right);
#endif
        }
        else
        {
#ifdef RIGHT_HAND_TRAFFIC
            n = 0;
#else
            n = tramExt->numOutTramLanes(dir, TL::Right);
#endif
        }
    }
    _outPointTram
        = _createInnerConnector(line, 2 * n + 1, 2 * sum - (2 * n + 1));
}

#endif //INCLUDE_TRAMS
