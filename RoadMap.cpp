/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RoadMap.cpp
 */
#include "RoadMap.hpp"
#include "AppMates.hpp"
#include "CSNodeFast.hpp"
#include "CSNodeNormal.hpp"
#include "CustomMessage.hpp"
#include "GVManager.hpp"
#include "Intersection.hpp"
#include "LaneBundle.hpp"
#include "ObjectManager.hpp"
#include "ODNode.hpp"
#include "Section.hpp"
#include "Signal.hpp"
#include "SubLaneBundle.hpp"
#include "VehicleEV.hpp"
#include "io/VehicleTripWriter.hpp"
#include <AmuPoint.hpp>
#include <cassert>
#include <fstream>
#include <iostream>
#include <typeinfo>

using namespace std;
using namespace amu::geometry;

//======================================================================
RoadMap::RoadMap()
{
    _intersections.clear();
    _sections.clear();
    _odNodes.clear();
    _usedLaneBundles.clear();
    _usedODNodes.clear();
    _signals.clear();
#ifdef _OPENMP
    omp_init_lock(&_lock);
#endif //_OPENMP
}

//======================================================================
RoadMap::~RoadMap()
{
    for (auto itr : _intersections)
    {
        delete itr.second;
    }
    _intersections.clear();

    for (auto itr : _sections)
    {
        delete itr.second;
    }
    _sections.clear();

    for (auto itr : _signals)
    {
        delete itr.second;
    }
    _signals.clear();

#ifdef _OPENMP
    omp_init_lock(&_lock);
#endif //_OPENMP
}

//======================================================================
void RoadMap::addIntersection(Intersection* ptInter)
{
    // 同一の識別番号がないか検査し，重複がなければ追加する
    // Check for the same ID number and add if there is no duplication
    string id = ptInter->id();
    if (_intersections.find(id) == _intersections.end())
    {
        _intersections.insert(make_pair(id, ptInter));
        _laneBundles.emplace_back(ptInter);
    }
    else
    {
        cerr << "WARNING: Intersection[" << id << "] is duplicated." << endl;
    }
    ODNode* odNode = dynamic_cast<ODNode*>(ptInter);
    if (odNode)
    {
        _odNodes.emplace_back(odNode);
    }
    // [eMATES]
    CSNodeBase* csNode = dynamic_cast<CSNodeBase*>(ptInter);
    if (csNode)
    {
        _csNodes.emplace_back(csNode);

        CSNodeFast* csNodeFast = dynamic_cast<CSNodeFast*>(ptInter);
        if (csNodeFast)
        {
            _csNodesFast.emplace_back(csNode);
        }
    }
}

//======================================================================
void RoadMap::addSection(Section* ptSection)
{
    // 同一の識別番号がないか検査し，重複がなければ追加する
    // Check for the same ID number and add if there is no duplication
    string id = ptSection->id();
    if (_sections.find(id) == _sections.end())
    {
        _sections.insert(make_pair(id, ptSection));
        _laneBundles.emplace_back(ptSection);
    }
    else
    {
        cerr << "WARNING: Section[" << id << "] is duplicated." << endl;
    }
}

//======================================================================
void RoadMap::addSignal(Signal* ptSignal)
{
    // 同一の識別番号がないか検査し，重複がなければ追加する
    // Check for the same ID number and add if there is no duplication
    string id = ptSignal->id();
    if (_signals.find(id) == _signals.end())
    {
        _signals[id] = ptSignal;
    }
    else
    {
        cout << "WARNING: Signal[" << id << "] is duplicated." << endl;
    }
}

//======================================================================
void RoadMap::addUnsignalizedIntersection(Intersection* ptInter)
{
    _unsignalizedIntersections.emplace_back(ptInter);
}

//======================================================================
bool RoadMap::hasUnsignalizedIntersection(Intersection* ptInter) const
{
    auto itr = find(
        _unsignalizedIntersections.begin(), //
        _unsignalizedIntersections.end(),   //
        ptInter);
    if (itr == _unsignalizedIntersections.end())
    {
        return false;
    }
    return true;
}

//======================================================================
void RoadMap::renewEstimatedWaitingTimeInCS()
{
    for (CSNodeBase* cs : _csNodesFast)
    {
        cs->renewEstimatedWaitingTime();
    }
}

//======================================================================
void RoadMap::renewRetainedAgentInformation()
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 自動車の処理
    // Car processing
#ifdef INCLUDE_VEHICLES
    /*
     * 各レーン束オブジェクトに含まれる _agents をクリアする必要がある
     * ため， renewAgentOrder はすべてのレーン束オブジェクトで実行する．
     *
     * renewAgentOrder runs on all lane bundle objects since it is
     * needed to clear _agents included each lane bundle object.
     */
    unsigned int bundleSize = _laneBundles.size();

#pragma omp parallel for schedule(guided)
    for (unsigned int i = 0; i < bundleSize; i++)
    {
        _laneBundles[i]->renewAgentOrder();
    }

    /*
     * _usedLaneBundles の更新
     *   同時書き込みを避けるためシングルスレッドで実行する
     *
     * Update _usedLaneBundles
     *   Run in single thread to avoid concurrent writes
     */
    _usedLaneBundles.clear();
    for (auto itr : _laneBundles)
    {
        if (itr->isUsed())
        {
            _usedLaneBundles.push_back(itr);
        }
        if (typeid(*itr) == typeid(ODNode)
            || typeid(*itr) == typeid(CSNodeFast) // [eMATES]
            || typeid(*itr) == typeid(CSNodeNormal)) // [eMATES]
        {
            _usedODNodes.push_back(dynamic_cast<ODNode*>(itr));
        }
    }
#endif //INCLUDE_VEHICLES

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 歩行者シミュレーション用の処理
    // Processing for pedestrian simulation
#ifdef INCLUDE_PEDESTRIANS
    for (auto itr : _intersections)
    {
        IntersectionPedExt* inter
            = const_cast<IntersectionPedExt*>(itr.second->pedExt());
        inter->renewPedestrianOrder();
        inter->renewOncomingVehicleOrder();
        inter->notifyLaneOfApproachingPedestrian();
    }
#endif //INCLUDE_PEDESTRIANS
}

//======================================================================
void RoadMap::deleteArrivedAgents()
{
    for (auto itr : _usedODNodes)
    {
        itr->deleteAgent();
    }
    _usedODNodes.clear();

#ifdef INCLUDE_PEDESTRIANS
    for (auto itr : _intersections)
    {
        const_cast<IntersectionPedExt*>(itr.second->pedExt())
            ->deleteFinishedPedestrians();
    }
#endif //INCLUDE_PEDESTRIANS
}

//======================================================================
void RoadMap::addStrandedAgent(VehicleEV* vehicle)
{
    _strandedAgents.push_back(vehicle);
}

//======================================================================
void RoadMap::deleteStrandedAgents()
{
    // 充電切れのエージェントを消去する
    for (VehicleEV* strandedAgent : _strandedAgents)
    {
        if (AppMates::getGVManager().getFlag("FLAG_OUTPUT_TRIP_INFO"))
        {
            // 消去する前にトリップ長の出力
            VehicleTripWriter writer;
            writer.writeVehicleTripEV(strandedAgent);
        }

        cout << "strandedAgent is deleted: (id)" << strandedAgent->id()
             << " " << strandedAgent->location()->tripLength()
             << " " << strandedAgent->globalRoute()->start()->id()
             << " " << strandedAgent->globalRoute()->goal()->id()
             << " ";

        if (strandedAgent->location()->section())
        {
            cout << strandedAgent->location()->section()->id();
        }
        else
        {
            cout << "*";
        }

        cout << " ";

        if (strandedAgent->location()->intersection())
        {
            cout << strandedAgent->location()->intersection()->id();
        }
        else
        {
            cout << "*";
        }

        if (strandedAgent->targetCS())
        {
            cout << " " << strandedAgent->targetCS()->id();
        }

        cout << endl;

        // Laneの_agentsからも削除しておく
        // NOTE by abe 2025/04/04
        // MATESのバージョンアップに伴い、任意の車両をlaneから削除する関数がなくなったため、
        // _agentsを直接操作する
        auto& agents = const_cast<Lane*>(strandedAgent->location()->lane())->agents();
        auto itr = find(agents.begin(), agents.end(), strandedAgent);
        assert(itr != agents.end());
        agents.erase(itr);

        // ここでObjectManagerから消去される
        AppMates::getObjectManager().deleteVehicle(strandedAgent);
    }
    _strandedAgents.clear();
}

bool RoadMap::checkLaneConnectivity() const
{
    bool result = true;

    ostringstream ss;
    ss << "check lane connectivity ... ";
    amu::msg::status(cout, ss.str());

    for (auto itr : _intersections)
    {
        result = itr.second->checkLaneConnectivity(true) && result;
    }
    for (auto itr : _sections)
    {
        result = itr.second->checkLaneConnectivity(false) && result;
    }

    ss << "done";
    amu::msg::status(cout, ss.str());
    return result;
}

//======================================================================
void RoadMap::getRegion(
    double& result_xmin, double& result_xmax, double& result_ymin,
    double& result_ymax) const
{
    result_xmin = DBL_MAX;
    result_xmax = -DBL_MAX;
    result_ymin = DBL_MAX;
    result_ymax = -DBL_MAX;

    for (auto itr : _intersections)
    {
        AmuPoint c  = itr.second->center();
        result_xmin = min(result_xmin, c.x());
        result_xmax = max(result_xmax, c.x());
        result_ymin = min(result_ymin, c.y());
        result_ymax = max(result_ymax, c.y());
    }
}

//======================================================================
void RoadMap::printMapSimple(ostream& out) const
{
    amu::msg::title(out, "RoadMap");
    ostringstream ss;
    ss << "Intersections : " << _intersections.size() << endl
       << "(unsignalized : " << _unsignalizedIntersections.size() << ")" << endl
       << "(odNodes      : " << _odNodes.size() << ")" << endl
       << "Sections      : " << _sections.size();
    amu::msg::message(out, ss.str());
}

//======================================================================
void RoadMap::printMapDetail(ostream& out) const
{
    amu::msg::title(out, "Intersection Connection");

    // 表示のためにソートする
    // Sort for display
    map<string, Intersection*> sorted(
        _intersections.begin(), _intersections.end());
    for (auto itr : sorted)
    {
        itr.second->printMapInfo(out);
    }
}
