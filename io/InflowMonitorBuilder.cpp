/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file InflowMonitorBuilder.cpp
 */
#include "InflowMonitorBuilder.hpp"
#include "../AppMates.hpp"
#include "../CustomMessage.hpp"
#include "../GVManager.hpp"
#include "../InflowMonitor.hpp"
#include "../Intersection.hpp"
#include "../ObjectManager.hpp"
#include "../ODNode.hpp"
#include <AmuConverter.hpp>
#include <AmuStringOperator.hpp>
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;
using namespace amu::converter;
using namespace amu::string_operator;

//==============================================================================
void InflowMonitorBuilder::buildInflowMonitors(RoadMap* roadMap)
{
    assert(roadMap);
    GVManager& gv    = AppMates::getGVManager();
    string     fname = gv.getString("INFLOW_MONITOR_FILE");

    ostringstream ss;
    ss << "read inflow monitor file (" << gv.stripDataDir(fname) << ") ... ";

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
        if (tokens.size() != 2)
        {
            ostringstream ssw;
            ssw << "invalid inflow monitor format - " << line;
            amu::msg::warn(ssw.str());
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling

        // 指定されたODNodeが存在するかチェックする
        // Check if the specified ODNode exists
        string  id       = formatId(tokens[0], NUM_FIGURE_FOR_MONITOR);
        string  odNodeId = formatId(tokens[1], NUM_FIGURE_FOR_INTERSECTION);
        ODNode* node = dynamic_cast<ODNode*>(roadMap->intersection(odNodeId));
        if (!node)
        {
            ostringstream ssw;
            ssw << "inflow monitor[" << id << "]: OD node[" << id
                << "] does not exist.";
            amu::msg::warn(ssw.str());
            continue;
        }

        // 流入車両検出器を生成しObjectManagerに追加する
        // Generate inflow vehicle monitor and add it to ObjectManager
        InflowMonitor* ptMonitor = new InflowMonitor(id);
        ptMonitor->setODNode(node);
        ptMonitor->setSection(node->nextSection(0));
        node->setInflowMonitor(ptMonitor);
        AppMates::getObjectManager().addInflowMonitor(ptMonitor);
    }
    fin.close();

    ss << "done";
    amu::msg::status(cout, ss.str());
    return;
}

