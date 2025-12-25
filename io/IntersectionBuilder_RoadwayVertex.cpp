/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file IntersectionBuilder_RoadwayVertex.cpp
 */
#include "IntersectionBuilder.hpp"
#include "RoadMapBuilder.hpp"
#include "SectionBuilder.hpp"
#include "../AppMates.hpp"
#include "../GVManager.hpp"
#include "../Intersection.hpp"
#include "../RelativeDirectionTable.hpp"
#include "../Section.hpp"
#ifdef INCLUDE_TRAMS
#include "../tram/BorderTram.hpp"
#include "../tram/IntersectionTramExt.hpp"
#endif //INCLUDE_TRAMS
#include <AmuLineSegment.hpp>
#include <AmuPoint.hpp>
#include <AmuStringOperator.hpp>
#include <cassert>
#include <iostream>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;
using namespace amu::string_operator;

//==============================================================================
bool IntersectionBuilder::_generateRoadwayVertexesFromFile(std::ifstream* fin)
{
    int  numVertexes               = 0;
    bool isVertexSpecificationLine = false;

    while ((*fin).good())
    {
        string         line;
        vector<string> tokens;
        if (!getTokens(fin, &line, &tokens, ','))
        {
            break;
        }

        if (tokens.size() == 1)
        {
            // "vertex"そのものは読み飛ばす
            // Skip "vertex" itself
            transform(
                tokens[0].begin(), tokens[0].end(), tokens[0].begin(),
                ::tolower);
            if (tokens[0] == "vertex")
            {
                isVertexSpecificationLine = true;
                continue;
            }
        }

        // 頂点座標指定行以外は読み飛ばす
        // Skip lines other than roadway vertexes specification line
        if (!isVertexSpecificationLine)
        {
            continue;
        }

        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 不正な行の処理
        // Invalid line handling
        if (tokens.size() != 3)
        {
            cerr << "ERROR: " << numVertexes << "th vertex in file("
                 << _inter->id() << ".txt) must specify "
                 << "x-y-z coordinates." << endl;
            exit(EXIT_FAILURE);
        }

        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling
        AmuPoint tmpVertex(
            _inter->center().x() + stof(tokens[0]),
            _inter->center().y() + stof(tokens[1]),
            _inter->center().z() + stof(tokens[2]));
        _roadwayVertexes.emplace_back(tmpVertex);
        numVertexes++;
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * n叉路(n>=2)はかならず2n角形になる.
     *   ODNode (n==1) はファイル入力しないので判定しない
     *
     * An n-way intersection must be 2n-gon.
     *   ODNode (n==1) is not checked because it ignores file input.
     */
    if (_roadwayVertexes.size()
        != static_cast<unsigned int>(_inter->numNexts()) * 2)
    {
        cerr << "ERROR: intersection[" << _inter->id() << " must have "
             << _inter->numNexts() * 2 << " vertexes, but "
             << _roadwayVertexes.size() << " vertexes are given in file("
             << _inter->id() << ".txt)." << endl;
        exit(EXIT_FAILURE);
    }

    return true;
}

//==============================================================================
bool IntersectionBuilder::_generateDefaultRoadwayVertexes()
{
    /*
     * 交差点はxy平面に平行で、頂点配列は左回りであることを前提とする
     * 接続数が4以下の場合，接続交差点にむかう2本のベクトルを用意して
     * 形状を規定する
     *
     * 実際には接続交差点の数だけ方向ベクトルを用意するが，
     * 基本となる2本を反転したもの
     * - 2: 1->0,0->1
     * - 3: 2->0,自分->1 (0->2がRD_STRAIGHTの場合)
     * - 4: 0->2,1->3
     */
#ifdef IB_DEBUG
    cout << "intersection[" << _inter->id() << "]" << endl;
#endif

    // 接続する単路の交点を求める
    // Find the intersection point of connecting section
    vector<AmuPoint> crossPoints(_inter->numNexts());
    _findCrossPointOfSectionBorders(crossPoints);

    // 多角形の頂点を決定する
    // Determine polygon vertexes
    _findRoadwayVertexes(crossPoints);

    return true;
}

//==============================================================================
void IntersectionBuilder::_findCrossPointOfSectionBorders(
    vector<AmuPoint>& result_points)
{
    unsigned int numNext = _inter->numNexts();
    AmuPoint     center  = _inter->center();
    for (unsigned int i = 0; i < numNext; i++)
    {
        Intersection* cur = _inter->next(i);
        AmuVector     curDV(center, cur->center());
        curDV.setZ(0);
        curDV.normalize();

        int           p    = (i + numNext - 1) % numNext; // means previous
        Intersection* prev = _inter->next(p);
        AmuVector     prevDV(center, prev->center());
        prevDV.setZ(0);
        prevDV.normalize();

        double theta = curDV.calcAngle(prevDV);

#ifdef IB_DEBUG
        cout << "    " << i << ": " << prevDV.x() << ", " << prevDV.y() << " - "
             << curDV.x() << ", " << curDV.y() << ": " << theta * 180 * M_1_PI
             << endl;
#endif

        if (sin(theta) > 1.0e-5)
        {
            _findCrossPointWithAngledSections(
                i, center, curDV, prevDV, theta, result_points[i]);
        }
        else
        {
            _findCrossPointWithFlatSections(i, center, result_points[i]);
        }
    }

#ifdef IB_DEBUG
    cout << "  CrossPoint:" << endl;
    for (unsigned int i = 0; i < result_points.size(); i++)
    {
        cout << "    " << i << ": " << result_points[i].x() << ","
             << result_points[i].y() << "," << result_points[i].z() << endl;
    }
#endif //IB_DEBUG
}

//==============================================================================
void IntersectionBuilder::_findCrossPointWithAngledSections(
    int i, const AmuPoint& center, const AmuVector& curDirectionVector,
    const AmuVector& prevDirectionVector, double theta, AmuPoint& result_point)
{
    unsigned int numNext = _inter->numNexts();
#ifdef INCLUDE_TRAMS
    IntersectionTramExt* tramExt = _inter->tramExt();
#endif //INCLUDE_TRAMS

    SectionBuilder* curSBuilder = _nextSBuilders[i];
    unsigned int    curNumConnectors
        = _inter->numLeftConnectors(i) + _inter->numRightConnectors(i);
#ifdef INCLUDE_TRAMS
    curNumConnectors += tramExt->numTotalTramLanes(i);
#endif //INCLUDE_TRAMS
    double halfWidthOfCurSection
        = curNumConnectors * 0.5 * curSBuilder->laneWidth()
        + curSBuilder->roadsideWidth();

    int             p            = (i + numNext - 1) % numNext; // means prev
    SectionBuilder* prevSBuilder = _nextSBuilders[p];
    unsigned int    prevNumConnectors
        = _inter->numLeftConnectors(p) + _inter->numRightConnectors(p);
#ifdef INCLUDE_TRAMS
    prevNumConnectors += tramExt->numTotalTramLanes(p);
#endif //INCLUDE_TRAMS
    double halfWidthOfPrevSection
        = prevNumConnectors * 0.5 * prevSBuilder->laneWidth()
        + prevSBuilder->roadsideWidth();

    result_point = center
        + (prevDirectionVector * halfWidthOfCurSection) / sin(theta)
        + (curDirectionVector * halfWidthOfPrevSection) / sin(theta);
}

//==============================================================================
void IntersectionBuilder::_findCrossPointWithFlatSections(
    int i, const AmuPoint& center, AmuPoint& result_point)
{
    /*
     * 2つの単路部のなす角が0[rad]またはPI[rad]の場合，交点は不定となる
     *   _next.size()が2でなければ，もう1つの交差する単路の方向ベクトル
     *   _insideDirectionVector(i+1) を利用する．_next.size()が2の場合
     *  には法線ベクトルで代用する
     *
     * If the angle between 2 sections is 0[rad] or PI[rad], the
     * intersection point cannot be defined.
     *   If _next.size() is not 2, use the direction vector of another
     *   intersecting section, _insideDirectionVector(i+1).
     *   If _next.size() is 2, substitute the normal vector for it.
     */
    unsigned int numNext = _inter->numNexts();
#ifdef INCLUDE_TRAMS
    IntersectionTramExt* tramExt = _inter->tramExt();
#endif //INCLUDE_TRAMS

    int n = (i + 1) % numNext;           // means next
    int p = (i + numNext - 1) % numNext; // means prev

    /// @todo 直接builderは求められないか（sectionにbuilderを持たせない）
    SectionBuilder* curSBuilder  = _nextSBuilders[i];
    SectionBuilder* nextSBuilder = _nextSBuilders[n];

    int curNumLeft   = _inter->numLeftConnectors(i);
    int nextNumLeft  = _inter->numLeftConnectors(n);
    int nextNumRight = _inter->numRightConnectors(n);
    int prevNumRight = _inter->numRightConnectors(p);
#ifdef INCLUDE_TRAMS
    curNumLeft += tramExt->numTotalTramLanes(i) / 2.0;
    nextNumLeft += tramExt->numTotalTramLanes(n) / 2.0;
    nextNumRight += tramExt->numTotalTramLanes(n) / 2.0;
    prevNumRight += tramExt->numTotalTramLanes(p) / 2.0;
#endif //INCLUDE_TRAMS

    if ((*_rdTable)(i, n) == RD::RIGHT)
    {
        // iが丁字路の左肩に相当
        // Index i corresponds the left shoulder of the T-junction.
        double ratio = 0.5;
        if (nextNumLeft + nextNumRight != 0)
        {
            ratio = (curNumLeft * nextNumRight + prevNumRight * nextNumLeft)
                / (nextNumLeft + nextNumRight);
        }
        result_point = center
            - (_internalUnitDirectionVector(n)
               * (nextSBuilder->laneWidth() * ratio
                  + nextSBuilder->roadsideWidth()));
    }
    else
    {
        AmuVector v = _internalUnitDirectionVector(i);
        v.revoltXY(M_PI_2);
        result_point = center
            - (v
               * (curSBuilder->laneWidth() * (curNumLeft + prevNumRight) * 0.5
                  + curSBuilder->roadsideWidth()));
    }
}

//==============================================================================
void IntersectionBuilder::_findRoadwayVertexes(
    const vector<AmuPoint>& crossPoints)
{
    for (int i = 0; i < _inter->numNexts(); i++)
    {
        // i番目の境界の方向ベクトル
        // Direction vector of i-th border
        AmuVector borderDirection;
        _findBorderDirection(i, borderDirection);

        // 境界を外側に拡張した線分
        // Line segments that expand the border outward
        double         margin = 2.0;
        AmuLineSegment borderLineLeft, borderLineRight;
        _findReferenceBorderLines(
            i, margin, crossPoints, borderDirection, borderLineLeft,
            borderLineRight);

        // 単路部の中心線
        // Center line of section
        AmuLineSegment centerLine;
        _findCenterLine(i, &centerLine);

        /*
         * 拡張された境界と単路部の中心線交点を求める
         *
         * Find intersection point of the expanded border and the 
         * center line of the section
         */
        AmuPoint midPoint;
        _findCrossPointOfBorderAndCenterLines(
            borderLineLeft, borderLineRight, centerLine, midPoint);

        /*
         * 拡張された境界と単路部の交点をもとに車道頂点を決定する
         *
         * Determine roadway vertexes based on the intersection point of
         * the expanded border and the center line of the section
         */
        _determineRoadwayVertexes(i, midPoint, borderDirection);
    }
}

//==============================================================================
void IntersectionBuilder::_findBorderDirection(int i, AmuVector& result_vector)
{
    int numNext = _inter->numNexts();
    int n       = (i + 1) % numNext;           // means next;
    int p       = (i + numNext - 1) % numNext; // menas prev;

    if (numNext <= 4 && (*_rdTable)(i, n) == RD::RIGHT
        && (*_rdTable)(i, p) == RD::LEFT)
    {
        result_vector = _internalUnitDirectionVector(n);
    }
    else
    {
        result_vector = _internalUnitDirectionVector(i);
        result_vector.revoltXY(M_PI_2);
    }
#ifdef IB_DEBUG
    cout << "  DirectionVector: " << result_vector.x() << ", "
         << result_vector.y() << endl;
#endif //IB_DEBUG
}

//==============================================================================
void IntersectionBuilder::_findReferenceBorderLines(
    int i, double margin, const vector<AmuPoint>& crossPoints,
    const AmuVector& borderDirection, AmuLineSegment& result_borderLineLeft,
    AmuLineSegment& result_borderLineRight)
{
    int n = (i + 1) % _inter->numNexts(); // means next;

    AmuPoint tmpVertexL, tmpVertexR;
    tmpVertexL = crossPoints[i] + _internalUnitDirectionVector(i) * margin;
    tmpVertexR = crossPoints[n] + _internalUnitDirectionVector(i) * margin;

    /*
     * 境界と中心線の交点を求めるための線分であるので長さは適当でよい
     *
     * It is a line segment to find the intersection point of the
     * boundary and the center line, so the length is not important.
     */
    result_borderLineLeft.setPoints(
        tmpVertexL, tmpVertexL + borderDirection * 100);
    result_borderLineRight.setPoints(
        tmpVertexR, tmpVertexR - borderDirection * 100);

#ifdef IB_DEBUG
    cout << "  ReferenceBorderLineLeft : "
         << result_borderLineLeft.pointBegin().x() << ", "
         << result_borderLineLeft.pointBegin().y() << ", "
         << result_borderLineLeft.pointBegin().z() << " - "
         << result_borderLineLeft.pointEnd().x() << ", "
         << result_borderLineLeft.pointEnd().y() << ", "
         << result_borderLineLeft.pointEnd().z() << endl;
    cout << "  ReferenceBorderLineRight: "
         << result_borderLineRight.pointBegin().x() << ", "
         << result_borderLineRight.pointBegin().y() << ", "
         << result_borderLineRight.pointBegin().z() << " - "
         << result_borderLineRight.pointEnd().x() << ", "
         << result_borderLineRight.pointEnd().y() << ", "
         << result_borderLineRight.pointEnd().z() << endl;
#endif //IB_DEBUG
}

//==============================================================================
void IntersectionBuilder::_findCenterLine(
    int i, AmuLineSegment* result_centerLine)
{
    AmuPoint nextCenter = _inter->next(i)->center();

    // 平面ベクトルにすることに注意
    // Note that it is a planer vector
    result_centerLine->setPoints(
        _inter->center(),
        AmuPoint(nextCenter.x(), nextCenter.y(), _inter->center().z()));
#ifdef IB_DEBUG
    cout << "  CenterLine: " << (*result_centerLine).pointBegin().x() << ", "
         << (*result_centerLine).pointBegin().y() << ", "
         << (*result_centerLine).pointBegin().z() << " - "
         << (*result_centerLine).pointEnd().x() << ", "
         << (*result_centerLine).pointEnd().y() << ", "
         << (*result_centerLine).pointEnd().z() << endl;
#endif //IB_DEBUG
}

//==============================================================================
void IntersectionBuilder::_findCrossPointOfBorderAndCenterLines(
    const AmuLineSegment& borderLineLeft, const AmuLineSegment& borderLineRight,
    const AmuLineSegment& centerLine, AmuPoint& result_point)
{
    AmuPoint midPointL, midPointR;
    AmuPoint center = _inter->center();


    /*
     * 境界線と中心線の交点
     *   交わらない場合中心点を代入し，次の処理で選択されないようにする
     *
     * Intersection points of  border lines and center line
     *   If they do not intersect, substitute the center point so that
     *   it will not be selected in the next process
     */
    if (!centerLine.createIntersectionPoint(&borderLineLeft, &midPointL))
    {
        midPointL = center;
    }
    if (!centerLine.createIntersectionPoint(&borderLineRight, &midPointR))
    {
        midPointR = center;
    }

    // 交差点の中心からより遠い点を採用
    // Take the point farther from the center of the intersection
    if (center.distance(midPointL) >= center.distance(midPointR))
    {
        result_point = midPointL;
    }
    else
    {
        result_point = midPointR;
    }

    // 交差点サイズの制限
    // Intersection size limit
    AmuVector mv(center, result_point);
    mv.setZ(0);
    double maxMidVec
        = AppMates::getGVManager().getNumeric("INTERSECTION_SIZE_LIMIT");
    if (mv.size() > maxMidVec)
    {
        mv.normalize();
        mv           = mv * maxMidVec;
        result_point = center + mv;
    }
#ifdef IB_DEBUG
    cout << "  MidPointLeft : " << midPointL.x() << ", " << midPointL.y()
         << ", " << midPointL.z() << endl;
    cout << "  MidPointRight: " << midPointR.x() << ", " << midPointR.y()
         << ", " << midPointR.z() << endl;
    cout << "  MidPoint     : " << result_point.x() << ", " << result_point.y()
         << ", " << result_point.z() << endl;
#endif //IB_DEBUG
}

//==============================================================================
void IntersectionBuilder::_determineRoadwayVertexes(
    int i, const AmuPoint& midPoint, const AmuVector& borderDirection)
{
    AmuVector dv(_inter->center(), _inter->next(i)->center());
    dv.setZ(0);
    dv.revoltXY(M_PI_2);
    double theta = dv.calcAngle(borderDirection);

    SectionBuilder* incSBuilder = _nextSBuilders[i];
    int             numConnectors
        = _inter->numLeftConnectors(i) + _inter->numRightConnectors(i);
#ifdef INCLUDE_TRAMS
    IntersectionTramExt* tramExt = _inter->tramExt();
    numConnectors += tramExt->numTotalTramLanes(i);
#endif //INCLUDE_TRAMS

    AmuPoint tmpVertexL = midPoint
        - borderDirection
            * (numConnectors * 0.5 * incSBuilder->laneWidth()
               + incSBuilder->roadsideWidth())
            / abs(cos(theta));
    AmuPoint tmpVertexR = midPoint
        + borderDirection
            * (numConnectors * 0.5 * incSBuilder->laneWidth()
               + incSBuilder->roadsideWidth())
            / abs(cos(theta));

#ifdef IB_DEBUG
    cout << "  Vertex:" << endl;
    cout << "    " << i << "th Left : " << tmpVertexL.x() << ", "
         << tmpVertexL.y() << ", " << tmpVertexL.z() << endl;
    cout << "    " << i << "th Right: " << tmpVertexR.x() << ", "
         << tmpVertexR.y() << ", " << tmpVertexR.z() << endl;
#endif
    _roadwayVertexes.push_back(tmpVertexL);
    _roadwayVertexes.push_back(tmpVertexR);
}

//==============================================================================
const AmuVector IntersectionBuilder::_internalUnitDirectionVector(int i)
{
    assert(0 <= i && i < _inter->numNexts());

    int      numNext = _inter->numNexts();
    AmuPoint center  = _inter->center();
    int      n1      = (i + 1) % numNext; // means next (i+1)
    int      n2      = (i + 2) % numNext; // means next (i+2)

    /*
     * 単路部の単位方向ベクトル（隣接交差点へ向かう方向が正）
     *
     * Unit direction vector of the section (direction towards adjacent
     * intersection is positive)
     */
    AmuVector tmpVBegin, tmpVEnd;

    AmuVector result;
    if (numNext == 2)
    {
        Intersection* next  = _inter->next(i);
        Intersection* next1 = _inter->next(n1);
        tmpVBegin.setPoints(center, next1->center());
        tmpVBegin.normalize();
        tmpVEnd.setPoints(center, next->center());
        tmpVEnd.normalize();
        result = tmpVEnd - tmpVBegin;
    }
    else if (numNext == 3)
    {
        if ((*_rdTable)(i, n1) == RD::STRAIGHT)
        {
            Intersection* next  = _inter->next(i);
            Intersection* next1 = _inter->next(n1);
            tmpVBegin.setPoints(center, next1->center());
            tmpVBegin.normalize();
            tmpVEnd.setPoints(center, next->center());
            tmpVEnd.normalize();
            result = tmpVEnd - tmpVBegin;
        }
        else if ((*_rdTable)(i, n2) == RD::STRAIGHT)
        {
            Intersection* next  = _inter->next(i);
            Intersection* next2 = _inter->next(n2);
            tmpVBegin.setPoints(center, next2->center());
            tmpVBegin.normalize();
            tmpVEnd.setPoints(center, next->center());
            tmpVEnd.normalize();
            result = tmpVEnd - tmpVBegin;
        }
        else
        {
            Intersection* next = _inter->next(i);
            result.setPoints(center, next->center());
        }
    }
    else if (numNext == 4)
    {
        Intersection* next  = _inter->next(i);
        Intersection* next2 = _inter->next(n2);
        tmpVBegin.setPoints(center, next2->center());
        tmpVBegin.normalize();
        tmpVEnd.setPoints(center, next->center());
        tmpVEnd.normalize();
        result = tmpVEnd - tmpVBegin;
    }
    else
    {
        Intersection* next = _inter->next(i);
        result.setPoints(center, next->center());
    }
    result.setZ(0);
    result.normalize();

    return result;
}
