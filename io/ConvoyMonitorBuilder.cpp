/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file ConvoyMonitorBuilder.cpp
 */
#include "ConvoyMonitorBuilder.hpp"
#include "../AppMates.hpp"
#include "../ConvoyMonitor.hpp"
#include "../CustomMessage.hpp"
#include "../GVManager.hpp"
#include "../Section.hpp"
#ifdef INCLUDE_TRAMS
#include "../tram/TramLaneInSection.hpp"
#endif //INCLUDE_TRAMS
#include <AmuConverter.hpp>
#include <AmuStringOperator.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
using namespace amu::converter;
using namespace amu::string_operator;

//======================================================================
void ConvoyMonitorBuilder::buildConvoyMonitors(RoadMap* roadMap)
{
    assert(roadMap);
    const GVManager& gv    = AppMates::getGVManager();
    string           fname = gv.getString("CONVOY_MONITOR_FILE");

    ostringstream ss;
    ss << "read convoy monitor file (" << gv.stripDataDir(fname) << ") ... ";

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

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 不正な行の処理
        // Invalid line handling
        if (tokens.size() != 3)
        {
            ostringstream ssw;
            ssw << "invalid convoy monitor file format - " << line;
            amu::msg::warn(ssw.str());
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling
        string id           = formatId(tokens[0], NUM_FIGURE_FOR_MONITOR);
        string beginInterId = formatId(tokens[1], NUM_FIGURE_FOR_INTERSECTION);
        string endInterId   = formatId(tokens[2], NUM_FIGURE_FOR_INTERSECTION);

        // 指定されたsectionが存在するかチェックする
        // Check if the specified section exists
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
            ssw << "convoy monitor[" << id << "] - section[" << sectionId
                << "] does not exist.";
            amu::msg::warn(ssw.str());
            continue;
        }

        // 車列観測器を生成しObjectManagerに追加する
        // Generate vehicle convoy monitor and add it to ObjectManager
        ConvoyMonitor* ptMonitor = new ConvoyMonitor(id);
        ptMonitor->setSection(section);
        ptMonitor->setIsUp(isUp);
        for (auto itr : section->lanesWithDirection(isUp))
        {
#ifdef INCLUDE_TRAMS
            // 路面電車レーンは観測対象としない
            // Ignore tram lanes
            if (dynamic_cast<TramLaneInSection*>(const_cast<Lane*>(itr)))
            {
                continue;
            }
#endif //INCLUDE_TRAMS
            ptMonitor->addMonitoredLane(const_cast<Lane*>(itr));
        }
        AppMates::getObjectManager().addConvoyMonitor(ptMonitor);
    }
    fin.close();

    ss << "done";
    amu::msg::status(cout, ss.str());
    return;
}
