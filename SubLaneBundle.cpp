/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file SubLaneBundle.cpp
 */
#include "SubLaneBundle.hpp"
#include "AppMates.hpp"
#include "GVManager.hpp"
#include "LaneBundle.hpp"
#include "Signal.hpp"
#ifdef INCLUDE_PEDESTRIANS
#include "ped/Zebra.hpp"
#include "ped/ZebraODEdge.hpp"
#endif //INCLUDE_PEDESTRIANS
#include <cassert>
#include <sstream>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//======================================================================
SubLaneBundle::SubLaneBundle(const string& id, SubsectionType type)
{
    _id     = id;
    _type   = type;
    _signal = nullptr;
    _lanes.clear();
    _vertexes.clear();
    _adjSubLaneBundles.clear();

    _strictJudgeInside = AppMates::getGVManager().getFlag(
        "ROAD_ENTITY_STRICT_JUDGE_INSIDE");
}

//======================================================================
SubLaneBundle::~SubLaneBundle() {}

//======================================================================
int SubLaneBundle::edgeNum(SubLaneBundle* subsec) const
{
    assert(subsec);
    for (unsigned int i = 0; i < _adjSubLaneBundles.size(); i++)
    {
        if (_adjSubLaneBundles[i] == subsec)
        {
            return static_cast<int>(i);
        }
    }
    cout << "ERROR: SubLaneBundle[" << _id << "(@" << _parent->id()
         << "}] doesn't connect SubLaneBundle[" << subsec->id() << "(@"
         << subsec->parent()->id() << ")" << endl;
    exit(EXIT_FAILURE);
    return -1;
}

//======================================================================
bool SubLaneBundle::isInside(const AmuPoint& point) const
{
    /*
     * 点がエッジ上にあるかどうか判定したあと，指定した点と各頂点との
     * 角度の総和を計算する．0 なら外，±2π なら中．z = 0 平面に
     * 投影して処理している．
     *
     * After checking whether a point lies on an edge, calculate the sum
     * of the angles between the specified point and each vertex.
     * 0 means outside, plus-minus 2pi means inside. It is processed by
     * projecting points onto the z = 0 plane.
     */
    /**
     * @todo 
     * これだと遅くなる．角度計算が遅そう．中点からの三角形に分割しても
     * 正確に分割できるか分からない．将来的に凹角を完全に排除した方が
     * いいかもしれない．
     */
    if (_strictJudgeInside)
    {
        for (unsigned int i = 0; i < _vertexes.size(); i++)
        {
            if (edge(i).z0().distance(point) < 1.0e-6)
            {
                return false;
            }
        }
        double angle = 0;
        for (unsigned int i = 0; i < _vertexes.size(); i++)
        {
            AmuVector v1(point.z0(), _vertexes[i].z0());
            AmuVector v2(
                point.z0(), _vertexes[(i + 1) % _vertexes.size()].z0());
            angle += v1.calcAngle(v2);
        }
        if (fabs(angle) < M_PI)
        {
            return false;
        }
    }
    else
    {
        bool ccw = edge(0).isLeftSide(point);
        for (unsigned int i = 0; i < _vertexes.size(); i++)
        {
            if (edge(i).length() > 1.0e-6
                && edge(i).isLeftSide(point) != ccw)
            {
                return false;
            }
        }
    }
    return true;
}

//======================================================================
bool SubLaneBundle::includes(const Lane* lane) const
{
    const AmuPoint& beginPoint = lane->lineSegment()->pointBegin();
    bool            includesBeginPoint = false;

    // 始点が辺上にあるかどうか調査
    // Check if the start point is on the edge
    for (unsigned int i = 0; i < _vertexes.size(); i++)
    {
        if (edge(i).distance(beginPoint) < 1.0e-6)
        {
            includesBeginPoint = true;
            break;
        }
    }

    // 始点が内部にあるかどうか調査
    // Check if the start point is inside
    if (!includesBeginPoint)
    {
        includesBeginPoint = true;
        bool isLeft        = edge(0).isLeftSide(beginPoint);
        for (unsigned int i = 1; i < _vertexes.size(); i++)
        {
            if (edge(i).isLeftSide(beginPoint) != isLeft)
            {
                includesBeginPoint = false;
                return false;
            }
        }
    }

    const AmuPoint& endPoint         = lane->lineSegment()->pointEnd();
    bool            includesEndPoint = false;

    // 終点が辺上にあるかどうか調査
    // Check if the end point is on the edge
    for (unsigned int i = 0; i < _vertexes.size(); i++)
    {
        if (edge(i).distance(endPoint) < 1.0e-6)
        {
            includesEndPoint = true;
            break;
        }
    }

    // 終点が内部にあるかどうか調査
    // Check if the end point is inside
    if (!includesEndPoint)
    {
        includesEndPoint = true;
        bool isLeft      = edge(0).isLeftSide(endPoint);
        for (unsigned int i = 1; i < _vertexes.size(); i++)
        {
            if (edge(i).isLeftSide(endPoint) != isLeft)
            {
                includesEndPoint = false;
                return false;
            }
        }
    }

    return includesBeginPoint && includesEndPoint;
}

//======================================================================
void SubLaneBundle::print(ostream& out) const
{
    stringstream ss;

    ss << "--- Subsection Information ---" << endl;
    ss << "ID: " << _id << ", Parent ID: " << _parent->id()
       << ", Type: ";
    switch (_type)
    {
    case SubsectionType::Roadway:
        ss << "Roadway";
        break;
    case SubsectionType::Sidewalk:
        ss << "Sidewalk";
        break;
    case SubsectionType::Crosswalk:
        ss << "Crosswalk";
        break;
    default:
        ss << "?";
    }
    ss << endl;
    ss << "Position: " << _center << endl;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ss << "Vertex:" << endl;
    int num = 0;
    for (auto itr : _vertexes)
    {
        ss << "  vertex[" << num << "]: " << itr << endl;
        num++;
    }
    ss << "Adjacent Subsections:" << endl;
    for (unsigned int i = 0; i < _vertexes.size(); i++)
    {
        ss << "  edge[" << i << "]: ";
        SubLaneBundle* subsec = adjSubLaneBundle(i);
        if (subsec)
        {
            ss << subsec->id() << "(@" << subsec->parent()->id() << ")";
        }
        else
        {
            ss << "none";
        }
        ss << endl;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ss << "Lanes:" << endl;
    if (_lanes.empty())
    {
        ss << "  none" << endl;
    }

    // 表示のためにソートする
    // Sort for display
    map<string, Lane*> sorted(_lanes.begin(), _lanes.end());

    for (auto itr : sorted)
    {
        ss << "  " << itr.second->id();
#ifdef INCLUDE_PEDESTRIANS
        if (itr.second->pedExt()->hasApproachingPedestrian())
        {
            ss << " [P]";
        }
        else
        {
            ss << " [ ]";
        }
#endif //INCLUDE_PEDESTRIANS
        ss << endl;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef INCLUDE_PEDESTRIANS
    Zebra* zebra
        = dynamic_cast<Zebra*>(const_cast<SubLaneBundle*>(this));
    if (zebra)
    {
        for (unsigned int i = 0; i < 2; i++)
        {
            const ZebraODEdge* begin = zebra->beginEdge(i);
            const ZebraODEdge* end   = zebra->endEdge(i);
            ss << "CrossingDirection[" << i << "]:" << endl;
            ss << "  begin edge: " << begin->lineSegment().pointBegin()
               << " - " << begin->lineSegment().pointEnd() << endl;
            ss << "  end edge  : " << end->lineSegment().pointBegin()
               << " - " << end->lineSegment().pointEnd() << endl;
            ss << "  desired direction: "
               << zebra->odEdge(i)->direction() << endl;
            ss << "  generation volume: " << begin->generationVolume()
               << endl;
        }
    }

#endif //INCLUDE_PEDESTRIANS

#pragma omp critical(out_critical)
    out << ss.str();
}
