/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file IntersectionBuilder_Vertex.cpp
 */
#include "IntersectionBuilder.hpp"
#include "RoadMapBuilder.hpp"
#include "SectionBuilder.hpp"
#include "../Intersection.hpp"
#include "../RelativeDirectionTable.hpp"
#include "../Section.hpp"
#ifdef INCLUDE_TRAMS
#include "../tram/BorderTram.hpp"
#include "../tram/IntersectionTramExt.hpp"
#endif //INCLUDE_TRAMS
#include <AmuLineSegment.hpp>
#include <AmuPoint.hpp>
#include <AmuVector.hpp>
#include <cassert>
#include <iostream>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//======================================================================
bool IntersectionBuilder::_generateVertexes()
{
    // 横断歩道・歩道の幅を考慮して交差点を拡幅する
    // Expand intersections considering width of crosswalk and sidewalk
    _findVertexesOfExpandedIntersection();

    // 交差点の頂点を補正する
    bool         isRevised;
    vector<bool> isRevisedCorner;
    isRevisedCorner.clear();
    for (int i = 0; i < _inter->numNexts(); i++)
    {
        isRevisedCorner.push_back(false);
    }
    isRevised = _reviseVertexes(isRevisedCorner);

    // 交差点の頂点の補正を_borderPointsに反映する
    if (isRevised)
    {
        _reviseBorderPoints(isRevisedCorner);
    }

    if (isRevised)
    {
        cerr << "WARNING: revise intersection internal structure." << "("
             << _inter->id() << ")" << endl;
    }

    return true;
}

//======================================================================
void IntersectionBuilder::_findVertexesOfExpandedIntersection()
{
    for (int i = 0; i < _inter->numNexts(); i++)
    {
        int connectDir = _inter->next(i)->direction(_inter);

        AmuLineSegment streetLine1(
            _roadwayVertexes[i * 2],
            _nextIBuilders[i]->roadwayVertex(connectDir * 2 + 1));
        AmuLineSegment streetLine2(
            _roadwayVertexes[i * 2 + 1],
            _nextIBuilders[i]->roadwayVertex(connectDir * 2));

        AmuPoint p1, p2;

        // 横断歩道の幅のぶんだけ拡張する
        // Expand by the width of the crosswalk
        double curCrosswalkWidth = _inter->crosswalkWidth(i);
        if (curCrosswalkWidth < 1e-6)
        {
            p1 = _roadwayVertexes[i * 2];
            p2 = _roadwayVertexes[i * 2 + 1];
        }
        else
        {
            p1 = streetLine1.createInteriorPoint(
                _inter->crosswalkWidth(i),
                streetLine1.length() - curCrosswalkWidth);
            p2 = streetLine2.createInteriorPoint(
                _inter->crosswalkWidth(i),
                streetLine2.length() - curCrosswalkWidth);
        }

        /*
         * 得られた点を境界生成用の点として一時保存
         *
         * Temporarily save obtained points as points for border
         * generation
         */
        _borderPoints.push_back(p1);
        _borderPoints.push_back(p2);

        // 歩道の幅のぶんだけ拡張する
        // Expand by the width of the sidewalk
        double sidewalkWidth1 = _nextSBuilders[i]->sidewalkWidth(_inter, false);
        double sidewalkWidth2 = _nextSBuilders[i]->sidewalkWidth(_inter, true);

        if (sidewalkWidth1 != 0.0 || sidewalkWidth2 != 0.0)
        {
            AmuLineSegment crosswalkLine(p1, p2);
            AmuVector      crosswalkVector = crosswalkLine.directionVector();
            crosswalkVector.normalize();
            p1 -= crosswalkVector * sidewalkWidth1;
            p2 += crosswalkVector * sidewalkWidth2;
        }

        // 得られた点を交差点の多角形頂点として登録
        // Save the resulting points as intersection polygon vertexes
        _inter->addVertex(p1);
        _inter->addVertex(p2);
    }
}

//======================================================================
bool IntersectionBuilder::_reviseVertexes(vector<bool>& result_isRevisedCorner)
{
    bool result = false;

    /*
     * 反時計回りで接続単路の次の頂点 outerIndex1 - 2 が対象の角
     *
     * The corners of interest are the next vertex outerIndex1 - 2 of
     * the counterclockwise connected sections
     */
    vector<AmuPoint>& vertexes
        = const_cast<vector<AmuPoint>&>(_inter->vertexes());

    for (int i = 0; i < _inter->numNexts(); i++)
    {
        int outerIndex0 = i * 2;
        int outerIndex1 = (i * 2 + 1) % vertexes.size();
        int outerIndex2 = (i * 2 + 2) % vertexes.size();
        int outerIndex3 = (i * 2 + 3) % vertexes.size();

        // 歩道がある場合の処理
        // Processing when there is sidewalk or crosswalk
        if (!(vertexes[outerIndex1] == _roadwayVertexes[outerIndex1])
            && !(vertexes[outerIndex2] == _roadwayVertexes[outerIndex2]))
        {
            AmuLineSegment outerLine01(
                vertexes[outerIndex0], vertexes[outerIndex1]);
            AmuLineSegment outerLine12(
                vertexes[outerIndex1], vertexes[outerIndex2]);
            AmuLineSegment outerLine23(
                vertexes[outerIndex2], vertexes[outerIndex3]);

            /*
             * 交点を頂点の中点に補正する
             * Revise intersection point to the midpoint of vertices
             */
            AmuPoint tmpPoint;
            AmuPoint revisedPoint;
            if (outerLine01.createIntersectionPointZ0(&outerLine23, &tmpPoint))
            {
                revisedPoint = outerLine12.createInteriorPoint(1.0, 1.0);
                result_isRevisedCorner[i] = true;
            }
            /*
             * 非凸角は外側の頂点で補正する
             * Non-convex vertex is revised at outer vertex
             */
            else if (!outerLine01.isLeftSide(vertexes[outerIndex2]))
            {
                revisedPoint              = vertexes[outerIndex2];
                result_isRevisedCorner[i] = true;
            }
            else if (!outerLine23.isLeftSide(vertexes[outerIndex1]))
            {
                revisedPoint              = vertexes[outerIndex1];
                result_isRevisedCorner[i] = true;
            }
            // 頂点の設定
            // Determine vertexes
            if (result_isRevisedCorner[i])
            {
                result                = true;
                vertexes[outerIndex1] = revisedPoint;
                vertexes[outerIndex2] = revisedPoint;
            }
        }
    }

    return result;
}

//======================================================================
bool IntersectionBuilder::_reviseBorderPoints(
    const vector<bool>& isRevisedCorner)
{
    int               numNext = _inter->numNexts();
    vector<AmuPoint>& vertexes
        = const_cast<vector<AmuPoint>&>(_inter->vertexes());
    int numVertexes = _inter->numVertexes();
    for (int i = 0; i < numNext; i++)
    {
        int n = (i + numNext - 1) % numNext;
        if (!isRevisedCorner[i] && !isRevisedCorner[n])
        {
            continue;
        }

        int            outerIndexM2 = (i * 2 + numVertexes - 2) % numVertexes;
        int            outerIndexM1 = (i * 2 + numVertexes - 1) % numVertexes;
        int            outerIndex0  = i * 2;
        int            outerIndex1  = (i * 2 + 1) % numVertexes;
        int            outerIndex2  = (i * 2 + 2) % numVertexes;
        int            outerIndex3  = (i * 2 + 3) % numVertexes;
        int            connectDir   = _inter->next(i)->direction(_inter);
        AmuLineSegment outerLine01(
            vertexes[outerIndex0], vertexes[outerIndex1]);
        AmuLineSegment streetLine0(
            _roadwayVertexes[outerIndex0],
            _nextIBuilders[i]->roadwayVertex(connectDir * 2 + 1));
        AmuLineSegment streetLine1(
            _roadwayVertexes[outerIndex1],
            _nextIBuilders[i]->roadwayVertex(connectDir * 2));
        /*
         * 車道へ入り込む頂点と車道境界の補正
         * - 車道と外部頂点による線の交点から車道境界点を作る
         * - 交点がない場合は車道に入り込んでいるとみなし
         *   別の外部頂点による線との交点から車道境界点を作る
         * - 車道境界点が作れない場合は外部頂点を車道境界点とする
         *  （ちょうど車道の線上）
         * - 車道には入り込んでいる場合と単路歩道がない場合は
         *   車道境界点に合わせて外部頂点を直す
         *
         * @todo English translation
         */
        if (!streetLine0.createIntersectionPointZ0(
                &outerLine01, &_borderPoints[outerIndex0]))
        {
            AmuLineSegment outerLineM2M1(
                vertexes[outerIndexM2], vertexes[outerIndexM1]);
            if (!streetLine0.createIntersectionPointZ0(
                    &outerLineM2M1, &_borderPoints[outerIndex0]))
            {
                _borderPoints[outerIndex0] = vertexes[outerIndex0];
            }
            vertexes[outerIndex0]  = _borderPoints[outerIndex0];
            vertexes[outerIndexM1] = _borderPoints[outerIndex0];
        }

        SectionBuilder* incSBuilder = _nextSBuilders[i];
        if (incSBuilder->sidewalkWidth(_inter, false) == 0.0)
        {
            vertexes[outerIndex0] = _borderPoints[outerIndex0];
        }
        if (!streetLine1.createIntersectionPointZ0(
                &outerLine01, &_borderPoints[outerIndex1]))
        {
            AmuLineSegment outerLine23(
                vertexes[outerIndex2], vertexes[outerIndex3]);
            if (!streetLine1.createIntersectionPointZ0(
                    &outerLine23, &_borderPoints[outerIndex1]))
            {
                _borderPoints[outerIndex1] = vertexes[outerIndex1];
            }
            vertexes[outerIndex1] = _borderPoints[outerIndex1];
            vertexes[outerIndex2] = _borderPoints[outerIndex1];
        }
        if (incSBuilder->sidewalkWidth(_inter, true) == 0.0)
        {
            vertexes[outerIndex1] = _borderPoints[outerIndex1];
        }
    }
    return true;
}
