/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file VehicleGenerator.cpp
 */
#include "VehicleGenerator.hpp"
#include "AppMates.hpp"
#include "Config.hpp"
#include "CustomMessage.hpp"
#include "GVManager.hpp"
#include "Intersection.hpp"
#include "Lane.hpp"
#include "ObjectManager.hpp"
#include "ODNode.hpp"
#include "ODNodeGroup.hpp"
#include "RandomNumberGenerator.hpp"
#include "RoadMap.hpp"
#include "Section.hpp"
#include "TimeManager.hpp"
#include "Vehicle.hpp"
#include "VehicleBodyProperty.hpp"
#include "VehicleEVBodyProperty.hpp"
#include "VehicleEVTypeProperty.hpp"
#include "VehicleTypeManager.hpp"
#include "io/Logger.hpp"
#include "io/VehicleGeneratorBuilder.hpp"
#ifdef INCLUDE_TRAMS
#include "tram/VehicleTram.hpp"
#endif //INCLUDE_TRAMS
#include <AmuConverter.hpp>
#include <cassert>
#include <climits>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;
using namespace amu::converter;

/*
 * 車両を確率的でなく等間隔で発生させるは以下のマクロを定義する
 *
 * Define the macro below to generate vehicles at equal intervals instead of
 * stochastically
 */
// #define GENERATE_VEHICLE_EQUAL_INTERVAL

//==============================================================================
VehicleGenerator::VehicleGenerator(RoadMap* roadMap)
{
    _roadMap = roadMap;

    _generatingQueue.clear();
    _generatingPriorQueue.clear();
    _routingParams.clear();
    _routingPrefRank.clear();
    _odStartGroup.clear();
    _odGoalGroup.clear();
    _odPairGroup.clear();
    _startNodes.clear();
    for (unsigned int i = 0; i < 3; i++)
    {
        _startLevel[i].clear();
    }
    _goalNodes.clear();
    for (unsigned int i = 0; i < 3; i++)
    {
        _goalLevel[i].clear();
    }
    _totalDefaultGoalTrafficVolume = 0;
    _excludedStartNodes.clear();
    _excludedGoalNodes.clear();
    _rng.reset();
#ifdef _OPENMP
    omp_init_lock(&_lock);
#endif //_OPENMP
}

//==============================================================================
VehicleGenerator::~VehicleGenerator()
{
    for (auto itr : _odStartGroup)
    {
        delete itr.second;
    }
    _odStartGroup.clear();

    for (auto itr : _odGoalGroup)
    {
        delete itr.second;
    }
    _odGoalGroup.clear();

    for (auto itr : _odPairGroup)
    {
        delete itr.second;
    }
    _odPairGroup.clear();

#ifdef _OPENMP
    omp_destroy_lock(&_lock);
#endif //_OPENMP
}

//==============================================================================
void VehicleGenerator::addExcludedRandomStartNode(ODNode* node)
{
    // 重複をチェックする
    // Check for duplicates
    if (_excludedStartNodes.find(node->id()) != _excludedStartNodes.end())
    {
        ostringstream ssw;
        ssw << "origin node[" << node->id()
            << "] has been already added to _excludedStartNodes.";
        amu::msg::warn(ssw.str());
        return;
    }

    _excludedStartNodes.insert(make_pair(node->id(), node));
}

//==============================================================================
void VehicleGenerator::addExcludedRandomGoalNode(ODNode* node)
{
    // 重複をチェックする
    // Check for duplicates
    if (_excludedGoalNodes.find(node->id()) != _excludedGoalNodes.end())
    {
        ostringstream ssw;
        ssw << "destination node[" << node->id()
            << "] has been already added to _excludedGoalNodes.";
        amu::msg::warn(ssw.str());
        return;
    }

    _excludedGoalNodes.insert(make_pair(node->id(), node));
}

//==============================================================================
int VehicleGenerator::getStartLevel(ODNode* node) const
{
    int result = -1;

    // ODNodeからの流出点の数
    // The number of outflow points from ODNode
    if (node->border(0)->numOut() >= 3)
    {
        result = 0;
    }
    else if (node->border(0)->numOut() == 2)
    {
        result = 1;
    }
    else if (node->border(0)->numOut() == 1)
    {
        result = 2;
    }
    return result;
}

//==============================================================================
int VehicleGenerator::getGoalLevel(ODNode* node) const
{
    int result = -1;

    // ODNodeへの流入点の数
    // The number of inflow points to ODNode
    if (node->border(0)->numIn() >= 3)
    {
        result = 0;
    }
    else if (node->border(0)->numIn() == 2)
    {
        result = 1;
    }
    else if (node->border(0)->numIn() == 1)
    {
        result = 2;
    }
    return result;
}

//==============================================================================
void VehicleGenerator::addODGroup(GroupingType type, ODNodeGroup* group)
{
    switch (type)
    {
    case GroupingType::START:
        _odStartGroup.insert(make_pair(group->id(), group));
        break;
    case GroupingType::GOAL:
        _odGoalGroup.insert(make_pair(group->id(), group));
        break;
    case GroupingType::PAIR:
        _odPairGroup.insert(make_pair(group->id(), group));
        break;
    case GroupingType::NONE:
    default:
        cerr << "invalid od group type" << endl;
        break;
    }
    return;
}

//==============================================================================
void VehicleGenerator::addGTCellToTable(
    GeneratingTableType type, GeneratingTableCell* cell)
{
    switch (type)
    {
    case GeneratingTableType::NORMAL:
        _table.addGTCell(cell);
        break;
    case GeneratingTableType::DEFAULT:
        _defaultTable.addGTCell(cell);
        break;
    case GeneratingTableType::FIXED:
        _fixedTable.addGTCell(cell);
        break;
    case GeneratingTableType::GROUP:
        _groupedTable.addGTCell(cell);
        break;
    case GeneratingTableType::RANDOM:
        _randomTable.addGTCell(cell);
        break;
    default:
        cerr << "ERROR: GeneratingTableType[" << toUnderlying(type)
             << "] is invalid" << endl;
        exit(EXIT_FAILURE);
    }
}

//==============================================================================
void VehicleGenerator::sortGeneratingTable(GeneratingTableType type)
{
    switch (type)
    {
    case GeneratingTableType::NORMAL:
        _table.sortGTCells();
        break;
    case GeneratingTableType::DEFAULT:
        _defaultTable.sortGTCells();
        break;
    case GeneratingTableType::FIXED:
        _fixedTable.sortGTCells();
        break;
    case GeneratingTableType::GROUP:
        _groupedTable.sortGTCells();
        break;
    case GeneratingTableType::RANDOM:
        _randomTable.sortGTCells();
        break;
    default:
        cerr << "ERROR: GeneratingTableType[" << toUnderlying(type)
             << "] is invalid" << endl;
        exit(EXIT_FAILURE);
    }
}

//==============================================================================
void VehicleGenerator::generateVehicle()
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 現在適用可能であるが有効化されていない GeneratingTableCell を有効化し，
     * 車両発生イベントキューを更新する．
     *
     * Activate GeneratingTableCells that are presently applicable but not yet
     * activated and update the vehicle generation event queue.
     */
    _activatePresentGTCells();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 車両を生成し，該当する ODNode の _waitingVehicles に登録する．
     *   発生時刻が固定された優先車両は _waitingVehicles の先頭に追加し，
     *   それ以外の車両は末尾に追加する．
     *
     * Generate a vehicle and register it in _waitingVehicles of the
     * corresponding ODNode.
     *   Priority vehicles with fixed generation time are added to the head of
     *   waitingVehicles, and other vehicles are added to the tail.
     */

    // 優先車両
    // Priority vehicles
    _generateVehiclesFromPriorQueue();

    // 一般の車両
    // Regular vehicles
    _generateVehiclesFromQueue();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 流入点付近に十分な空きのある ODNode において，_waitingVehicles の先頭から
     * 車両を取り出し，車線に登録する．
     *   登録順に車両IDが付与されるため，この処理はシングルスレッド上で実行
     *   しなければならない
     *
     * At the ODNode with sufficient space near the inflow point, take the
     * vehicle from the head of _waitingVehicles and register it in the lane.
     *   Since vehicle ID numbers are assigned in order of registration, this
     *   process must be executed on a single thread.
     */
    vector<ODNode*> tmpODNodes;
    for (auto itr : _waitingODNodes)
    {
        itr->pushVehicleToRoadMap(_roadMap);
        if (itr->hasWaitingVehicles())
        {
            /*
             * 登録すべき waitingVehicles が残っている場合，次のステップで再度
             * pushVehicle を試みる．
             *
             * If there are still waitingVehicles to register, try pushVehicle
             * again in the next step.
             */
            tmpODNodes.push_back(itr);
        }
        else
        {
            itr->setWaitsToPushVehicle(false);
        }
    }
    _waitingODNodes.swap(tmpODNodes);
}

//==============================================================================
void VehicleGenerator::generateVehicleManually(
    const std::string& startId, const std::string& goalId,
    std::vector<std::string>* gateIds, VehicleType vehicleType,
    std::vector<double> params, unsigned int prefRank)
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 出発地・目的地・経由地の決定
    // Determine origin, destination, and way-points
    ODNode* start = dynamic_cast<ODNode*>(_roadMap->intersection(startId));
    if (!start)
    {
        cerr << "WARNING: start[" << startId << "] is not a OD node." << endl;
        return;
    }

    ODNode* goal;
    if (goalId == "******")
    {
        goal = _decideGoalRandomly(start);
        if (!goal)
        {
            cerr << "WARNING: goal[" << goalId << "] cannot be decided."
                 << endl;
            return;
        }
    }
    else
    {
        goal = dynamic_cast<ODNode*>(_roadMap->intersection(goalId));
        if (!goal)
        {
            cerr << "WARNING: goal[" << goalId << "] is not a OD node." << endl;
            return;
        }
    }

    /*
     * gateIdsの最後が"******"の場合は上で決定されたgoalに上書きされる
     *
     * If the tail of gateIds may "******", it will be overwritten with the
     * goal determined above.
     */
    vector<string> myGateIds;
    myGateIds.clear();
    for (unsigned int j = 0; j < gateIds->size() - 1; j++)
    {
        myGateIds.push_back((*gateIds)[j]);
    }
    myGateIds.push_back(goal->id());

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 車両を生成し，出発地の_waitingVehiclesの先頭に追加する
     *
     * Generate a vehicle and add it to the head of _waitingVehicles at the
     * origin.
     */
    Vehicle* newVehicle = _createVehicle(
        start, goal, start, start->next(0), myGateIds, vehicleType);

    newVehicle->setRoutingParams(params);
    newVehicle->setPreferredNetworkRank(prefRank);

    start->prependWaitingVehicle(newVehicle);

#ifdef _OPENMP
    omp_set_lock(&_lock);
#endif //_OPENMP
    if (!(start->waitsToPushVehicle()))
    {
        _waitingODNodes.push_back(start);
        start->setWaitsToPushVehicle(true);
    }
#ifdef _OPENMP
    omp_unset_lock(&_lock);
#endif
    newVehicle->globalRoute()->print(cout);
}

//==============================================================================
void VehicleGenerator::_activatePresentGTCells()
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 現在時刻で有効なセルを抽出する
    // Extract valid cells at present time
    vector<const GeneratingTableCell*> activeGTCells;
    _table.extractActiveGTCells(activeGTCells);
    _defaultTable.extractActiveGTCells(activeGTCells);
    _groupedTable.extractActiveGTCells(activeGTCells);
    _randomTable.extractActiveGTCells(activeGTCells);

    // 抽出したセルすべてについて車両発生時刻を求める
    // Find vehicle generation time for all extracted cells
    for (auto itr : activeGTCells)
    {
        _determineNextTimeOfGeneration(itr->begin(), itr);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 上記と同様の手順を _fixedTable についても繰り返す
     *   ただしフラグ DEBUG_FLAG_GEN_FIXED_VEHICLE_ALL_AT_ONCE が立っている
     *   場合は現在時刻に関係なくすべてのセルを抽出する
     *
     * Repeat the same steps as above for _fixedTable.
     *   But if the flag DEBUG_FLAG_GEN_FIXED_VEHICLE_ALL_AT_ONCE is on,
     *   extract all cells regardless of present time.
     */
    vector<const GeneratingTableCell*> activeFixedGTCells;
    if ((AppMates::getGVManager().getFlag(
            "DEBUG_FLAG_GEN_FIXED_VEHICLE_ALL_AT_ONCE")))
    {
        _fixedTable.extractActiveGTCellsAllAtOnce(activeFixedGTCells);
    }
    else
    {
        _fixedTable.extractActiveGTCells(activeFixedGTCells);
    }

    for (auto itr : activeFixedGTCells)
    {
        _determineFixedTimeOfGeneration(itr);
    }
}

//==============================================================================
bool VehicleGenerator::_determineNextTimeOfGeneration(
    ulint startTime, const GeneratingTableCell* cell)
{
    if (cell->volume() < 1.0e-3)
    {
        return false;
    }

    // 平均発生時間間隔 [ms]
    // Mean generation time interval [ms]
    double meanInterval = (60.0 * 60.0 * 1000.0) / (cell->volume());

    // 発生時間間隔 [ms]
    // Generation time interval [ms]
    ulint interval;

    // 等間隔で車両を生成する場合
    // In the case of generating vehicles at equal intervals
    if (AppMates::getGVManager().getFlag("FLAG_GEN_VEHICLE_EQUAL_INTERVAL"))
    {
        if (cell->numGeneratedVehicles() == 0)
        {
            /*
             * このセルによって初めて車両が生成される場合
             *   たとえば発生交通量が1 veh./h の場合は30分で最初の1台を発生
             *   させる．
             *
             * If this cell generates a vehicle for the first time
             *   For example, if the generation volume is 1 veh./h, generate
             *   the first vehicle at 30 min.
             */
            interval = ceil(meanInterval / 2.0);
        }
        else
        {
            /*
             * このセルによってすでに車両が生成されている場合
             *   たとえば発生交通量が1 veh.h の場合は60分間隔で車両を発生
             *   させる．
             *
             * If this cell already generated a vehicle
             *   For example, if the generation volume is 1 veh./h, generate
             *   vehicles every 60 mins.
             */
            interval = ceil(meanInterval);
            interval += AppMates::getTimeManager().unit()
                - interval % AppMates::getTimeManager().unit();
        }
    }

    // 車両発生間隔を確率的に決定する場合
    // In the case of determining vehicle generation intervals stochastically
    else
    {
        interval = ceil(-meanInterval * log(1 - _rng.uniform()));
        interval += AppMates::getTimeManager().unit()
            - interval % AppMates::getTimeManager().unit();
    }

    // 発生時刻 [ms]
    // Time of generation [ms]
    ulint nextTime = startTime + interval;

    if (nextTime <= cell->end())
    {
#ifdef _OPENMP
        omp_set_lock(&_lock);
#endif //_OPENMP
        GeneratingQueueKey key(nextTime, cell->id());
        _generatingQueue.insert(make_pair(key, cell));
        const_cast<GeneratingTableCell*>(cell)->incrementGeneratedVehicles();
#ifdef _OPENMP
        omp_unset_lock(&_lock);
#endif //_OPENMP
        return true;
    }
    else
    {
        return false;
    }
}

//==============================================================================
bool VehicleGenerator::_determineFixedTimeOfGeneration(
    const GeneratingTableCell* cell)
{
#ifdef _OPENMP
    omp_set_lock(&_lock);
#endif //_OPENMP
    GeneratingQueueKey key(cell->begin(), cell->id());
    _generatingPriorQueue.insert(make_pair(key, cell));

#ifdef _OPENMP
    omp_unset_lock(&_lock);
#endif //_OPENMP
    return true;
}

//==============================================================================
void VehicleGenerator::_generateVehiclesFromQueue()
{
    // 現在のステップで車両を発生させるGeneratingTableCell
    // GeneratingTableCell generating a vehicle at the present step
    vector<const GeneratingTableCell*> validCells;
    _extractValidCellFromEventQueue(_generatingQueue, validCells, false);
    if (validCells.empty())
    {
        return;
    }
    for (unsigned int i = 0; i < validCells.size(); i++)
    {
        //--------------------------------------------------------------
        // 車両の生成
        // Generate vehicle
        Vehicle* newVehicle = _generateVehicleFromGTCell(validCells[i]);
        if (!newVehicle)
        {
            cerr << "ERROR: cannot generate vehicle at "
                 << validCells[i]->origin() << endl;
            exit(EXIT_FAILURE);
        }

        // 経路探索パラメータ
        // Routing parameters
        _setRoutingParamsRandomly(newVehicle);

        // 経路探索で選好するネットワークランク
        // Preferred network rank for routing
        _setRoutingPrefRankRandomly(newVehicle);

        //--------------------------------------------------------------
        // 出発地における処理
        // Processing at the origin
        ODNode* start = dynamic_cast<ODNode*>(
            const_cast<Intersection*>(newVehicle->globalRoute()->start()));
        assert(start);
        if (newVehicle)
        {
            start->appendWaitingVehicle(newVehicle);
        }
        if (!(start->waitsToPushVehicle()))
        {
            _waitingODNodes.push_back(start);
            start->setWaitsToPushVehicle(true);
        }

        //--------------------------------------------------------------
        // 次の発生時刻の決定
        // Determine next time of generation
        _determineNextTimeOfGeneration(
            AppMates::getTimeManager().time(), validCells[i]);
    }
}

//==============================================================================
void VehicleGenerator::_generateVehiclesFromPriorQueue()
{
    // 現在のステップで車両を発生させる GeneratingTableCell
    // GeneratingTableCell generating a vehicle at the present step
    vector<const GeneratingTableCell*> validCells;
    _extractValidCellFromEventQueue(
        _generatingPriorQueue, validCells,
        AppMates::getGVManager().getFlag(
            "DEBUG_FLAG_GEN_FIXED_VEHICLE_ALL_AT_ONCE"));
    if (validCells.empty())
    {
        return;
    }
    for (unsigned int i = 0; i < validCells.size(); i++)
    {
        //----------------------------------------------------------------------
        // 車両の生成
        // Generate vehicle
        Vehicle* newVehicle = _generateVehicleFromGTCell(validCells[i]);
        if (!newVehicle)
        {
            cerr << "ERROR: cannot generate prior vehicle at "
                 << validCells[i]->origin() << endl;
            continue;
        }

        // 経路探索パラメータ
        // Routing parameters
        _setRoutingParamsRandomly(newVehicle);

        // 経路探索で選好するネットワークランク
        // Preferred network rank for routing
        _setRoutingPrefRankRandomly(newVehicle);

        //----------------------------------------------------------------------
        /* 出発地における処理
         *   出発を優先するので生成した車両を_waitingVehicleの先頭に追加する
         *
         * Processing at the origin
         *   Add the generated vehicle to the head of _waitingVehicles to give
         *   priority to departure
         */
        ODNode* start = dynamic_cast<ODNode*>(
            const_cast<Intersection*>(newVehicle->globalRoute()->start()));
        assert(start);
        start->prependWaitingVehicle(newVehicle);

        if (!(start->waitsToPushVehicle()))
        {
            _waitingODNodes.push_back(start);
            start->setWaitsToPushVehicle(true);
        }

        /*
         * 1台のみ確定的に発生させるので，次の発生時刻は必要ない
         *
         * Since only 1 vehicle is generated deterministically, the next time
         * of generation is not required.
         */
    }
}

//==============================================================================
void VehicleGenerator::_extractValidCellFromEventQueue(
    multimap<GeneratingQueueKey, const GeneratingTableCell*>& queue,
    vector<const GeneratingTableCell*>&                       result_validCells,
    bool generateVehicleAllAtOnceIsOn)
{
    while (true)
    {
        if (queue.empty())
        {
            break;
        }
        if (queue.begin()->first.time() > AppMates::getTimeManager().time())
        {
            // これ以降は次ステップ以降に処理する
            // After this, it will be processed after the next step
            if (!generateVehicleAllAtOnceIsOn)
            {
                break;
            }
        }
        result_validCells.emplace_back(queue.begin()->second);
        queue.erase(queue.begin());
    }

    return;
}

//==============================================================================
Vehicle* VehicleGenerator::_generateVehicleFromGTCell(
    const GeneratingTableCell* cell)
{
    // 出発地と目的地
    // Origin and destination
    ODNode* start = nullptr;
    ODNode* goal  = nullptr;

    if (cell->hasPairedODs())
    {
        _getStartAndGoalNodes(cell, &start, &goal);
        assert(start && goal);
    }
    else
    {
        start = _getStartNode(cell);
        if (!start)
        {
            amu::msg::error("Start[" + cell->origin() + "] not found");
            exit(EXIT_FAILURE);
        }
        assert(start);

        goal = _getGoalNode(cell, start);
        if (!goal)
        {
            if (cell->destination() == "******")
            {
                amu::msg::error(
                    "Cannot set goal randomly (start[" + cell->origin() + "])");
            }
            else
            {
                amu::msg::error(
                    "Goal[" + cell->destination() + "] not found (start["
                    + cell->origin() + "])");
            }
            exit(EXIT_FAILURE);
        }
        assert(goal);
    }


    // 経由地の取得
    // Get way-points
    vector<string> gateIds;
    _getGateIds(cell, start, goal, gateIds);


    // 車種
    // Vehicle type
    VehicleType type = cell->vehicleType();

    // 車両の生成
    // Create vehicle
    return _createVehicle(start, goal, start, start->next(0), gateIds, type);
}

//==============================================================================
Vehicle* VehicleGenerator::_createVehicle(
    ODNode* start, ODNode* goal, Intersection*, Intersection*,
    const vector<string>& gateIds, VehicleType vehicleType)
{
    assert(start && goal);

    // 生成された車両を指すポインタ
    // Pointer to the created vehicle
    Vehicle* tmpVehicle;
    if (vehicleType.category() == VehicleCategory::TRAM)
    {
#ifdef INCLUDE_TRAMS
        tmpVehicle = AppMates::getObjectManager().createTram();
#else //INCLUDE_TRAMS not defined
        cerr << "WARNING: cannot create VehicleTram "
             << "without compiling INCLUDE_TRAMS flag on." << endl;
        return nullptr;
#endif
    }
    else if (AppMates::getGVManager().getFlag("FLAG_GEN_EV")
            && vehicleType.category() == VehicleCategory::EV) // [eMATES]
    {
        tmpVehicle = AppMates::getObjectManager().createVehicleEV();
    }
    else
    {
        tmpVehicle = AppMates::getObjectManager().createVehicle();
    }

    // 車両属性の設定
    // Set vehicle attributes
    _setVehicleBodyAttributes(tmpVehicle, vehicleType);

    // 経由地の設定
    // Set way-points
    vector<const Intersection*> gates;
    _convertGateIds2Gates(gateIds, gates, start, goal);
    const_cast<VehicleGlobalRoute*>(tmpVehicle->globalRoute())->setGates(gates);

    return tmpVehicle;
}

//==============================================================================
void VehicleGenerator::_setVehicleBodyAttributes(
    Vehicle* vehicle, VehicleType type)
{
    double length, width, height;
    int    numCars;
    double accel, decel;
    double r, g, b;

    VehicleTypeProperty* vtp = AppMates::getVehicleTypeManager().property(type);
    if (!vtp)
    {
        if (type.category() == VehicleCategory::TRUCK)
        {
            vtp = AppMates::getVehicleTypeManager().property(
                VehicleType(VehicleCategory::TRUCK, 0));
        }
        else
        {
            vtp = AppMates::getVehicleTypeManager().property(
                VehicleType(VehicleCategory::PASSENGER, 0));
        }
    }
    vtp->getSize(&length, &width, &height);
    numCars = vtp->numCars();
    vtp->getPerformance(&accel, &decel);
    vtp->getBodyColor(&r, &g, &b);

    VehicleBodyProperty* body
        = const_cast<VehicleBodyProperty*>(vehicle->body());
    body->setType(type);
    body->setBodySize(length, width, height);

    body->setNumCars(numCars);

    body->setMaxAcceleration(accel * 1.0e-6); // [m/(s^2)]->[m/(ms^2)]
    body->setMaxDeceleration(decel * 1.0e-6);

    if (type.category() == VehicleCategory::BUS)
    {
        body->setJamDistance(
            AppMates::getGVManager().getNumeric("JAM_DISTANCE_BUS"));
    }
    else if (type.category() == VehicleCategory::TRUCK)
    {
        body->setJamDistance(
            AppMates::getGVManager().getNumeric("JAM_DISTANCE_TRUCK"));
    }
    else
    {
        body->setJamDistance(
            AppMates::getGVManager().getNumeric("JAM_DISTANCE_PASSENGER"));
    }

    body->setBodyColor(r, g, b);

    // [eMATES]
    if (AppMates::getGVManager().getFlag("FLAG_GEN_EV")
            && type.category() == VehicleCategory::EV)
    {
        VehicleEVTypeProperty* vtpEV = static_cast<VehicleEVTypeProperty*>(vtp);
        VehicleEVBodyProperty* bodyEV = static_cast<VehicleEVBodyProperty*>(body);

        double bodyWeight;
        double batteryCapacityWs;
        double frontalProjectedArea;
        double coeffDrag;
        double coeffRollingFriction;
        double mechanicalLoss;
        double finalGearRatio;

        vtpEV->getEVspec(
            &bodyWeight,
            &batteryCapacityWs,
            &frontalProjectedArea,
            &coeffDrag,
            &coeffRollingFriction,
            &mechanicalLoss,
            &finalGearRatio);
        bodyEV->setEVspec(
            bodyWeight,
            batteryCapacityWs,
            frontalProjectedArea,
            coeffDrag,
            coeffRollingFriction,
            mechanicalLoss,
            finalGearRatio);

        VehicleEV* ev = static_cast<VehicleEV*>(vehicle);
        // 初期SOCを設定する
        ev->setInitialSOC();
    }
}

//==============================================================================
void VehicleGenerator::_setRoutingParamsRandomly(Vehicle* vehicle)
{
    vehicle->setRoutingParams(
        _routingParams[_rng.uniform(0, _routingParams.size())]);
}

//==============================================================================
void VehicleGenerator::_setRoutingPrefRankRandomly(Vehicle* vehicle)
{
    unsigned int rank = 1;
    double       r    = _rng.uniform();
    for (auto itr : _routingPrefRank)
    {
        if (r <= itr.second)
        {
            rank = itr.first;
            break;
        }
        else
        {
            r -= itr.second;
        }
    }
    vehicle->setPreferredNetworkRank(rank);
}

//==============================================================================
void VehicleGenerator::_getStartAndGoalNodes(
    const GeneratingTableCell* cell, ODNode** result_start,
    ODNode** result_goal)
{
    string       startId    = cell->origin();
    string       fmtStartId = formatId(startId, NUM_FIGURE_FOR_OD_GROUP);
    ODNodeGroup* group      = _odPairGroup[fmtStartId];
    if (!group)
    {
        cerr << "ERROR: OD pair -" << startId << " not found." << endl;
    }
    else
    {
        group->getODPair(result_start, result_goal);
    }
}

//==============================================================================
ODNode* VehicleGenerator::_getStartNode(const GeneratingTableCell* cell)
{
    ODNode* start = nullptr;

    string startId = cell->origin();
    if (cell->hasPairedODs())
    {
        cerr << "ERROR: OD pair must be processed in other function." << endl;
        exit(EXIT_FAILURE);
    }
    else if (cell->hasGroupedOrigins())
    {
        // グループ化された出発地
        // Grouped origin
        string       fmtStartId = formatId(startId, NUM_FIGURE_FOR_OD_GROUP);
        ODNodeGroup* group      = _odStartGroup[fmtStartId];
        if (!group)
        {
            cerr << "ERROR: OriginGroup -" << startId << " not found." << endl;
        }
        else
        {
            group->getODSolo(&start);
        }
    }
    else
    {
        // 単一の始点
        // Single origin
        start = dynamic_cast<ODNode*>(_roadMap->intersection(startId));
    }

    return start;
}

//==============================================================================
ODNode* VehicleGenerator::_getGoalNode(
    const GeneratingTableCell* cell, ODNode* start)
{
    ODNode* goal = NULL;

    string goalId = cell->destination();
    if (goalId == "******")
    {
        goal = _decideGoalRandomly(start);
        return goal;
    }

    if (cell->hasPairedODs())
    {
        cerr << "ERROR: OD pair must be processed in other function." << endl;
        exit(EXIT_FAILURE);
    }
    else if (cell->hasGroupedDestinations())
    {
        // グループ化された目的地
        // Grouped destination
        string       fmtGoalId = formatId(goalId, NUM_FIGURE_FOR_OD_GROUP);
        ODNodeGroup* group     = _odGoalGroup[fmtGoalId];
        if (!group)
        {
            cerr << "ERROR: DestinationGroup-" << goalId << " not found."
                 << endl;
        }
        else
        {
            group->getODSolo(&goal);
        }
    }
    else
    {
        // 単独の目的地
        // Single destination
        goal = dynamic_cast<ODNode*>(_roadMap->intersection(goalId));
    }

    return goal;
}

//==============================================================================
ODNode* VehicleGenerator::_decideGoalRandomly(ODNode* start)
{
    ODNode* result = NULL;

    // _defaultTrafficVolumeにもとづくルーレット選択
    // Roulette selection based on _defaultTrafficVolume
    /**
     * @todo
     * _defaultTrafficVolumeがすべてゼロの場合はこのアルゴリズムは無効．
     * _rnd.uniform(0)で確実に落ちる．
     */
    int totalVolume = _totalDefaultGoalTrafficVolume;
    if (!_excludesGoalNode(start) && getGoalLevel(start) >= 0
        && getGoalLevel(start) < 3)
    {
        totalVolume -= _defaultTrafficVolume[getGoalLevel(start)];
    }

    unsigned int residual = _rng.uniform(totalVolume);
    for (auto itr : _roadMap->odNodes())
    {
        if (itr == start || _excludesGoalNode(itr) || getGoalLevel(itr) < 0
            || getGoalLevel(itr) >= 3)
        {
            continue;
        }
        if (residual <= static_cast<unsigned int>(
                _defaultTrafficVolume[getGoalLevel(itr)]))
        {
            result = itr;
            break;
        }
        residual -= _defaultTrafficVolume[getGoalLevel(itr)];
    }

    assert(result);
    return result;
}

//==============================================================================
void VehicleGenerator::_getGateIds(
    const GeneratingTableCell* cell, ODNode* start, ODNode* goal,
    vector<string>& result_gateIds)
{
    result_gateIds.clear();
    const vector<string> gates = cell->gates();

    /*
     * gatesの最初の要素は出発地をあらわす
     *   gatesの識別番号はグループIDである可能性があるため，決定した出発地の
     *   IDで書き換え
     *
     * The first component of gates is the origin
     *   Since the ID number of gates may be the group ID number, rewrite it
     *   with the ID number of the determined origin.
     */
    result_gateIds.push_back(start->id());

    for (unsigned int i = 1; i < gates.size() - 1; i++)
    {
        result_gateIds.push_back(gates[i]);
    }

    /*
     * gatesの最後の要素は目的地をあらわす
     *   gatesの識別番号はグループIDや"******"である可能性があるため，決定した
     *   目的地のIDで書き換え
     *
     * The last component of gates is the destination
     *   Since the ID number of gates may be the group ID number or "******",
     *   rewrite it with the ID number of the determined destination.
     */
    result_gateIds.push_back(goal->id());
}

//==============================================================================
void VehicleGenerator::_convertGateIds2Gates(
    const vector<string>& gateIds, vector<const Intersection*>& result_gates,
    ODNode* start, ODNode* goal)
{
    result_gates.clear();
    for (auto itr : gateIds)
    {
        Intersection* gate = _roadMap->intersection(itr);
        if (!gate)
        {
            cerr << "ERROR: gate id (" << itr << ") is not found." << endl;
        }
        result_gates.emplace_back(gate);
    }

    assert(result_gates[0] == start);
    assert(result_gates[result_gates.size() - 1] == goal);
}

//==============================================================================
void VehicleGenerator::printExcludedRandomStarts(ostream& out) const
{
    amu::msg::title(out, "Excluded Origin Nodes from Random Selection");
    if (_excludedStartNodes.size() == 0)
    {
        amu::msg::message(out, "none");
        return;
    }
    for (auto itr : _excludedStartNodes)
    {
        amu::msg::message(out, itr.first);
    }
    return;
}

//==============================================================================
void VehicleGenerator::printExcludedRandomGoals(ostream& out) const
{
    amu::msg::title(out, "Excluded Destination Nodes from Random Selection");
    if (_excludedGoalNodes.size() == 0)
    {
        amu::msg::message(out, "none");
        return;
    }
    for (auto itr : _excludedGoalNodes)
    {
        amu::msg::message(out, itr.first);
    }
    return;
}

//==============================================================================
void VehicleGenerator::printODGroup(ostream& out) const
{
    if (_odStartGroup.size() > 0 || _odGoalGroup.size() > 0
        || _odPairGroup.size() > 0)
    {
        amu::msg::title(out, "OD Group");
    }

    if (_odStartGroup.size() > 0)
    {
        amu::msg::message(out, "Start OD Group:");
        for (auto itr : _odStartGroup)
        {
            itr.second->print(out);
        }
    }
    if (_odGoalGroup.size() > 0)
    {
        amu::msg::message(out, "Goal OD Group:");
        for (auto itr : _odGoalGroup)
        {
            itr.second->print(out);
        }
    }
    if (_odPairGroup.size() > 0)
    {
        amu::msg::message(out, "Pair OD Group:");
        for (auto itr : _odPairGroup)
        {
            itr.second->print(out);
        }
    }
}

//==============================================================================
void VehicleGenerator::printVehicleRoutingParams(ostream& out) const
{
    amu::msg::title(out, "Vehicle Routing Parameter");
    ostringstream ss1;
    ss1 << "NumParams: " << _routingParams.size() << ", "
        << _routingParams[0].size();
    amu::msg::message(out, ss1.str());

    for (unsigned int i = 0; i < _routingParams.size(); i++)
    {
        ostringstream ss2;
        ss2 << i << ": ";
        for (auto itr : _routingParams[i])
        {
            ss2 << itr << ",";
        }
        amu::msg::message(out, ss2.str());
    }
}

//==============================================================================
void VehicleGenerator::printVehicleRoutingPrefRank(ostream& out) const
{
    amu::msg::title(out, "Vehicle Routing Preferred Network Rank");
    for (auto itr : _routingPrefRank)
    {
        ostringstream ss;
        ss << "Rank" << itr.first << ": " << itr.second;
        amu::msg::message(out, ss.str());
    }
}
