/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file TrafficCounter.cpp
 */
#include "TrafficCounter.hpp"
#include "AppMates.hpp"
#include "Config.hpp"
#include "CustomMessage.hpp"
#include "Lane.hpp"
#include "LaneBundle.hpp"
#include "Section.hpp"
#include "Vehicle.hpp"
#include "VehicleBodyProperty.hpp"
#include "VehicleTypeManager.hpp"
#include <cassert>
#include <map>

using namespace std;

//=====================================================================
void TrafficCounter::AggregatedRecord::clear()
{
    _beginTime     = AppMates::getTimeManager().time();
    _sumPassengers = 0;
    for (int& n : _numPassengers)
    {
        n = 0;
    }
    _sumTrucks = 0;
    for (int& n : _numTrucks)
    {
        n = 0;
    }
}

//======================================================================
void TrafficCounter::setPosition(
    Section* section, bool isUp, double distance, ulint interval)
{
    _section  = section;
    _isUp     = isUp;
    _distance = distance;
    _interval = interval;

    // 観測対象となるレーンを取得する
    // Get lanes to be observed
    for (auto itr : section->lanes())
    {
        bool   isTargetLane   = false;
        double distanceOnLane = 0;

        // レーンの方向が一致しなければ観測しない
        // Not observed if the direction of the lane does not match
        if (section->isUp(itr.second) != isUp)
        {
            continue;
        }

        if (distance >= 0)
        {
            /*
             * 正の場合は上流交差点からの距離
             *   始点からsectionの上流端までの距離がdistanceより小さく，
             *   終点から上流端までの距離がdistanceより大きい車線が対象
             *
             * Distance from upstream intersection if positive
             *   Lanes whose distance from its starting point to the
             *   upstream end of the section is less than distance
             *   and whose distance from its end to the upstream
             *   end is greater than distance are targeted.
             */
            if (section->distanceFromPrevious(itr.second, 0) < distance
                && section->distanceFromPrevious(
                       itr.second, itr.second->length())
                       > distance)
            {
                isTargetLane = true;
                distanceOnLane
                    = distance
                      - section->distanceFromPrevious(itr.second, 0);
            }
        }
        else
        {
            /*
             * 負の場合は下流交差点からの距離
             *   始点からsectionの下流端までの距離が-distanceより大きく，
             *   終点から下流までの距離が-distanceより小さい車線が対象
             *
             * Distance from downstream intersection if negative
             *   Lanes whose distance from its starting point to the
             *   upstream end of the section is greater than -distance
             *   and whose distance from its end to the downstream
             *   end is less than -distance are targeted.
             */
            if (section->distanceToNext(itr.second, 0) > -distance
                && section->distanceToNext(
                       itr.second, itr.second->length())
                       < -distance)
            {
                isTargetLane = true;
                distanceOnLane
                    = section->distanceToNext(itr.second, 0) + distance;
            }
        }

        if (isTargetLane)
        {
            // TrafficCounterComponentの生成
            // Generate TrafficCounterComponent
            TrafficCounterComponent* tmpComponent
                = new TrafficCounterComponent(this);
            tmpComponent->setPosition(itr.second, distanceOnLane);
            _components.emplace_back(tmpComponent);
            itr.second->addTrafficCounter(distanceOnLane, tmpComponent);
            _lanes.emplace_back(itr.second);
        }
    }

    // 集計データ格納用コンテナ準備する
    // Prepare aggregated result container
    _aggregatedRecord.resize(_components.size());
}

//======================================================================
void TrafficCounter::aggregateRecords()
{
    for (unsigned int i = 0; i < _components.size(); i++)
    {
        /*
         * 個々のTrafficCounterComponentによる観測結果の集約
         *
         * Aggregation of monitoring results by individual
         * TrafficCounterComponents
         */
        for (auto itr : _components[i]->records())
        {
            if (itr->vehicleType().category() == VehicleCategory::TRUCK
                || itr->vehicleType().category()
                       == VehicleCategory::BUS)
            {
                // 大型車
                _aggregatedRecord.incrementNumTrucks(i);
            }
            else
            {
                // 乗用車
                _aggregatedRecord.incrementNumPassengers(i);
            }
        }
    }
}

//======================================================================
void TrafficCounter::print(ostream& out) const
{
    ostringstream ss;
    ss << _id << ": " << "section[" << _section->id() << "], interval("
       << _interval << ")";
    amu::msg::message(out, ss.str());
    for (unsigned int i = 0; i < _components.size(); i++)
    {
        ostringstream ssc;
        ssc << "  " << i << ": lane[" << _components[i]->lane()->id()
            << "], distance(" << _components[i]->distance() << ")";
        amu::msg::message(out, ssc.str());
    }
}
