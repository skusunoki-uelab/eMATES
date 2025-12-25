/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file InflowMonitorBuilder.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "InflowPedestrianMonitorBuilder.hpp"
#include "InflowPedestrianMonitor.hpp"
#include "Zebra.hpp"
#include "ZebraODEdge.hpp"
#include "../AppMates.hpp"
#include "../CustomMessage.hpp"
#include "../GVManager.hpp"
#include "../Intersection.hpp"
#include <AmuConverter.hpp>
#include <AmuStringOperator.hpp>
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace amu::converter;
using namespace amu::string_operator;

//==============================================================================
void InflowPedestrianMonitorBuilder::buildInflowPedestrianMonitors(
    RoadMap* roadMap)
{
    assert(roadMap);
    GVManager& gv    = AppMates::getGVManager();
    string     fname = gv.getString("PEDESTRIAN_INFLOW_MONITOR_FILE");

    ostringstream ss;
    ss << "read inflow pedestrian monitor file (" << gv.stripDataDir(fname)
       << ") ... ";

    // ファイルを読み込む
    // Load file
    ifstream fin(fname.c_str(), ios::in);
    if (!fin)
    {
        ss << "not found";
        amu::msg::status(cout, ss.str());
        return;
    }
    amu::msg::status(cout, ss.str());

    while (fin.good())
    {
        string         line;
        vector<string> tokens;
        if (!getTokens(&fin, &line, &tokens, ','))
        {
            break;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 不正な行の処理
        // Invalid line handling
        if (tokens.size() != 4)
        {
            ostringstream ssw;
            ssw << "invalid inflow pedestrian monitor format - " << line;
            amu::msg::warn(ssw.str());
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling
        string id          = formatId(tokens[0], NUM_FIGURE_FOR_MONITOR);
        string interId     = formatId(tokens[1], NUM_FIGURE_FOR_INTERSECTION);
        int    dir         = stoi(tokens[2]);
        int    crossingDir = stoi(tokens[3]);

        // 指定された横断歩道が存在するかチェックする
        // Check if the specified crosswalk exists
        Intersection* inter = roadMap->intersection(interId);
        if (!inter)
        {
            ostringstream ssw;
            ssw << "inflow pedestrian monitor[" << id << "]: intersection["
                << interId << "] does not exist.";
            amu::msg::warn(ssw.str());
            continue;
        }
        Zebra* zebra = inter->pedExt()->zebra(dir);
        if (!zebra)
        {
            ostringstream ssw;
            ssw << "inflow pedestrian monitor[" << id
                << "]: zebra is not installed in dir(" << dir
                << ") at intersection[" << interId << "].";
            amu::msg::warn(ssw.str());
            continue;
        }
        if (crossingDir != 0 && crossingDir != 1)
        {
            ostringstream ssw;
            ssw << "inflow pedestrian monitor[" << id << "]: cross_dir("
                << crossingDir << ") must be 0 or 1.";
            amu::msg::warn(ssw.str());
            continue;
        }
        ZebraODEdge* edge = zebra->beginEdge(crossingDir);
        assert(edge);

        /*
         * 流入歩行者検出器を生成しObjectManagerに追加する
         *
         * Generate inflow pedestrian monitor and add it to
         * ObjectManager
         */
        InflowPedestrianMonitor* ptMonitor = new InflowPedestrianMonitor(id);
        ptMonitor->setZebraODEdge(edge);
        edge->setInflowMonitor(ptMonitor);
        AppMates::getObjectManager().addInflowPedestrianMonitor(ptMonitor);
    }
    fin.close();

    ss << "done";
    amu::msg::status(cout, ss.str());
    return;
}

#endif //INCLUDE_PEDESTRIANS
