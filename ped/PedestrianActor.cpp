/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file PedestrianActor.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "PedestrianActor.hpp"
#include "PedestrianBehavior.hpp"
#include "PedestrianDecision.hpp"
#include "PedestrianLocation.hpp"
#include "Zebra.hpp"
#include "../AppMates.hpp"
#include "../TimeManager.hpp"
#include <AmuPoint.hpp>
#include <AmuVector.hpp>
#include <cassert>
#include <iostream>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//======================================================================
void PedestrianActor::setPedestrian(
    Pedestrian* ped, PedestrianBehavior* behavior,
    PedestrianDecision* decision, PedestrianLocation* location)
{
    _pedestrian = ped;
    _behavior   = behavior;
    _decision   = decision;
    _location   = location;
}

//======================================================================
void PedestrianActor::act()
{
    Zebra* zebra = _location->zebra();

    if (!zebra)
    {
        cout << _pedestrian->id() << endl;
        exit(EXIT_FAILURE);
    }

    assert(zebra);

    _renewVelocity();

    // 移動後の位置の候補
    // Possible position after move
    AmuPoint positionCandidate
        = _location->position()
          + _behavior->velocity() * AppMates::getTimeManager().unit();

    // 側方境界を越える場合は境界線付近で一旦停止する
    // When crossing a side border, pause near the border.
    const AmuLineSegment trajectory(
        _location->position(), positionCandidate);
    AmuPoint              crossPoint;
    const AmuLineSegment& edge0 = zebra->edge(0);
    const AmuLineSegment& edge2 = zebra->edge(2);
    if (edge0.createIntersectionPoint(&trajectory, &crossPoint)
        || edge2.createIntersectionPoint(&trajectory, &crossPoint))
    {
        positionCandidate
            = _location->position()
              + AmuVector(_location->position(), crossPoint) * 0.5;
        _behavior->setVelocity(AmuVector(0.0, 0.0, 0.0));
    }

    // 停止すべきレーンに近づいたら一旦停止する
    static constexpr double gap = 2.0; //[m]
    const Lane* laneToStop      = _pedestrian->scene()->laneToStop();
    if (laneToStop
        && laneToStop->lineSegment()->distance(_location->position())
               > gap
        && laneToStop->lineSegment()->distance(positionCandidate) < gap)
    {
        positionCandidate = _location->position();
        _behavior->setVelocity(AmuVector(0.0, 0.0, 0.0));
    }

    // 横断歩道を渡り終えたら停止する
    // Stop after crossing the crosswalk
    const AmuLineSegment newTrajectory(
        _location->position(), positionCandidate);
    const AmuLineSegment& goalEdge
        = zebra->edge((_behavior->crossingDirection() == 0 ? 3 : 1));
    if (goalEdge.createIntersectionPoint(&newTrajectory, &crossPoint))
    {
        _behavior->setVelocity(AmuVector(0.0, 0.0, 0.0));
        _location->setIsOnZebra(false);
        zebra->endEdge(_behavior->crossingDirection())
            ->appendFinishedPedestrian(_pedestrian);
    }
    else
    {
        _location->setPosition(positionCandidate);
        zebra->putPedestrian(_pedestrian);
    }
}

//======================================================================
void PedestrianActor::_renewVelocity()
{
    // STOP: 速度をゼロに
    // STOP: Set velocity to zero
    if (_decision->walkType() == WalkType::STOP)
    {
        _behavior->setVelocity(AmuVector(0.0, 0.0, 0.0));
        return;
    }

    AmuVector dir;
    double    distanceToTargetPosition;
    if (_decision->walkType() == WalkType::FREEWALK)
    {
        // FREEWALK: 歩行方向を希望歩行方向に
        // FREEWALK: Set walking direction to the desired direction
        dir = _behavior->desiredDirection();
    }
    else
    {
        // 歩行方向を目標地点向きに
        // Set walking direction to that toward the target position
        dir = AmuVector(
            _location->position(), _decision->targetPosition());
    }
    distanceToTargetPosition = dir.size();
    dir.normalize();

    // 歩行速さの候補
    // A candidate for walking speed
    double speed = _behavior->maxSpeed();
    if (_decision->walkType() == WalkType::OVERTAKE
        || _location->zebra()->signal()->walkerColor(
               _behavior->crossingDirection())
               != SignalColor::WalkerState::BLUE)
    {
        /*
         * 青信号以外や追い越し時は加速
         *
         * Accelerate when the traffic light is not green or when
         * overtaking
         */
        speed *= _behavior->accelFactor();
    }

    /*
     * 目標地点を越えそうな場合には速さを調節する
     *
     * Adjust speed if the pedestrian is about to go over the target
     * point.
     */
    if (speed * AppMates::getTimeManager().unit()
        > distanceToTargetPosition)
    {
        speed = distanceToTargetPosition
                / AppMates::getTimeManager().unit();
    }

    _behavior->setVelocity(dir * speed);
}

#endif //INCLUDE_PEDESTRIANS
