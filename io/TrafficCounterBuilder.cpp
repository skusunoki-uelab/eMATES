/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file TrafficCounterReader.cpp
 */
#include "TrafficCounterBuilder.hpp"
#include "TrafficCounterWriter.hpp"
#include "../AppMates.hpp"
#include "../CustomMessage.hpp"
#include "../GVManager.hpp"
#include "../ObjectManager.hpp"
#include <AmuConverter.hpp>
#include <AmuStringOperator.hpp>
#include <sstream>

using namespace std;
using namespace amu::converter;
using namespace amu::string_operator;

//==============================================================================
void TrafficCounterBuilder::buildTrafficCounters(RoadMap* roadMap)
{
    assert(roadMap);
    GVManager& gv    = AppMates::getGVManager();
    string     fname = gv.getString("TRAFFIC_COUNTER_FILE");

    ostringstream ss;
    ss << "read traffic counter file (" << gv.stripDataDir(fname) << ") ... ";

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
        if (tokens.size() != 5)
        {
            ostringstream ssw;
            ssw << "invalid traffic counter format - " << line;
            amu::msg::warn(ssw.str());
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling
        string id           = formatId(tokens[0], NUM_FIGURE_FOR_MONITOR);
        string beginInterId = tokens[1];
        string endInterId   = tokens[2];
        double distance     = stod(tokens[3]);
        ulint  interval     = stoul(tokens[4]);

        // 指定されたsectionが存在するかチェックする
        // Check if the specified section exists
        string sectionId;
        bool   isUp;
        if (stoi(beginInterId) < stoi(endInterId))
        {
            sectionId = formatId(beginInterId, NUM_FIGURE_FOR_INTERSECTION)
                + formatId(endInterId, NUM_FIGURE_FOR_INTERSECTION);
            isUp = true;
        }
        else
        {
            sectionId = formatId(endInterId, NUM_FIGURE_FOR_INTERSECTION)
                + formatId(beginInterId, NUM_FIGURE_FOR_INTERSECTION);
            isUp = false;
        }

        Section* section = roadMap->section(sectionId);
        if (!section)
        {
            ostringstream ssw;
            ssw << "traffic counter[" << id << "]: section[" << sectionId
                << "] does not exist.";
            amu::msg::warn(ssw.str());
            continue;
        }
        if (fabs(distance) > section->length())
        {
            ostringstream ssw;
            ssw << "traffic counter[" << id << "]: " << "distance(" << distance
                << ") is too large for section[" << sectionId << "]";
            amu::msg::warn(ssw.str());
            continue;
        }

        // 車両感知器を生成しObjectManagerに追加する
        // Generate traffic counter and add it to ObjectManager
        TrafficCounter* ptCounter = new TrafficCounter(id);
        if (!(AppMates::getObjectManager().addTrafficCounter(ptCounter)))
        {
            continue;
        }
        ptCounter->setPosition(section, isUp, distance, interval);
    }
    fin.close();

    ss << "done";
    amu::msg::status(cout, ss.str());
    return;
}
