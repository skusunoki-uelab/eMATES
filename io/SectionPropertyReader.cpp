/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file SectionPropertyReader.cpp
 */
#include "SectionPropertyReader.hpp"
#include "../AppMates.hpp"
#include "../Config.hpp"
#include "../CustomMessage.hpp"
#include "../GVManager.hpp"
#include "../Lane.hpp"
#include "../RoadMap.hpp"
#include "../ScheduleManager.hpp"
#include "../Section.hpp"
#include "../SpeedLimitItem.hpp"
#include "../TimeManager.hpp"
#include "../VehicleType.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <AmuConverter.hpp>
#include <AmuStringOperator.hpp>

using namespace std;
using namespace amu::converter;
using namespace amu::string_operator;

//==============================================================================
bool SectionPropertyReader::setSpeedLimit()
{
    assert(_roadMap);
    GVManager& gv    = AppMates::getGVManager();
    string     fname = gv.getString("SPEED_LIMIT_FILE");

    ostringstream ss;
    ss << "read speed limit file (" << gv.stripDataDir(fname) << ") ... ";

    // ファイルを読み込む
    // Load file
    ifstream fin(fname.c_str(), ios::in);
    if (!fin)
    {
        ss << "not found";
        amu::msg::status(cout, ss.str());
        return true;
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
        /*
         * 後方互換性の確保
         *   旧フォーマットでは，第1カラム(開始時刻)および第2カラム
         *   (終了時刻)が不足するので補完する．
         *
         * Ensuring backward compatibility
         *   In the old format, the 1st (start time) and 2nd (end time)
         *   colums are missing, so they are supplemented.
         */
        if (tokens.size() == 3)
        {
            tokens.resize(5);
            for (int i = 4; i >= 2; i--)
            {
                tokens[i] = tokens[i - 2];
            }
            tokens[0] = "0";
            tokens[1] = "86400000";
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 不正な行の処理
        // Invalid line handling
        if (tokens.size() != 5)
        {
            cerr << "WARNING: invalid speed limit file format - " << line
                 << endl;
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling
        ulint  beginTime    = atoi(tokens[0].c_str());
        ulint  endTime      = atoi(tokens[1].c_str());
        string beginInterId = formatId(tokens[2], NUM_FIGURE_FOR_INTERSECTION);
        string endInterId   = formatId(tokens[3], NUM_FIGURE_FOR_INTERSECTION);
        double limit        = stod(tokens[4]);

        bool     isUp;
        Section* section = _roadMap->section(beginInterId, endInterId, isUp);
        if (!section)
        {
            cerr << "WARNING: read speed limit - "
                 << "section connecting intersection[" << beginInterId
                 << "]<->intersection[" << endInterId << "] does not exist."
                 << endl;
            continue;
        }


        if (beginTime == 0)
        {
            /* 
             * 開始時刻が0の場合は ScheduleManager に登録せず直接設定する
             *
             * If the start time is 0, set it directly without registering it
             * with ScheduleManager.
             */
            section->setSpeedLimit(isUp, limit);
        }
        else
        {
            SpeedLimitItem* item
                = new SpeedLimitItem(beginTime, section, isUp, limit);
            AppMates::getScheduleManager().addEnvironmentItem(beginTime, item);
            section->addSpeedLimitItem(item);

            // 値のセットはSimulation::timeIncrement()内でおこなう
            // Set value to section in Simulation::timeIncrement()
        }
    }
    fin.close();

    ss << "done";
    amu::msg::status(cout, ss.str());
    return true;
}

//==============================================================================
bool SectionPropertyReader::setTrafficControl()
{
    assert(_roadMap);
    GVManager&    gv    = AppMates::getGVManager();
    string        fname = gv.getString("TRAFFIC_CONTROL_SECTION_FILE");
    ostringstream ss;
    ss << "read traffic control file (" << gv.stripDataDir(fname) << ") ... ";

    // ファイルを読み込む
    // Load file
    ifstream fin(fname.c_str(), ios::in);
    if (!fin)
    {
        ss << "not found";
        amu::msg::status(cout, ss.str());
        return true;
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
        if (tokens.size() < 4)
        {
            cerr << "WARNING: invalid traffic control file format - " << line
                 << endl;
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling
        bool   allowListMode = true;
        string beginInterId  = formatId(tokens[0], NUM_FIGURE_FOR_INTERSECTION);
        string endInterId    = formatId(tokens[1], NUM_FIGURE_FOR_INTERSECTION);
        /*
         * tokens[2]が'+'なら許可リスト， '-'なら禁止リストに登録
         *
         * If tokens[2] is '+', add vehicle types to allow list,
         * and if '-', add vehicle types to deny list. 
         */
        if (tokens[2] == "+")
        {
            allowListMode = true;
        }
        else if (tokens[2] == "-")
        {
            allowListMode = false;
        }
        else
        {
            cerr << "WARNING: read traffic control - "
                 << "The 3rd column must be \"+\" or \"-\" in line:" << endl
                 << line << endl;
            continue;
        }

        bool     isUp;
        Section* section = _roadMap->section(beginInterId, endInterId, isUp);
        if (!section)
        {
            cerr << "WARNING: read traffic control - "
                 << "section connecting intersection[" << beginInterId
                 << "]<->intersection[" << endInterId << "] does not exist."
                 << endl;
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        if (allowListMode)
        {
            // 車種を単路部の許可リストに登録
            // Add vehicle types to the allow list of the section
            for (unsigned int i = 3; i < tokens.size(); i++)
            {
                section->addAllowedVehicleType(isUp, VehicleType(tokens[i]));
            }
        }
        else
        {
            // 車種を単路部を禁止リストに登録
            // Add vehicle types to the deny list of the section
            for (unsigned int i = 3; i < tokens.size(); i++)
            {
                section->addDeniedVehicleType(isUp, VehicleType(tokens[i]));
            }
        }
    }
    fin.close();

    ss << "done";
    amu::msg::status(cout, ss.str());
    return true;
}

//==============================================================================
bool SectionPropertyReader::setRoutingProbability()
{
    assert(_roadMap);
    GVManager&    gv    = AppMates::getGVManager();
    string        fname = gv.getString("ROUTING_PROBABILITY_FILE");
    ostringstream ss;
    ss << "read routing probability file (" << gv.stripDataDir(fname)
       << ") ... ";

    // ファイルを読み込む
    // Load file
    ifstream fin(fname.c_str(), ios::in);
    if (!fin)
    {
        ss << "not found";
        amu::msg::status(cout, ss.str());
        return true;
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
        if (tokens.size() < 4)
        {
            cerr << "WARNING: invalid routing probability file format" << " - "
                 << line << endl;
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling
        string beginInterId = formatId(tokens[0], NUM_FIGURE_FOR_INTERSECTION);
        string endInterId   = formatId(tokens[1], NUM_FIGURE_FOR_INTERSECTION);

        bool     isUp;
        Section* section = _roadMap->section(beginInterId, endInterId, isUp);
        if (!section)
        {
            cerr << "WARNING: read routing probability - "
                 << "section connecting intersection[" << beginInterId
                 << "]<->intersection[" << endInterId << "] does not exist."
                 << endl;
            continue;
        }

        double probability = stod(tokens[2]);

        for (unsigned int i = 3; i < tokens.size(); i++)
        {
            VehicleType type(tokens[i].c_str());
            section->addRoutingProbability(isUp, type, probability);
        }
    }
    fin.close();

    ss << "done";
    amu::msg::status(cout, ss.str());
    return true;
}
