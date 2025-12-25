/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file Zebra.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "Zebra.hpp"
#include "LanePedExt.hpp"
#include "Pedestrian.hpp"
#include "VehiclePedExt.hpp"
#include "../AppMates.hpp"
#include "../Vehicle.hpp"
#include "../TimeManager.hpp"
#include <AmuLineSegment.hpp>
#include <AmuPoint.hpp>
#include <AmuVector.hpp>
#include <algorithm>
#include <cassert>
#include <cfloat>
#include <cstdlib>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//==============================================================================
Zebra::Zebra(const std::string& id)
    : SubIntersection(id, SubsectionType::Crosswalk)
{
    _dirInIntersection = -1;
    _pedestrians.clear();
    _tmpPedestrians.clear();

#ifdef _OPENMP
    omp_init_lock(&_lock);
#endif //_OPENMP
}

//==============================================================================
Zebra::~Zebra()
{
#ifdef _OPENMP
    omp_destroy_lock(&_lock);
#endif //_OPENMP
}

//==============================================================================
void Zebra::initialize(int dirInIntersection)
{
    _dirInIntersection = dirInIntersection;
    _calcTransposeMatrix();

    /*
     * 始境界と終境界それぞれの中点を結ぶベクトルで希望歩行方向を定める
     *
     * Define the desired walking direction using the vector connecting
     * the midpoints of the start and end boundaries. 
     */
    AmuVector dv((vertex(1) + vertex(2)) * 0.5, (vertex(3) + vertex(0)) * 0.5);
    dv.normalize();

    // (_vertexes[1], _vertexes[2])->(_vertexes[3], _vertexes[0])
    _odEdges[0].setZebra(this);
    _odEdges[0].setDirection(dv);
    _odEdges[0].setLineSegment(edge(3));
    _odEdges[0].setCrossingDirection(0);

    // (_vertexes[3], _vertexes[0])->(_vertexes[1], _vertexes[2])
    _odEdges[1].setZebra(this);
    _odEdges[1].setDirection(-dv);
    _odEdges[1].setLineSegment(edge(1));
    _odEdges[0].setCrossingDirection(1);
}

//==============================================================================
void Zebra::_calcTransposeMatrix()
{
    AmuPoint  origin = vertex(2);
    AmuVector udir(origin, vertex(3));
    AmuVector vdir(origin, vertex(1));
    udir.normalize();
    vdir.normalize();
    _transMat.setEntries(udir.x(), vdir.x(), udir.y(), vdir.y());
    _transMat.inverse();
}

//==============================================================================
void Zebra::renewPedestrianOrder()
{
#ifdef PDS_DEBUG
    cerr << "renewPedestrianLine(): z[" << id() << "] has "
         << _tmpPedestrians.size() << " tmp_pedestrian(s)." << endl;
#endif

    _pedestrians.swap(_tmpPedestrians);

    /*
     * ソートは必要ないかもしれない
     * Sorting may not be necessary
     */
    /*
    sort(_pedestrians.begin(), _pedestrians.end(),
         [&](const Pedestrian* rl,
             const Pedestrian* rr)
         {
             double xl
                 = _transMat.getItem(0, 0) * rl->location()->x()
                 + _transMat.getItem(0, 1) * rl->location()->y();
             double xr
                 = _transMat.getItem(0, 0) * rr->location()->x()
                 + _transMat.getItem(0, 1) * rr->location()->y();
             return xl < xr;
         });
    */

    // 一時的コンテナをクリアする
    // Clear temporary container
    _tmpPedestrians.clear();

#ifdef PDS_DEBUG
    cerr << "renewPedestrianLine(): z[" << id() << "] has "
         << _pedestrians.size() << " pedestrian(s)." << endl;
    for (unsigned int i = 0; i < _pedestrians.size(); i++)
    {
        Pedestrian* ped = _pedestrians[i];
        cerr << "   [" << i << "] p[" << ped->id() << "]@"
             << ped->location()->position().toString() << "%"
             << ped->behavior()->velocity().toString()
             << ": onZebra=" << ped->location()->isOnZebra() << endl;
    }
#endif
}

//==============================================================================
void Zebra::renewApproachingVehicleOrder()
{
    for (auto itr : _lanes)
    {
        itr.second->pedExt()->renewApproachingVehicleOrder();
    }
}

//==============================================================================
void Zebra::notifyLaneOfApproachingPedestrian()
{
    // フラグのリセットとレーンの分類
    // Reset flags and classify lanes
    vector<Lane*> openLanes;
    vector<Lane*> closedLanes;
    for (auto itr_l : _lanes)
    {
        itr_l.second->pedExt()->setHasApproachingPedestrian(false);
        if (itr_l.second->pedExt()->hasApproachingVehicles())
        {
            closedLanes.emplace_back(itr_l.second);
        }
        else
        {
            openLanes.emplace_back(itr_l.second);
        }
    }

    static constexpr double duration = 5.0; //[s]
    for (auto itr_p : _pedestrians)
    {
        if (itr_p->behavior()->isAlmostStopped())
        {
            continue;
        }

        /*
         * duration [s] 以内に歩行者が到達するレーンのフラグ有効化する
         *   ただし自動車が進入予定のレーンおよびそれより前方のレーンを
         *   除く
         *
         * Enable flag for lanes which the pedestrian will reach within
         * duration [s]
         *   However, the lanes into which a car is scheduled to  enter
         *   and the lanes ahead of them are excluded.
         */
        AmuLineSegment trajectory(
            itr_p->position(),
            itr_p->position()
                + (itr_p->behavior()->velocity().size() * duration * 1000)
                    * itr_p->behavior()->desiredDirection());

        double   distanceToStop = DBL_MAX;
        AmuPoint point;
        for (auto itr_l : closedLanes)
        {
            itr_l->pedExt()->setHasApproachingPedestrian(true);
            if (trajectory.createIntersectionPoint(
                    itr_l->lineSegment(), &point))
            {
                distanceToStop
                    = min(distanceToStop, itr_p->position().distance(point));
            }
        }
        distanceToStop += 1.0e-6;
        for (auto itr_l : openLanes)
        {
            if (trajectory.createIntersectionPoint(
                    itr_l->lineSegment(), &point))
            {
                if (itr_p->position().distance(point) < distanceToStop)
                {
                    itr_l->pedExt()->setHasApproachingPedestrian(true);
                }
            }
        }
    }
}

//==============================================================================
bool Zebra::putPedestrian(Pedestrian* ped)
{
#ifdef _OPENMP
    omp_set_lock(&_lock);
#endif //_OPENMP

    if (find(_tmpPedestrians.begin(), _tmpPedestrians.end(), ped)
        != _tmpPedestrians.end())
    {
        cerr << "ERROR: Pedestrian[" << ped->id()
             << "] is already put at zebra[" << _id << "(@" << _parent->id()
             << ")]" << endl;
        exit(EXIT_FAILURE);
    }
    _tmpPedestrians.emplace_back(ped);

#ifdef _OPENMP
    omp_unset_lock(&_lock);
#endif //_OPENMP

#ifdef PDS_DEBUG
    cerr << "z[" << _id << "](" << _tmpPedestrians.size() << ") puts p["
         << ped->id() << "]$(" << ped << ")" << endl;
#endif

    return true;
}

#endif //INCLUDE_PEDESTRIANS
