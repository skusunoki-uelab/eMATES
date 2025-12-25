/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file PedestrianPerceiver.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "Pedestrian.hpp"
#include "PedestrianLocation.hpp"
#include "VehiclePedExt.hpp"
#include "Zebra.hpp"
#include "../Lane.hpp"
#include "../LaneBundle.hpp"
#include "../Vehicle.hpp"
#include <cassert>
#include <cfloat>
#include <cmath>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//======================================================================
void PedestrianPerceiver::setPedestrian(
    Pedestrian* ped, const PedestrianLocation* location,
    PedestrianScene* scene)
{
    _pedestrian = ped;
    _location   = location;
    _scene      = scene;
}

//======================================================================
void PedestrianPerceiver::perceive()
{
    // 状態のリセット
    // Reset status
    _scene->clear();

    // 横断歩道上にいない
    // Not on the crosswalk
    if (!(_location->zebra()))
    {
        return;
    }

    // 最近傍歩行者を探す
    // Find a nearest pedestrian
    _searchNearestPedestrian();

    /*
     * 前方の自動車を探す
     *
     * Find a car ahead
     */
    _searchLaneToStop();
}

//======================================================================
void PedestrianPerceiver::_searchNearestPedestrian()
{
    assert(_location->zebra());

    Pedestrian* nearestPed  = nullptr;
    double      viewLength  = _pedestrian->body()->viewLength();
    double      minDistance = viewLength;
    double      viewAngle   = _pedestrian->body()->viewAngle() * 0.5;

    // この歩行者の歩行方向
    // Walking direction of this pedestrian
    AmuVector dir = _pedestrian->velocity();
    if (_pedestrian->behavior()->isAlmostStopped())
    {
        dir = _pedestrian->behavior()->desiredDirection();
    }
    dir.normalize();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 横断歩道上の他の歩行者について走査
    // Scan for other pedestrians on the crosswalk
    for (auto itr : _location->zebra()->pedestrians())
    {
        if (itr == _pedestrian || !(itr->location()->isOnZebra()))
        {
            continue;
        }

        // 相手の位置ベクトル
        // The position vector of the candidate
        AmuVector relPos(_location->position(), itr->position());

        // 後ろにいる相手は無視
        // Ignore the candidate behind this pedestrian
        if (dir.calcScalar(relPos) < 0)
        {
            continue;
        }

        // 相手が視野の中にいるかどうか
        // Whether the candidate is with in the visual field
        double distance
            = _location->position().distance(itr->position());
        double angle = relPos.calcAngle(dir);
        if (distance <= viewLength && angle < viewAngle)
        {
            // 対向歩行者であればフラグを有効化
            // If there is an oncoming pedestrian, activate flag.
            if (_pedestrian->behavior()->crossingDirection()
                != itr->behavior()->crossingDirection())
            {
                _scene->setExistsOncomingPedestrian(true);
            }
        }
        else
        {
            continue;
        }

        // 相手との距離が minDistance 未満かどうか
        // Whether the distance to candidate is less than minDistance
        if (distance < minDistance)
        {
            nearestPed  = itr;
            minDistance = distance;
        }
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef PDS_DEBUG
    if (nearestPed)
    {
        cerr << "p[" << _pedestrian->id() << "] found nearest p["
             << nearestPed->id() << "], distance = " << minDistance
             << endl;
    }
#endif

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 最近接歩行歩行者の情報を保存する
    // Save information of the nearest pedestrian
    if (nearestPed)
    {
        AmuVector relPos(_location->position(), nearestPed->position());
        double    frontDistance = dir.calcScalar(relPos);
        dir.revoltXY(-M_PI_2);
        double sideDistance = dir.calcScalar(relPos);
        _scene->setNearestPedestrian(nearestPed);
        _scene->setDistance(minDistance, frontDistance, sideDistance);
    }

    return;
}

//======================================================================
void PedestrianPerceiver::_searchLaneToStop()
{
    const Lane* result = nullptr;

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
    static constexpr double duration = 5.0; //[s]
    AmuLineSegment          trajectory(
        _location->position(),
        _location->position()
            + (_pedestrian->behavior()->velocity().size() * duration
               * 1000)
                  * _pedestrian->behavior()->desiredDirection());

    double   distanceToStop = DBL_MAX;
    AmuPoint point;
    for (auto itr : _location->zebra()->lanes())
    {
        if (!(itr.second->pedExt()->hasApproachingVehicles()))
        {
            continue;
        }
        if (trajectory.createIntersectionPoint(
                itr.second->lineSegment(), &point))
        {
            double distance = _location->position().distance(point);
            if (distance < distanceToStop)
            {
                distanceToStop = distance;
                result         = itr.second;
            }
        }
    }

    if (result)
    {
        _scene->setLaneToStop(result);
    }
}

#endif //INCLUDE_PEDESTRIANS
