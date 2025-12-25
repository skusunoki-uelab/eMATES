/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file LinkFlowMonitorBuilder.cpp
 */
#include "LinkFlowMonitorBuilder.hpp"
#include "../AppMates.hpp"
#include "../Config.hpp"
#include "../CustomMessage.hpp"
#include "../GVManager.hpp"
#include "../LinkFlowMonitor.hpp"
#include "../Section.hpp"
#include <AmuConverter.hpp>
#include <AmuStringOperator.hpp>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace amu::converter;
using namespace amu::string_operator;

//==============================================================================
void LinkFlowMonitorBuilder::buildLinkFlowMonitors(RoadMap* roadMap)
{
    assert(roadMap);
    GVManager& gv    = AppMates::getGVManager();
    string     fname = gv.getString("LINK_FLOW_MONITOR_FILE");

    ostringstream ss;
    ss << "read link travel monitor file (" << gv.stripDataDir(fname)
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
        if (tokens.size() != 3)
        {
            ostringstream ssw;
            ssw << "invalid link travel monitor file format - " << line;
            amu::msg::warn(ssw.str());
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling
        string id           = formatId(tokens[0], NUM_FIGURE_FOR_MONITOR);
        string beginInterId = formatId(tokens[1], NUM_FIGURE_FOR_INTERSECTION);
        string endInterId   = formatId(tokens[2], NUM_FIGURE_FOR_INTERSECTION);

        // 指定されたSectionが存在するかチェックする
        // Check if the specified Section exists
        string sectionId;
        bool   isUp;
        if (beginInterId < endInterId)
        {
            sectionId = beginInterId + endInterId;
            isUp      = true;
        }
        else
        {
            sectionId = endInterId + beginInterId;
            isUp      = false;
        }

        Section* section = roadMap->section(sectionId);
        if (!section)
        {
            ostringstream ssw;
            ssw << "link travel monitor[" << id << "] - section[" << sectionId
                << "] does not exist.";
            amu::msg::warn(ssw.str());
            continue;
        }

        // リンク旅行時間観測器を生成しObjectManagerに追加する
        // Generate link travel time monitor and add it to ObjectManager
        LinkFlowMonitor* ptMonitor = new LinkFlowMonitor(id);
        ptMonitor->setSection(section);
        ptMonitor->setIsUp(isUp);
        section->setLinkFlowMonitor(isUp, ptMonitor);
        AppMates::getObjectManager().addLinkFlowMonitor(ptMonitor);
    }
    fin.close();

    ss << "done";
    amu::msg::status(cout, ss.str());
    return;
}
