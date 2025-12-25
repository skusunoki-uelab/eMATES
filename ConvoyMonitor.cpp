/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file ConvoyMonitor.cpp
 */
#include "ConvoyMonitor.hpp"
#include "CustomMessage.hpp"
#include "Intersection.hpp"
#include "Lane.hpp"
#include "ObjectInLane.hpp"
#include "Section.hpp"
#include <algorithm>
#include <sstream>

using namespace std;

//==============================================================================
void ConvoyMonitor::addMonitoredLane(Lane* lane)
{
    // duplication check
    if (find(_lanes.begin(), _lanes.end(), lane) != _lanes.end())
    {
        ostringstream ssw;
        ssw << "Lane[" << lane->id()
            << "] is already registered to ConvoyMonitor[" << _id << "]."
            << endl;
        amu::msg::warn(ssw.str());
        return;
    }
    _lanes.emplace_back(lane);

    // 観測結果のコンテナを用意する
    // Prepare a container for measured result
    ConvoyMonitor::Record* record = new ConvoyMonitor::Record(lane->id());
    _records.insert(make_pair(lane->id(), record));
}

//==============================================================================
void ConvoyMonitor::monitorLanes()
{
    for (auto itr_l : _lanes)
    {
        bool isStopVehicleFound = false;
        for (auto itr_a : itr_l->agents())
        {
            /*
             * 始点に近い順に並んでいるため，最後尾の停車車両とは agentsの
             * 先頭から走査して最初に速度が0になったもの
             *
             * Since they are lined up in the order closest to starting point,
             * the rearmost stopped vehicle is the first one whose speed is 0
             * when scanning from the beginning of agents.
             */
            if (itr_a->velocity() < 1.0e-6)
            {
                _records[itr_l->id()]->setDistance(
                    _section->distanceToNext(itr_l, itr_a->distance()));
                isStopVehicleFound = true;
                break;
            }
        }
        if (!isStopVehicleFound)
        {
            _records[itr_l->id()]->setDistance(0.0);
        }
    }
}

//==============================================================================
void ConvoyMonitor::print(ostream& out) const
{
    ostringstream oss;
    oss << _id << ": section[" << _section->id() << "], is_up("
        << (_isUp ? "1" : "0") << "), intersections["
        << _section->intersection(!_isUp)->id() << "->"
        << _section->intersection(_isUp)->id() << "]";
    amu::msg::message(out, oss.str());
}
