/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file Border.cpp
 */
#include "Border.hpp"
#include "AppMates.hpp"
#include "Config.hpp"
#include "Connector.hpp"
#include "CustomMessage.hpp"
#include "ObjectManager.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//==============================================================================
const Connector* Border::connector(int i) const
{
    if (i < static_cast<signed int>(_inPoints.size()))
    {
        assert(0 <= i);
        assert(i < static_cast<signed int>(_inPoints.size()));
        return _inPoints[i];
    }
    else if (i < static_cast<signed int>(_inPoints.size() + _outPoints.size()))
    {
        i -= _inPoints.size();
        assert(0 <= i);
        assert(i < static_cast<signed int>(_outPoints.size()));
        return _outPoints[i];
    }
    else
    {
        return nullptr;
    }
}

//==============================================================================
void Border::createConnectors(int numIn, int numOut)
{
    _inPoints.resize(numIn);
    _outPoints.resize(numOut);
    int sum = numIn + numOut;

    AmuVector vec = _lineSegment.directionVector();
    vec.normalize();
    AmuLineSegment line(
        _lineSegment.pointBegin() + vec * _roadsideWidth,
        _lineSegment.pointEnd() - vec * _roadsideWidth);

    /*
     * コネクタは線分を sum 個に等分する．
     *
     * i=0からi=numIn-1までのnumIn個が流入コネクタ
     * i=numInからi=numIn+numOut-1までのnumOut個が流出コネクタ
     *
     * 左側通行の場合，流入方向左から見て以下の順で並ぶ
     *   0(in), ..., numIn-1(in), numIn(out), ..., sum-1(out)
     * 右側通行の場合，流入方向左から見て以下の順で並ぶ
     *   numIn(out), ..., sum-1(out), 0(in), ..., numIn-1(in)
     *
     * -----------------------------------------------------------------
     * A connector divides a line segment into equal parts.
     *
     * [numIn] connectors from i=0 to i=numIn-1 are inflow connectors,
     * [numOut] connectors from i=numIn to i=numIn+numOut-1 are outflow
     * connectors.
     *
     * In the case of left-hand traffic, line up in the following order
     * when viewed from the left in the inflow direction;
     *   0(in), ..., numIn-1(in), numIn(out), ..., sum-1(out)
     * In the case of right-hand traffic, line up in the following order
     * when viewed from the left in the inflow direction
     *   numIn(out), ..., sum-1(out), 0(in), ..., numIn-1(in)
     */
#ifdef RIGHT_HAND_TRAFFIC
    for (int i = 0; i < numIn; i++)
    {
        _inPoints[i] = _createInnerConnector(
            line, (numOut + i) * 2 + 1, sum * 2 - ((numOut + i) * 2 + 1));
    }
    for (int i = 0; i < numOut; i++)
    {
        _outPoints[i]
            = _createInnerConnector(line, i * 2 + 1, sum * 2 - (i * 2 + 1));
    }
#else //RIGHT_HAND_TRAFFIC not defined
    for (int i = 0; i < numIn; i++)
    {
        _inPoints[i]
            = _createInnerConnector(line, i * 2 + 1, sum * 2 - (i * 2 + 1));
    }
    for (int i = 0; i < numOut; i++)
    {
        _outPoints[i] = _createInnerConnector(
            line, (numIn + i) * 2 + 1, sum * 2 - ((numIn + i) * 2 + 1));
    }
#endif
}

//==============================================================================
const Connector* Border::_createInnerConnector(
    const AmuLineSegment& line, double d0, double d1)
{
    const AmuPoint tmpPoint = line.createInteriorPoint(d0, d1);
    if (!tmpPoint.isValid())
    {
        amu::msg::error("Border::createInnerConnector() create bad Point.");
        exit(EXIT_FAILURE);
    }
    return AppMates::getObjectManager().createConnector(
        tmpPoint.x(), tmpPoint.y(), tmpPoint.z());
}
