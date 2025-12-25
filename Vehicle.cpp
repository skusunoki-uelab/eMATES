/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file Vehicle.cpp
 */
#include "Vehicle.hpp"
#include "AppMates.hpp"
#include "GVManager.hpp"
#include "Intersection.hpp"
#include "Lane.hpp"
#include "LaneBundle.hpp"
#include "ODNode.hpp"
#include "RouterAStarHierarchy.hpp"
#include "RandomNumberGenerator.hpp"
#include "RoadMap.hpp"
#include "RouterBase.hpp"
#include "RouterManager.hpp"
#include "RoutingRecorder.hpp"
#include "Section.hpp"
#include "Signal.hpp"
#include "Simulator.hpp"
#include "SubLaneBundle.hpp"
#include "TimeManager.hpp"
#include "VirtualLeader.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <sstream>
#include <AmuPoint.hpp>
#include <AmuVector.hpp>
#ifdef INCLUDE_PEDESTRIANS
#include "ped/VehiclePedExt.hpp"
#endif //INCLUDE_PEDESTRIANS

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//==============================================================================
Vehicle::Vehicle() : _id()
{
    _rng.reset();

    _body->setVehicle(this);
    _behavior.setVehicle(this);
    _decision.setVehicle(this);
    _localRoute.setVehicle(this);
    _location.setVehicle(this);
    _scene.setVehicle(this);

    _perceiver.setVehicle(
        this, &_behavior, _body.get(), &_decision, &_globalRoute, &_localRoute,
        &_location, &_scene);
    _lcPerceiver.setVehicle(
        this, &_behavior, _body.get(), &_decision, &_localRoute, &_location, &_scene,
        &_localRouter);
    _determiner.setVehicle(
        this, _body.get(), &_behavior, &_decision, &_location, &_scene);
    _lcDeterminer.setVehicle(
        this, _body.get(), &_behavior, &_decision, &_location, &_scene);
    _actor->setVehicle(
        this, &_behavior, _body.get(), &_decision, &_globalRoute, &_localRoute,
        &_location, &_scene, &_localRouter, &_blinker);
    _lcActor.setVehicle(
        this, _body.get(), &_behavior, &_decision, &_location, &_scene);
    _localRouter.setVehicle(this, &_globalRoute, &_localRoute);

    _localRoute.clearLocalRoute();
    _preferredNetworkRank = INT_MAX;
    _blinker.setNone();

#ifdef INCLUDE_PEDESTRIANS
    _pedExt = new VehiclePedExt();
    _pedExt->setVehicle(
        this, &_behavior, _body.get(), &_globalRoute, &_location, &_scene);
    _perceiver.setPedExt(_pedExt);
#endif //INCLUDE_PEDESTRIANS
}

//==============================================================================
Vehicle::~Vehicle()
{
    if (_location.intersection())
    {
        const_cast<Intersection*>(_location.intersection())
            ->eraseWatchedVehicle(this);
    }
    if (_location.section())
    {
        const_cast<Section*>(_location.section())->eraseWatchedVehicle(this);
    }

#ifdef INCLUDE_PEDESTRIANS
    delete _pedExt;
#endif //INCLUDE_PEDESTRIANS
}

//==============================================================================
void Vehicle::preperceive()
{
    _lcPerceiver.preperceive();
    _perceiver.preperceive();
}

//==============================================================================
void Vehicle::perceive()
{
    // スリープ中は何もしない
    // Do nothing during inactive
    if (_behavior.sleepDuration() > 0)
    {
        return;
    }
    _lcPerceiver.perceive();
    _perceiver.perceive();
}

//==============================================================================
void Vehicle::determine()
{
    if (_location.section())
    {
        _lcDeterminer.determine();
    }
    _determiner.determine();
}

//==============================================================================
void Vehicle::act()
{
    if (_location.section())
    {
        _lcActor.act();
    }
    _actor->act();
}

//==============================================================================
void Vehicle::postact()
{
    _actor->postact();
}

//==============================================================================
bool Vehicle::addToSection(
    RoadMap* roadMap, Section* section, const Lane* lane, double distance)
{
    // 車両が初めて登場したときにだけ呼ばれる
    // Called only when the car first appears
    assert(_location.roadMap());

    _location.setRoadMap(roadMap);
    _location.setSection(section);
    _location.setPrevIntersection(
        section->intersection(!(section->isUp(lane))));
    _location.setDistance(distance);
    _location.setSectionInflowTime(AppMates::getTimeManager().time());

    _location.setLane(lane);

#ifdef GENERATE_VEHICLE_VELOCITY_ZERO
    _behavior.setVelocity(0);
#else  //GENERATE_VEHICLE_VELOCITY_ZERO not defined
    _behavior.setVelocity(lane->speedLimit() / 3600); //[km/h]->[m/ms]
#endif //GENERATE_VEHICLE_VELOCITY_ZERO

    double limit
        = AppMates::getGVManager().getNumeric("GENERATE_VELOCITY_LIMIT")
        / 3600.0;
    if (limit >= 0.0 && _behavior.velocity() > limit)
    {
        _behavior.setVelocity(limit);
    }

    const_cast<Lane*>(lane)->putAgent(this);
    return true;
}

//==============================================================================
void Vehicle::firstLocalReroute(
    const Section* section, const Lane* lane, double distance)
{
    _localRouter.clear();
    _localRouter.localReroute(section, lane, distance);
    _location.setNextLane(_localRouter.nextLaneWithoutLaneChange(lane));
    _location.setPrevLane(lane);
}

//==============================================================================
bool Vehicle::isAwayFromOriginNode() const
{
    if (_location.tripLength() < AppMates::getGVManager().getNumeric(
            "NO_OUTPUT_LENGTH_FROM_ORIGIN_NODE"))
    {
        return false;
    }
    return true;
}

//==============================================================================
int Vehicle::directionFrom() const
{
    const Intersection* inter;
    if (_location.intersection())
    {
        inter = _location.intersection();
    }
    else
    {
        inter = _location.section()->intersection(
            _location.section()->isUp(_location.lane()));
    }
    if (inter)
    {
        auto liInter = _localRoute.lanesInIntersection();
        assert(inter->containsLane(*liInter.begin()));
        return inter->direction((*liInter.begin())->beginConnector());
    }
    else
    {
        return -1;
    }
}

//==============================================================================
int Vehicle::directionTo() const
{
    const Intersection* inter;
    if (_location.intersection())
    {
        inter = _location.intersection();
    }
    else
    {
        inter = _location.section()->intersection(
            _location.section()->isUp(_location.lane()));
    }
    if (inter)
    {
        auto liInter = _localRoute.lanesInIntersection();
        assert(inter->containsLane(*liInter.rbegin()));
        return inter->direction((*liInter.rbegin())->endConnector());
    }
    else
    {
        return -1;
    }
}

//==============================================================================
bool Vehicle::reroute(const Intersection* rear, const Intersection* front)
{
    /*
     * 経路探索に失敗した場合の繰り返し回数
     *   探索に失敗すると目的地を変更して再探索を試みる
     *
     * Number of rerouting when routing fails
     *   After a routing failed, change the destination and try again.
     */
    const int maxRerouting = 2;

    for (int i = 0; i < maxRerouting; i++)
    {
        // 走行中の再探索である場合はカウンタをインクリメント
        // If rerouting while driving, increment the counter
        if (rear != _globalRoute.start())
        {
            _globalRoute.incrementNumRerouting();
        }

        // 経路選択器を取得
        // Get router
        RouterBase* router = AppMates::getRouterManager().assignRouter();

        // 属性を設定
        // Set attributes
        router->setId(_id);
        router->setVehicleType(_body->type());
        router->setWeights(_routingParams);
        if (dynamic_cast<RouterAStarHierarchy*>(router))
        {
            dynamic_cast<RouterAStarHierarchy*>(router)
                ->setPreferredNetworkRank(_preferredNetworkRank);
        }
        router->setRandomNumberGenerator(&_rng);

        // 経由すべき交差点
        // Intersections to be passed
        vector<const Intersection*> gates;
        _globalRoute.getGatesToPass(gates);

        // 経路選択結果を_globalRouteに設定
        // Set routing result to _globalRoute
        Route route = router->search(rear, front, gates);
        _globalRoute.setRoute(route);

        // 経路選択器を返却
        // Return path router
        AppMates::getRouterManager().releaseRouter(router);

        if (_globalRoute.route().isValid())
        {
            break;
        }

        // 経路選択に失敗した場合は新しい目的地を設定
        // Set new destination if routing fails
        _setNewGoal();
    }
    return _globalRoute.route().isValid();
}

//==============================================================================
void Vehicle::_setNewGoal()
{
    // 現在の目的地への経路探索は失敗したのでfailedGoalsに登録
    // Register to failedGoals since routing to current goal has failed.
    _globalRoute.addFailedGoal(_globalRoute.goal());

    auto odNodes = _location.roadMap()->odNodes();

    double              minAngle = 10000;
    const Intersection* newGoal  = NULL;

    for (auto itr : odNodes)
    {
        const Intersection* inter = itr;
        if (find(
                _globalRoute.failedGoals().begin(),
                _globalRoute.failedGoals().end(), inter)
            != _globalRoute.failedGoals().end())
        {
            continue;
        }

        AmuVector dv(_location.position(), inter->center());
        double    angle = _location.directionVector().calcAngle(dv);
        if (fabs(angle) < minAngle)
        {
            minAngle = fabs(angle);
            newGoal  = itr;
        }
    }

    // 候補がなければ最初の目的地に戻す
    // Reset to the first goal if there are no candidates
    if (!newGoal)
    {
        newGoal = _globalRoute.failedGoal(0);
    }

    cout << "vehicle: " << _id << ", set new goal: " << newGoal->id() << endl;

    vector<const Intersection*> gates;
    gates.clear();
    gates.push_back(_globalRoute.start());
    gates.push_back(newGoal);
    _globalRoute.setGates(gates);
    _globalRoute.setLastPassedGateIndex(_globalRoute.start());
}

//==============================================================================
void Vehicle::print(ostream& out) const
{
    stringstream ss;

    ss << "--- Vehicle Information ---" << endl;
    ss << "ID: " << _id << ", Type: " << *(_body->type()) << endl;

    _location.print(ss);
    _behavior.print(ss);

    ss << "Routing Parameter: (" << _routingParams[0];
    for (auto itr : _routingParams)
    {
        ss << ", " << itr;
    }
    ss << ")" << endl;

    ss << "PreferredNetworkRank: " << _preferredNetworkRank << endl;

    _globalRoute.print(ss);
    _localRoute.print(ss);

    _localRouter.print(ss);
    _decision.printLaneChangeFlags(ss);

    _scene.print(ss);
    ss << endl;

#pragma omp critical(out_critical)
    out << ss.str();
}
