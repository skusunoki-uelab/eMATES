/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file IntersectionBuilder_Border.cpp
 */
#include "IntersectionBuilder.hpp"
#include "RoadMapBuilder.hpp"
#include "SectionBuilder.hpp"
#include "../AppMates.hpp"
#include "../Intersection.hpp"
#include "../ObjectManager.hpp"
#include "../RelativeDirectionTable.hpp"
#include "../Section.hpp"
#ifdef INCLUDE_TRAMS
#include "../tram/BorderTram.hpp"
#include "../tram/IntersectionTramExt.hpp"
#endif //INCLUDE_TRAMS
#include <cassert>
#include <iostream>
#include <AmuConverter.hpp>
#include <AmuLineSegment.hpp>
#include <AmuPoint.hpp>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;
using amu::converter::formatId;

//======================================================================
bool IntersectionBuilder::_generateBorders()
{
    // _border[i]は_borderPoints[2i]と_borderPoints[2i+1]を結ぶ
    // _border[i] connects _borderPoints[2i] and _borderPoints[2i+1]
    for (int i = 0; i < _inter->numNexts(); i++)
    {
        Border* border;
#ifdef INCLUDE_TRAMS
        IntersectionTramExt* tramExt = _inter->tramExt();
        if (tramExt->numTotalTramLanes(i) > 0)
        {
            BorderTram* borderTram = new BorderTram(
                _borderPoints[2 * i], _borderPoints[2 * i + 1],
                _nextSBuilders[i]->roadsideWidth());
            borderTram->createConnectors(
                _inter->numIn(i), _inter->numOut(i), tramExt, i);
            border = borderTram;
        }
        else
#endif //INCLUDE_TRAMS
        {
            border = new Border(
                _borderPoints[2 * i], _borderPoints[2 * i + 1],
                _nextSBuilders[i]->roadsideWidth());
            border->createConnectors(_inter->numIn(i), _inter->numOut(i));
        }
        assert(border);
        _inter->addBorder(border, i * 2);
    }
    return true;
}

//======================================================================
bool IntersectionBuilder::_generateInternalConnectors()
{
    // ODNodeには内部コネクタを生成しない
    // Not generate internal connectors for ODNodes
    if (_inter->numNexts() == 1)
    {
        return true;
    }

#ifdef INCLUDE_TRAMS
    IntersectionTramExt* tramExt = _inter->tramExt();
#endif //INCLUDE_TRAMS

    for (auto itr : _inter->subLaneBundles())
    {
        int idInt = stoi(itr.second->id());

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 中心サブセクションには内部コネクタを生成しない
        // Not generate internal connectors for central subsections
        if (idInt == 0)
        {
            // do nothing
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 周辺サブセクション
        // Peripheral subsections

        //--------------------------------------------------------------
        /*
         * 横断歩道の場合
         *  辺0（単路部に接するので_generateBordersでコネクタ作成済）と
         *  辺2を結ぶレーンを設置するための内部コネクタを生成する
         *
         * For crosswalks
         *   Generate internal connectors to set up lanes connecting
         *   side #0 (connected to section, so connectors was already
         *   generated with _generateBorders) and side #2.
         */
        if (itr.second->type() == SubsectionType::Crosswalk)
        {
            int            dir  = _inter->edge2dir(idInt - 10);
            AmuLineSegment line = itr.second->edge(2).reversal();
#ifdef INCLUDE_TRAMS
            if (tramExt->numTotalTramLanes(dir) > 0)
            {
                _builderTramExt->generateConnectorsOnLine(
                    line, _inter->numIn(dir), _inter->numOut(dir), dir);
            }
            else
            {
                _generateConnectorsOnLine(
                    line, _inter->numIn(dir), _inter->numOut(dir), dir);
            }
#else  //INCLUDE_TRAMS not defined
            _generateConnectorsOnLine(
                line, _inter->numIn(dir), _inter->numOut(dir), dir);
#endif //INCLUDE_TRAMS
        }
    }
    return true;
}

//======================================================================
bool IntersectionBuilder::_generateConnectorsOnLine(
    AmuLineSegment line, int numIn, int numOut, int borderId)
{
    int             sum         = numIn + numOut;
    SectionBuilder* incSBuilder = _nextSBuilders[borderId];
    assert(incSBuilder);

    AmuVector vec = line.directionVector();
    vec.normalize();
    AmuLineSegment newLine(
        line.pointBegin() + vec * incSBuilder->roadsideWidth(),
        line.pointEnd() - vec * incSBuilder->roadsideWidth());
    /*
     * 流入コネクタ数はnumIn(i=0からi=numIn-1まで)，
     * 流出コネクタ数はnumOut(i=numInからi=numIn+numOut-1まで)
     *
     * The number of inflow connectors is numIn (from i=0 to i=numIn-1),
     * and the number of outflow connectors is numOut (from i=numIn to
     * i=numIn+numOut-1).
     */
    /*
     * 左側通行の場合，流入方向から見て左から以下の順で並ぶ
     *   0(in), ..., numIn-1(in), numIn(out), ..., sum-1(out)
     * 右側通行の場合，流入方向から見て左から以下の順で並ぶ
     *   numIn(out), ..., sum-1(out), 0(in), ..., numIn-1(in)
     * ここで，sum=numIn+numOutである
     *
     * In the case of left-hand traffic, line up in the following order
     * from the left when viewed from the inflow direction.
     *   0(in), ..., numIn-1(in), numIn(out), ..., sum-1(out)
     * In the case of right-hand traffic, line up in the following order
     * from the left when viewed from the inflow direction.
     *   numIn(out), ..., sum-1(out), 0(in), ..., numIn-1(in)
     * where sum=numIn+numOut
     */
    for (int i = 0; i < sum; i++)
    {
        string id = formatId(
            to_string(borderId * 100 + i), NUM_FIGURE_FOR_CONNECTOR_LOCAL);
#ifdef RIGHT_HAND_TRAFFIC
        AmuPoint tmpPoint;
        if (i < numIn)
        {
            // 流入コネクタ
            // Inflow connector
            tmpPoint = newLine.createInteriorPoint(
                (numOut + i) * 2 + 1, sum * 2 - ((numOut + i) * 2 + 1));
        }
        else
        {
            // 流出コネクタ
            // Outflow connector
            tmpPoint = newLine.createInteriorPoint(
                (i - numIn) * 2 + 1, sum * 2 - ((i - numIn) * 2 + 1));
        }
#else  // RIGHT_HAND_TRAFFIC is not defined
        AmuPoint tmpPoint
            = newLine.createInteriorPoint(i * 2 + 1, sum * 2 - (i * 2 + 1));
#endif //RIGHT_HAND_TRAFFIC
        _inter->addInternalConnector(
            id,
            AppMates::getObjectManager().createConnector(
                tmpPoint.x(), tmpPoint.y(), tmpPoint.z()));
    }
    return true;
}
