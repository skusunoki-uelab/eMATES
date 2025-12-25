/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file PedestrianDeterminer.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "PedestrianDeterminer.hpp"
#include "Pedestrian.hpp"
#include "PedestrianBehavior.hpp"
#include "PedestrianBodyProperty.hpp"
#include "PedestrianDecision.hpp"
#include "PedestrianLocation.hpp"
#include "PedestrianPerceiver.hpp"
#include "VehiclePedExt.hpp"
#include "Zebra.hpp"
#include "../SignalColor.hpp"
#include "../Vehicle.hpp"
#include <AmuPoint.hpp>
#include <cassert>
#include <cmath>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//======================================================================
void PedestrianDeterminer::setPedestrian(
    Pedestrian* ped, PedestrianBehavior* behavior,
    PedestrianBodyProperty* body, PedestrianDecision* decision,
    PedestrianLocation* location, PedestrianPerceiver* perceiver,
    PedestrianScene* scene)
{
    _pedestrian = ped;
    _behavior   = behavior;
    _body       = body;
    _decision   = decision;
    _location   = location;
    _perceiver  = perceiver;
    _scene      = scene;
}

//======================================================================
void PedestrianDeterminer::determine()
{
    if (_scene->nearestPedestrian())
    {
        // 近傍歩行者が存在する
        // There is a pedestrian nearby
        _determineBehaviorAgainstPedestrian();
    }
    else
    {
        /*
         * 近くに何もいないので自由歩行
         *   自動車が接近するレーンが近い場合は，PedestrianActor にて
         *   挙動を上書きしレーンの目前で停止するようにする．
         *
         * There is nothing nearby so the pedestrian can walk freely
         *   If the lane where the car approaches is close, overwrite
         *   behavior to stop in front of the lane in PedestrianActor.
         */
        _decision->setWalkType(WalkType::FREEWALK);
    }

    return;
}

//======================================================================
void PedestrianDeterminer::_determineBehaviorAgainstPedestrian()
{
    double psRad = _body->psRadius();

    const Pedestrian* nearest = _scene->nearestPedestrian();
    assert(nearest);

    double distance     = _scene->distanceToNearestPedestrian();
    double sideDistance = _scene->sideDistanceToNearestPedestrian();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 最近接歩行者が同方向に歩いている場合
    // When the nearest pedestrian is walking in the same direction
    if (nearest->behavior()->crossingDirection()
        == _behavior->crossingDirection())
    {
        //--------------------------------------------------------------
        /*
         * 後ろにいるので最近接歩行者を無視して自由歩行する
         *   後方は視野の外であるので，例外的な状況でなければこの条件に
         *   適合することはない．
         *   
         * Walk freely with ignoring the nearest pedestrian because it's
         * behind this pedestrian.
         *   Since the rear is outside the visual field, this condition
         *   will not be met unless the exceptional circumstances.
         */
        if (distance <= 0)
        {
            _decision->setWalkType(WalkType::FREEWALK);
            return;
        }

        //--------------------------------------------------------------
        // 最近接歩行者が近すぎる場合は一時停止する
        // Pause if the nearest pedestrian is too close.
        else if (distance < psRad * 2)
        {
            _decision->setWalkType(WalkType::STOP);
            return;
        }

        //--------------------------------------------------------------
        /*
         * 最近接歩行者がある程度離れており，かつ，視野内に対向歩行者が
         * いる場合は，最近接歩行者に追従するための目標位置を定める
         *
         * If the nearest pedestrian is a certain distance away and
         * there are any oncoming pedestrians within visual field, set
         * the target position to follow the nearest pedestrian.
         */
        else if (_scene->existsOncomingPedestrian())
        {
            _decision->setWalkType(WalkType::FOLLOW);
            _determineTargetPosition(nearest, 2 * psRad, 0);
            return;
        }

        //--------------------------------------------------------------
        /*
         * 対向歩行者が存在せず，最近接歩行者よりこの歩行者が速い場合
         *
         * If there is no oncoming pedestrian and this pedestrian is
         * faster than the nearest pedestrian.
         */
        if (_behavior->velocity().size() > nearest->velocity().size())
        {
            //----------------------------------------------------------
            /*
             * 横方向に十分に離れていればそのまま自由に歩行する
             *
             * If the nearest pedestrian is far enough away laterally,
             * walk freely.
             */
            if (fabs(sideDistance) >= psRad * 2)
            {
                _decision->setWalkType(WalkType::FREEWALK);
                return;
            }

            //----------------------------------------------------------
            /*
             * 横方向に十分な間隔を確保できない場合は，最近接歩行者を
             * 追い越すための目標位置を定める
             *
             * If sufficient lateral distance cannot be ensured, set the
             * target position for overtaking the nearest pedestrian.
             */
            else
            {
                _decision->setWalkType(WalkType::OVERTAKE);
                double requiredGap
                    = psRad * 2 * (sideDistance >= 0 ? -1 : 1);
                _determineTargetPosition(nearest, 0, requiredGap);
                return;
            }
        }

        //--------------------------------------------------------------
        /*
         * 対向歩行者が存在せず，最近接歩行者よりこの歩行者が遅い場合
         *
         * If there is no oncoming pedestrian and this pedestrian is
         * slower than the nearest pedestrian.
         */
        else
        {
            //----------------------------------------------------------
            // そのまま自由に歩行する
            // Walk freely.
            _decision->setWalkType(WalkType::FREEWALK);
            return;
        }
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 最近接歩行者が逆方向に歩いている場合
    // When the nearest pedestrian is walking in the opposite direction
    else
    {
        //--------------------------------------------------------------
        /*
         * 横方向に十分に離れていればそのまま自由に歩行する
         *
         * If the nearest pedestrian is far enough away laterally, walk
         * freely.
         */
        if (fabs(sideDistance) >= psRad * 2)
        {
            _decision->setWalkType(WalkType::FREEWALK);
            return;
        }

        //--------------------------------------------------------------
        /*
         * 横方向に十分な間隔を確保できない場合は，最近接歩行者との
         * 衝突を回避するための目標位置を定める
         *
         * If sufficient lateral distance cannot be ensured, set the
         * target position to avoid collision with the nearest
         * pedestrian.
         */
        else
        {
            _decision->setWalkType(WalkType::AVOID);
            double requiredGap
                = psRad * 2 * (sideDistance >= 0 ? -1 : 1);
            _determineTargetPosition(nearest, 0, requiredGap);
        }
    }
}

//======================================================================
void PedestrianDeterminer::_determineTargetPosition(
    const Pedestrian* ped, double longitudinalGap, double lateralGap)
{
    // 希望歩行方向
    // Desired walking direction
    //AmuVector dvec = _behavior->desiredDirection();
    AmuVector dvec = _behavior->velocity();
    if (_behavior->isAlmostStopped())
    {
        dvec = _behavior->desiredDirection();
    }


    // 歩行方向を右回りに90度回転させたベクトル
    // A vector that rotates the walking direction 90 [deg] clockwise
    AmuVector nvec = dvec;
    nvec.revoltXY(-M_PI_2);
    nvec.normalize();

    // 目標位置の候補
    // Target position candidate
    AmuPoint positionCandidate
        = ped->position() - longitudinalGap * dvec + lateralGap * nvec;

    /*
     * 追い越し時に横断歩道の側方境界を越えるようであれば gap を反転する
     *
     * If the pedestrian will walk beyond the side border of crosswalk
     * when overtaking, reverse the gap.
     */
    Zebra* zebra = _location->zebra();
    double psRad = _body->psRadius();
    assert(zebra);
    if (_decision->walkType() == WalkType::OVERTAKE)
    {
        // 左方間隔の確認
        // Check left spacing
        if (lateralGap < 0)
        {
            const AmuLineSegment& leftLine = zebra->edge(
                _behavior->crossingDirection() == 0 ? 0 : 2);
            if (leftLine.distance(positionCandidate) < psRad)
            {
                lateralGap *= -1;
                positionCandidate = ped->position()
                                    - longitudinalGap * dvec
                                    + lateralGap * nvec;
            }
        }
        // 右方間隔の確認
        // Check right spacing
        else
        {
            const AmuLineSegment& rightLine = zebra->edge(
                _behavior->crossingDirection() == 0 ? 2 : 0);
            if (rightLine.distance(positionCandidate) < psRad)
            {
                lateralGap *= -1;
                positionCandidate = ped->position()
                                    - longitudinalGap * dvec
                                    + lateralGap * nvec;
            }
        }
    }

    _decision->setTargetPosition(positionCandidate);
}

#endif //INCLUDE_PEDESTRIANS
