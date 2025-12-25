/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file ODNodeTramExt.cpp
 */
#ifdef INCLUDE_TRAMS
#include "ODNodeTramExt.hpp"
#include "SectionTramExt.hpp"
#include "VehicleTram.hpp"
#include "../AppMates.hpp"
#include "../Lane.hpp"
#include "../ObjectInLane.hpp"
#include "../ObjectManager.hpp"
#include "../ODNode.hpp"
#include "../RoadMap.hpp"
#include "../Section.hpp"
#include "../TimeManager.hpp"
#include "../Vehicle.hpp"
#include "../VehicleLocation.hpp"
#include "../io/RoadMapBuilder.hpp"
#include "../io/VehicleTypeWriter.hpp"
#include <cassert>

using namespace std;

//==============================================================================
void ODNodeTramExt::pushTramToRoadMap(
    RoadMap* roadMap, deque<Vehicle*>& waitingVehicles,
    vector<Vehicle*>& result_skippedTrams)
{
    vector<const Lane*> lanes;
    _odNode->nextSection(0)->tramExt()->getTramLanesFrom(lanes, _odNode);

    VehicleTram* tmpTram = dynamic_cast<VehicleTram*>(waitingVehicles.front());
    waitingVehicles.pop_front();
    assert(tmpTram);

    if (lanes[0]->tailAgent() == nullptr
        || lanes[0]->tailAgent()->distance()
                - lanes[0]->tailAgent()->bodyLength() / 2
                - tmpTram->bodyLength() / 2
            > 1.0)
    {
        _placeTramInTramLane(tmpTram, lanes[0], roadMap);

        /*
         * 車両属性を出力する
         *   識別番号は addVehicleToReal で決まるためここで出力
         *
         * Output vehicle property
         *   Output here because the ID number is determined by addVehicleToReal
         */
        VehicleTypeWriter writer;
        writer.writeVehicleProperty(tmpTram);

        // 路面電車は InflowMonitor の計測対象としない
        // Trams are not monitored by InflowMonitor.
    }
    else
    {
        result_skippedTrams.push_back(tmpTram);
    }
}

//==============================================================================
void ODNodeTramExt::_placeTramInTramLane(
    VehicleTram* tram, const Lane* lane, RoadMap* roadMap)
{
    bool result = AppMates::getObjectManager().addVehicleToRoadMap(tram);
    assert(result);

    Section* section = _odNode->nextSection(0);

    // RoadMap を登録
    // Regiser RoadMap
    tram->setRoadMap(roadMap);

    // 道路上に登場
    // Appear on the road
    tram->addToSection(
        roadMap, section, const_cast<Lane*>(lane), tram->bodyLength() / 2);

    // グローバル経路の指定
    // Set global route
    tram->setRoute();

    /*
     * ローカル経路を探索
     *   路面電車なので車線変更は考えないが，レーンの分岐に対応する必要がある．
     *
     * Search local route
     *   Changing lanes is not needed for tram, but it is necessary to deal with
     *   lane divergence.
     */
    tram->firstLocalReroute(
        section, const_cast<Lane*>(lane), tram->bodyLength() / 2);


    // 流入時刻を保存する
    // Save inflow time
    const_cast<VehicleLocation*>(tram->location())
        ->setStartingTime(AppMates::getTimeManager().time());
}

#endif //INCLUDE_TRAMS
