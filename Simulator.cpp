/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file Simulator.cpp
 */
#include "Simulator.hpp"
#include "AppMates.hpp"
#include "ClockerManager.hpp"
#include "Config.hpp"
#include "CustomMessage.hpp"
#include "GVManager.hpp"
#include "Intersection.hpp"
#include "Lane.hpp"
#include "LaneBundle.hpp"
#include "LoggerManager.hpp"
#include "ObjectManager.hpp"
#include "ODNode.hpp"
// 2025/10/14 Kusunoki Addition
#include "CSNodeBase.hpp"
#include "CSNodeFast.hpp"
#include "InflowMonitor.hpp"

#include "RouteCacheContainer.hpp"
#include "RouteCacheManager.hpp"
#include "RouterManager.hpp"
#include "ScheduleManager.hpp"
#include "Section.hpp"
#include "SubLaneBundle.hpp"
#include "TimeManager.hpp"
#include "Vehicle.hpp"
#include "VehicleGenerator.hpp"
#include "io/ConvoyMonitorBuilder.hpp"
#include "io/ConvoyMonitorWriter.hpp"
#include "io/EVCommIO.hpp" // [eMATES]
#include "io/InflowMonitorBuilder.hpp"
#include "io/InflowMonitorWriter.hpp"
#include "io/LinkFlowMonitorBuilder.hpp"
#include "io/LinkFlowMonitorWriter.hpp"
#include "io/Logger.hpp"
#include "io/RoadMapBuilder.hpp"
#include "io/RoadMapShapeWriter.hpp"
#include "io/RouteCacheReader.hpp"
#include "io/RouteCacheWriter.hpp"
#include "io/SignalTimeSeriesWriter.hpp"
#include "io/TrafficCounterBuilder.hpp"
#include "io/TrafficCounterWriter.hpp"
#include "io/VehicleGeneratorBuilder.hpp"
#include "io/VehicleTimeSeriesWriter.hpp"
#ifdef INCLUDE_PEDESTRIANS
#include "ped/InflowPedestrianMonitor.hpp"
#include "ped/InflowPedestrianMonitorBuilder.hpp"
#include "ped/InflowPedestrianMonitorWriter.hpp"
#include "ped/PedestrianTimeSeriesWriter.hpp"
#include "ped/Pedestrian.hpp"
#include "ped/PedestrianGenerator.hpp"
#endif //INCLUDE_PEDESTRIANS
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#ifdef _OPENMP
#include <omp.h>
#endif //_OPENMP

using namespace std;

//==============================================================================
Simulator::Simulator()
{
    _roadMap        = 0;
    _failsLaneCheck = false;

#ifdef INCLUDE_PEDESTRIANS
    _pedExt = new SimulatorPedExt(this);
#endif //INCLUDE_PEDESTRIANS
}

//==============================================================================
Simulator::~Simulator()
{
    if (_vehicleGenerator)
    {
        delete _vehicleGenerator;
    }

    if (_roadMap)
    {
        delete _roadMap;
    }

#ifdef INCLUDE_PEDESTRIANS
    delete _pedExt;
#endif //INCLUDE_PEDESTRIANS
}

//==============================================================================
bool Simulator::getReadyRoadEnvironment()
{
    RoadMapBuilder builder;

    // 地図オブジェクトの作成とレーン整合性チェック
    // Create road map object and check lane consistency
    _roadMap        = builder.buildRoadMap();
    _failsLaneCheck = !_roadMap->checkLaneConnectivity();

    // 交差点の属性 (通行権など) の設定
    // Set intersection attribute (right-of-way, etc.)
    builder.setIntersectionProperty();

    // 単路部の属性 (制限速度，通行権など) の設定
    // Set section attributes (speed limit, right-of-way, etc.)
    builder.setSectionProperty();

    // 信号の作成
    // Create traffic lights
    builder.buildSignals();

    // 経路探索用ネットワークの作成
    // Create network for routing
    AppMates::getRouterManager().setRoadMap(_roadMap);
    AppMates::getRouterManager().getReadyRoutingNetworks();
    AppMates::getRouteCacheManager().setRoadMap(_roadMap);

    // [eMATES] EV情報入出力用オブジェクトへの設定
    EVCommIO::instance().setRoadMap(_roadMap);

    // 経路探索ネットワークの初期コストの設定
    for (auto itr : _roadMap->intersections())
    {
        itr.second->initializeLinkFlowObservation();
    }
    AppMates::getRouterManager().setInitialCosts();
    AppMates::getRouterManager().renewCosts();

    // 経路探索結果の読み込み
    // Loading routing results
    if (AppMates::getGVManager().getFlag("FLAG_CACHE_ROUTING_READ"))
    {
        RouteCacheReader reader;
        reader.readRouteCache(_roadMap);
    }

    return true;
}

//==============================================================================
bool Simulator::getReadyMonitors()
{
    assert(_roadMap);

    ObjectManager& objManager = AppMates::getObjectManager();

    // 車両感知器
    // Traffic counters
    {
        TrafficCounterBuilder builder;
        builder.buildTrafficCounters(_roadMap);
        TrafficCounterWriter writer;
        for (auto itr : objManager.trafficCounters())
        {
            writer.prepareFiles(itr);
        }
    }

    // リンク旅行時間観測器
    // Link travel time monitors
    {
        LinkFlowMonitorBuilder builder;
        builder.buildLinkFlowMonitors(_roadMap);
        LinkFlowMonitorWriter writer;
        for (auto itr : objManager.linkFlowMonitors())
        {
            writer.prepareFile(itr);
        }
    }

    // 流入車両検出器
    // Inflow vehicle monitors
    {
        InflowMonitorBuilder builder;
        builder.buildInflowMonitors(_roadMap);
        InflowMonitorWriter writer;
        for (auto itr : objManager.inflowMonitors())
        {
            writer.prepareFile(itr);
        }
    }

    // 車列観測器
    // Vehicle convoy monitors
    {
        ConvoyMonitorBuilder builder;
        builder.buildConvoyMonitors(_roadMap);
        ConvoyMonitorWriter writer;
        for (auto itr : objManager.convoyMonitors())
        {
            writer.prepareFile(itr);
        }
    }

#ifdef INCLUDE_PEDESTRIANS
    // 流入歩行者検出器
    // Inflow pedestrian monitors
    {
        InflowPedestrianMonitorBuilder builder;
        builder.buildInflowPedestrianMonitors(_roadMap);
        InflowPedestrianMonitorWriter writer;
        for (auto itr : objManager.inflowPedestrianMonitors())
        {
            writer.prepareFile(itr);
        }
    }
#endif //INCLUDE_PEDESTRIANS

    return true;
}

//==============================================================================
bool Simulator::getReadyVehicles()
{
    assert(_roadMap);
    AppMates::getVehicleTypeManager().prepareVehicleType();

    VehicleGeneratorBuilder builder(_roadMap);
    _vehicleGenerator = builder.buildVehicleGenerator();

    /**
     * @todo 削除を検討
     */
    _vehicleGenerator->setSimulator(this);

    return true;
}

//==============================================================================
void Simulator::writeInitializedRoadMap()
{
    // mapInfo.txt
    if (_roadMap)
    {
        RoadMapShapeWriter writer;
        writer.writeRoadMapShape(_roadMap);
    }

    // signal_count.txt
    string fname
        = AppMates::getGVManager().getString("RESULT_SIGNAL_COUNT_FILE");
    ofstream fout(fname.c_str(), ios::out);

    /*
     * 交差点数と信号機の総数の出力
     *   信号機の総数と交差点数とは一致しないので注意．
     *
     * Output the number of intersections and total number of traffic lights
     *   Note that the total number of traffic lights and the number of
     *   intersections do not match.
     */
    int totalNumberOfSignals = 0;
    for (auto itr : _roadMap->signals())
    {
        totalNumberOfSignals += itr.second->intersection()->numNexts();
    }

    fout << _roadMap->signals().size() << endl << totalNumberOfSignals << endl;
    fout.close();
}

//==============================================================================
bool Simulator::run(ulint time)
{
    /*
     * レーンに不整合がある場合は実行を中断する．画面表示により確認するため，
     * 強制終了はおこなわない．
     *
     * Execution is interrupted if there is lane inconsistency. To confirm by
     * the screen display, not terminate forcibly.
     */
    if (failsLaneCheck())
    {
        return false;
    }

    if (time <= AppMates::getTimeManager().time())
    {
        return false;
    }

    // [eMATES] OpenDSS連成対応
    // std::functionにコールバックを登録することでwhile文内部での条件分岐を回避
    std::function<void(void)> writeEVComm;
    bool useEVComm = AppMates::getGVManager().getFlag("FLAG_EV_COMM");
    if (useEVComm)
    {
        writeEVComm = [this](){return _writeEVComm();};
    }
    else
    {
        writeEVComm = [](){}; // 何もしない
    }

    while (time > AppMates::getTimeManager().time())
    {
        incrementStep();
        // 2022/11/29 by uchida
            //2025/7/30 by komatsu
        // 待ち行列と利用時間(1分毎に出力)
          (AppMates::getTimeManager().time()%60000 == 0) //%60000 == 0
            {
            // 元のベクトルポインタを取得
            vector<CSNodeBase*> fcsNodes = _roadMap->csNodesFast();
            vector<CSNodeBase*> csNodes = _roadMap->csNodes();
            // --- csNodes に含まれ、fcsNodes には含まれないポインタのリストを作成 ---
            vector<CSNodeBase*> ncsNodes;
            for (CSNodeBase* csNode_ptr : csNodes) {
                bool found = false;
                for (CSNodeBase* fcsNode_ptr : fcsNodes) {
                    if (csNode_ptr == fcsNode_ptr) {
                        found = true;
                        break; // fcsNodes に見つかったので、内側のループを終了
                    }
                }
                if (!found) {
                    ncsNodes.push_back(csNode_ptr); // fcsNodes に見つからなかったので追加
                }
            }
            for (int i = 0; i < fcsNodes.size(); i++)
            {
                cout << "log: " << fcsNodes[i]->id() << ","
                    << (double)(fcsNodes[i]->occupancy())  / (double)(fcsNodes[i]->capacity() * AppMates::getTimeManager().time()) << ","
                    << (fcsNodes[i]->waitingVehicles()).size() << ","
                    << fcsNodes[i]->occupancy() * fcsNodes[i]->outPower() / (3600 * 1000.0) << ","
                    << fcsNodes[i]->servedEV() << endl;
                // cout << "occupancy: " << csNodes[i]->occupancy() << ","
                //     << "capacity: " << csNodes[i]->capacity() << ","
                //     << "TimeManager: " << TimeManager::time() <<","
                //     << "outPower: " << csNodes[i]->outPower() << endl; //DEBUG2024/08/16
                // if (csNodes[i]->id() == "900002") //cout << "line: 320"<< endl; //DEBUG2024/05/30
                // {
                //     const auto& cgr = csNodes[i]->chargingVehicles();
                //     for (auto vItr = cgr.begin(); vItr != cgr.end(); ++vItr)
                //     {
                //         if ((*vItr) != nullptr)
                //         {
                //             cout << "Vehicle registered: " << (*vItr)->id() << endl;
                //         }
                //         else
                //         {
                //             cout << "No vehicle registered in this charger" << endl;
                //         }
                //     }
                // }//DEBUG2024/05/30
            }
            for (int i = 0; i < ncsNodes.size(); i++) // by obinata 2024/10/17
            {
                cout << "log: " << ncsNodes[i]->id() << ","
                    << (double)(ncsNodes[i]->occupancy())  / (double)(ncsNodes[i]->capacity() * AppMates::getTimeManager().time()) << ","
                    << (ncsNodes[i]->waitingVehicles()).size() << ","
                    << ncsNodes[i]->occupancy() * ncsNodes[i]->outPower() / (3600 * 1000.0) << ","
                    << ncsNodes[i]->servedEV() << endl;
            }
            // for (int i = 0; i < csNodes.size(); i++)
            // {
            //     cout << "log: " << csNodes[i]->id() << ","
            //         << (double)(csNodes[i]->occupancy())  / (double)(csNodes[i]->capacity() * TimeManager::time()) << ","
            //         << csNodes[i]->waitingLineSize() << ","
            //         << csNodes[i]->occupancy() * csNodes[i]->outPower() / (3600 * 1000.0) << ","
            //         << csNodes[i]->servedEV() << endl;
            // }
            // // for (int j = 0; j < ncsNodes.size(); j++)
            // // {
            // //     cout << "log: " << ncsNodes[j]->id() << "," << ncsNodes[j]->occupancy() << "," << ncsNodes[j]->waitingLineSize() << endl;
            // // }
            }
        writeEVComm(); // [eMATES]
    }

    return true;
}

//==============================================================================
bool Simulator::incrementStep()
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 時刻の更新と表示
    // Update and display time
    AppMates::getTimeManager().increment();
    _printTime();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // エージェントの発生
    // Generate agents

    /*
     * 発生したエージェントは Lane::_tmpAgents に追加されるため，エージェント
     * 情報の更新より前でなければならない．
     *
     * Because the agent that generated is added to Lane::_tmpAgents, it must be
     * done before the update of agent information
     */
#ifdef INCLUDE_VEHICLES
    _startClock("GENERATE");
    _vehicleGenerator->generateVehicle();
    _stopClock("GENERATE");
#endif //INCLUDE_VEHICLES

#ifdef INCLUDE_PEDESTRIANS
    _startClock("GENERATE_PEDESTRIAN");
    _pedExt->generatePedestrian();
    _stopClock("GENERATE_PEDESTRIAN");
#endif //INCLUDE_PEDESTRIANS

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 環境が保持するエージェントの情報の更新
    // Update agent information retained by the environment
    _startClock("RENEW_AGENT");
    _roadMap->renewRetainedAgentInformation();
    _stopClock("RENEW_AGENT");

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // リンク交通流の観測
    // Observe link traffic flow
    _startClock("RENEW_PASSTIME");
    _observeLinkFlow();
    _stopClock("RENEW_PASSTIME");

    // 経路探索結果コンテナの更新
    // Update route search result containers
    _startClock("RENEW_ROUTE_STORAGE");
    AppMates::getRouteCacheManager().renewContainers();
    _stopClock("RENEW_ROUTE_STORAGE");

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // CSの推定待ち時間を更新 [eMATES]
    _roadMap->renewEstimatedWaitingTimeInCS();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 環境の更新
    // Update environment
    _startClock("RENEW_ENV");
    AppMates::getScheduleManager().activateEnvironmentItem();
    _stopClock("RENEW_ENV");

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 前ステップに流出したエージェントの消去
    // Delete agents flow out in the previous step

    /*
     * ObjectManager::_vehicles および Lane::_agents からエージェントを削除する
     * ため，前ステップの結果の出力より後で，さらに今ステップのエージェント
     * 情報の更新より後，かつ，エージェントの認知より前でなければならない．
     *
     * Because the agents are deleted from ObjectManager::_vehicles and
     * Lane::_agents, it must be done after the output of the result in the
     * previous step, after the update of the agent information in this step,
     * and before the recognition of agents in this step.
     */
    _startClock("DELETE_AGENT");
    _roadMap->deleteArrivedAgents();
    _stopClock("DELETE_AGENT");

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // エージェントの認知・判断・行動
    // Recognition, decision-making, action of agents
    auto bundles    = _roadMap->usedLaneBundles();
    int  bundleSize = bundles.size();
#ifdef INCLUDE_PEDESTRIANS
    auto pedestrians    = AppMates::getObjectManager().pedestrians();
    int  pedestrianSize = pedestrians.size();
#endif //INCLUDE_PEDESTRIANS

#pragma omp parallel
    {
        //--------------------------------------------------------------
        // 車両: 認知前に必要な処理
        // Vehicle: necessary processing before recognition
#ifdef INCLUDE_VEHICLES
#pragma omp master
        _startClock("PREPERCEIVE");
#pragma omp for schedule(guided)
        for (int i = 0; i < bundleSize; i++)
        {
            bundles[i]->makeAgentsPreperceive();
        }
#pragma omp master
        _stopClock("PREPERCEIVE");
#endif //INCLUDE_VEHICLES

        //--------------------------------------------------------------
        // 車両: 認知
        // Vehicle: recognition
#ifdef INCLUDE_VEHICLES
#pragma omp master
        _startClock("PERCEIVE");
#pragma omp for schedule(guided)
        for (int i = 0; i < bundleSize; i++)
        {
            bundles[i]->makeAgentsPerceive();
        }
#pragma omp master
        _stopClock("PERCEIVE");
#endif //INCLUDE_VEHICLES

        //--------------------------------------------------------------
        // 歩行者: 認知
        // Pedestrian: recognition
#ifdef INCLUDE_PEDESTRIANS
#pragma omp master
        _startClock("PERCEPT_PEDESTRIAN");
#pragma omp for schedule(guided)
        for (int i = 0; i < pedestrianSize; i++)
        {
            pedestrians[i]->perceive();
        }
#pragma omp master
        _stopClock("PERCEPT_PEDESTRIAN");
#endif //INCLUDE_PEDESTRIANS

        //--------------------------------------------------------------
        // 車両: 判断
        // Vehicle: decision-making
#ifdef INCLUDE_VEHICLES
#pragma omp master
        _startClock("MAKEDECISION");
#pragma omp for schedule(guided)
        for (int i = 0; i < bundleSize; i++)
        {
            bundles[i]->makeAgentsDetermine();
        }
#pragma omp master
        _stopClock("MAKEDECISION");
#endif //INCLUDE_VEHICLES

        //--------------------------------------------------------------
        // 歩行者: 判断
        // Pedestrian: decision-making
#ifdef INCLUDE_PEDESTRIANS
#pragma omp master
        _startClock("MAKEDECISION_PEDESTRIAN");
#pragma omp for schedule(guided)
        for (int i = 0; i < pedestrianSize; i++)
        {
            pedestrians[i]->determine();
        }
#pragma omp master
        _stopClock("MAKEDECISION_PEDESTRIAN");
#endif // INCLUDE_PEDESTRIANS

        //--------------------------------------------------------------
        // 車両: 行動
        // Vehicle: action
#ifdef INCLUDE_VEHICLES
#pragma omp master
        _startClock("ACT");
#pragma omp for schedule(guided)
        for (int i = 0; i < bundleSize; i++)
        {
            bundles[i]->makeAgentsAct();
        }
#pragma omp master
        _stopClock("ACT");
#endif //INCLUDE_VEHICLES

        //--------------------------------------------------------------
        // 歩行者: 行動
        // Pedestrian: action
#ifdef INCLUDE_PEDESTRIANS
#pragma omp master
        _startClock("WALK_PEDESTRIAN");
#pragma omp for schedule(guided)
        for (int i = 0; i < pedestrianSize; i++)
        {
            pedestrians[i]->act();
        }
#pragma omp master
        _stopClock("WALK_PEDESTRIAN");
#endif //INCLUDE_PEDESTRIANS

        //--------------------------------------------------------------
        // 車両: 行動後に必要な処理
        // Vehicle: necessary processing after action
#ifdef INCLUDE_VEHICLES
#pragma omp master
        _startClock("POSTACT");
#pragma omp for schedule(guided)
        for (int i = 0; i < bundleSize; i++)
        {
            bundles[i]->makeAgentsPostact();
        }
#pragma omp master
        _stopClock("POSTACT");
#endif //INCLUDE_VEHICLES
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // モニタリング
    // Monitoring
    _startClock("MONITORING");
    writeMonitorResult();
    _stopClock("MONITORING");

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 時系列データとログの出力
    // Time series data and log output
    _startClock("WRITE_RESULT");
    writeRunInfo();
    writeStepResult();
    AppMates::getLoggerManager().writeAllLogs();
    _stopClock("WRITE_RESULT");

    return true;
}

//==============================================================================
void Simulator::_printTime() const
{
    if (AppMates::getGVManager().getFlag("FLAG_VERBOSE")
        && AppMates::getTimeManager().time() % 1000 == 0)
    {
        cout << "Time: " << AppMates::getTimeManager().time() / 1000 << " [s]"
             << endl;
    }
}

//==============================================================================
void Simulator::_startClock(const string& clockerName)
{
#ifdef MEASURE_TIME
    AppMates::getClockerManager().startClock(clockerName);
#endif //MEASURE_TIME
}

//==============================================================================
void Simulator::_stopClock(const string& clockerName)
{
#ifdef MEASURE_TIME
    AppMates::getClockerManager().stopClock(clockerName);
#endif //MEASURE_TIME
}

//==============================================================================
void Simulator::_observeLinkFlow()
{
#ifdef INCLUDE_VEHICLES
    if (AppMates::getTimeManager().time()
            % (static_cast<int>(
                AppMates::getGVManager().getNumeric("INTERVAL_RENEW_LINK_FLOW")
                * 1000))
        == 0)
    {
        for (auto itr : _roadMap->unsignalizedIntersections())
        {
            itr->observeLinkFlow();
            itr->routeCacheContainer()->removeDynamicComponent();
        }
    }

    /*
     * 経路探索用ネットワークのコストの更新
     *   上記リンク旅行時間の更新後でなければならない
     *
     * Update network costs for routing
     *   Must be after update link travel times above
     */
    AppMates::getRouterManager().renewCosts();
#endif //INCLUDE_VEHICLES
}

//==============================================================================
void Simulator::_writeEVComm() // [eMATES]
{
    ulint inputInterval = AppMates::getGVManager().getNumeric("EV_COMM_INPUT_INTERVAL");
    ulint outputInterval = AppMates::getGVManager().getNumeric("EV_COMM_OUTPUT_INTERVAL");
    std::vector<Vehicle*>& vehicles = AppMates::getObjectManager().vehicles();
    EVCommIO& io = EVCommIO::instance();

    // by abe 2022/2/28
    // 指定秒数ごとにT******.csvとarrivelistの出力
    if (AppMates::getTimeManager().time() % outputInterval == 0)
    {
        bool success = true
            && io.writeCSTrafficData()
            && io.writeCSArriveList(&vehicles);
        if (!success)
        {
            io.writeErrorAndAbort();
        }
    }

    // 2022/2/18 added by uchida
    // 指定秒数ごとにE******.csvとerror.csvの存在確認・入力
    if (AppMates::getTimeManager().time() % inputInterval == 0)
    {
        bool success = io.waitAndReadOpenDSS(&vehicles);
        if (!success)
        {
            io.writeErrorAndAbort();
        }
    }
}

//==============================================================================
void Simulator::writeRunInfo() const
{
    string   fname = AppMates::getGVManager().getString("RESULT_RUN_INFO_FILE");
    ofstream fout(fname.c_str(), ios::trunc);
    if (fout)
    {
        fout << AppMates::getTimeManager().step() << endl
             << AppMates::getTimeManager().unit() << endl;
        fout.close();
    }
}

//==============================================================================
void Simulator::writeMonitorResult()
{
    GVManager&     gv         = AppMates::getGVManager();
    ObjectManager& objManager = AppMates::getObjectManager();

    bool outputsDetailed   = gv.getFlag("FLAG_OUTPUT_TRAFFIC_COUNTER_D");
    bool outputsAggregated = gv.getFlag("FLAG_OUTPUT_TRAFFIC_COUNTER_S");
    if (outputsDetailed || outputsAggregated)
    {
        TrafficCounterWriter writer;
        for (auto itr : objManager.trafficCounters())
        {
            itr->aggregateRecords();
            writer.writeRecords(itr, outputsDetailed, outputsAggregated);
        }
    }

    outputsDetailed   = gv.getFlag("FLAG_OUTPUT_LINK_FLOW_MONITOR_D");
    outputsAggregated = gv.getFlag("FLAG_OUTPUT_LINK_FLOW_MONITOR_S");
    if (outputsDetailed || outputsAggregated)
    {
        LinkFlowMonitorWriter writer;
        for (auto itr : objManager.linkFlowMonitors())
        {
            writer.writeRecord(itr, outputsDetailed, outputsAggregated);
        }
    }

    if (gv.getFlag("FLAG_OUTPUT_INFLOW_MONITOR"))
    {
        InflowMonitorWriter writer;
        for (auto itr : objManager.inflowMonitors())
        {
            writer.writeRecord(itr);
        }
    }

    if (gv.getFlag("FLAG_OUTPUT_CONVOY_MONITOR"))
    {
        ConvoyMonitorWriter writer;
        for (auto itr : objManager.convoyMonitors())
        {
            itr->monitorLanes();
            writer.writeRecord(itr);
        }
    }

#ifdef INCLUDE_PEDESTRIANS
    if (gv.getFlag("FLAG_OUTPUT_INFLOW_MONITOR"))
    {
        InflowPedestrianMonitorWriter writer;
        for (auto itr : objManager.inflowPedestrianMonitors())
        {
            writer.writeRecord(itr);
        }
    }
#endif //INCLUDE_PEDESTRIANS
}

//==============================================================================
void Simulator::writeStepResult() const
{
    GVManager& gv = AppMates::getGVManager();

    if (gv.getFlag("FLAG_OUTPUT_TIMELINE_VEHICLE_D")
        || gv.getFlag("FLAG_OUTPUT_TIMELINE_S"))
    {
        VehicleTimeSeriesWriter writer;
        writer.writeVehiclesData();
    }
#ifdef INCLUDE_PEDESTRIANS
    if (gv.getFlag("FLAG_OUTPUT_TIMELINE_PEDESTRIAN_D")
        || gv.getFlag("FLAG_OUTPUT_TIMELINE_S"))
    {
        PedestrianTimeSeriesWriter writer;
        writer.writePedestriansData();
    }
#endif //INCLUDE_PEDESTRIANS
    if (gv.getFlag("FLAG_OUTPUT_TIMELINE_SIGNAL_D"))
    {
        SignalTimeSeriesWriter writer;
        writer.writeSignalsData(_roadMap);
    }
}

//==============================================================================
void Simulator::writeResultAtExit() const
{
    if (AppMates::getGVManager().getFlag("FLAG_CACHE_ROUTING_WRITE"))
    {
        RouteCacheWriter writer;
        writer.writeRouteCache(_roadMap);
    }
}

//==============================================================================
bool Simulator::failsLaneCheck() const
{
    if (_failsLaneCheck)
    {
        ostringstream ss;
        ss << "lane connectivity error occurred" << " - cannot run simulation";
        amu::msg::warn(ss.str());
        return true;
    }
    return false;
}
