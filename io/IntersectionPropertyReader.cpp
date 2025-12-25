/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file IntersectionPropertyReader.cpp
 */
#include "IntersectionPropertyReader.hpp"
#include "../AppMates.hpp"
#include "../Config.hpp"
#include "../CustomMessage.hpp"
#include "../GVManager.hpp"
#include "../RoadMap.hpp"
#include "../VehicleType.hpp"
#include <cassert>
#include <AmuConverter.hpp>
#include <AmuStringOperator.hpp>

using namespace std;
using namespace amu::converter;
using namespace amu::string_operator;

//==============================================================================
bool IntersectionPropertyReader::setTrafficControl()
{
    assert(_roadMap);
    GVManager&    gv    = AppMates::getGVManager();
    string        fname = gv.getString("TRAFFIC_CONTROL_INTERSECTION_FILE");
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
        if (tokens.size() < 5 && (tokens[3] != "+" && tokens[3] != "-"))
        {
            cerr << "WARNING: invalid traffic control file format - " << line
                 << endl;
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling
        bool   allowListMode = true;
        string fromInterId   = formatId(tokens[0], NUM_FIGURE_FOR_INTERSECTION);
        string viaInterId    = formatId(tokens[1], NUM_FIGURE_FOR_INTERSECTION);
        string toInterId     = formatId(tokens[2], NUM_FIGURE_FOR_INTERSECTION);
        /*
         * tokens[3]が'+'なら許可リスト， '-'なら禁止リストに登録
         *
         * If tokens[3] is '+', add vehicle types to allow list,
         * and if '-', add vehicle types to deny list. 
         */
        if (tokens[3] == "+")
        {
            allowListMode = true;
        }
        else if (tokens[3] == "-")
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

        Intersection* from = _roadMap->intersection(fromInterId);
        if (!from)
        {
            cerr << "WARNING: read traffic control - upstream intersection["
                 << fromInterId << "] is not found." << endl;
            continue;
        }

        Intersection* via = _roadMap->intersection(viaInterId);
        if (!via)
        {
            cerr << "WARNING: read traffic control - intersection["
                 << viaInterId << "] is not found." << endl;
            continue;
        }

        Intersection* to = _roadMap->intersection(toInterId);
        if (!to)
        {
            cerr << "WARNING: read traffic control - downstream intersection["
                 << toInterId << "] is not found." << endl;
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        if (allowListMode)
        {
            // 車種を交差点の許可リストに登録
            // Add vehicle types to the allow list of the intersection
            for (unsigned int i = 4; i < tokens.size(); i++)
            {
                via->addAllowedVehicleType(from, to, VehicleType(tokens[i]));
            }
        }
        else
        {
            // 車種を交差点を禁止リストに登録
            // Add vehicle types to the deny list of the intersection
            for (unsigned int i = 4; i < tokens.size(); i++)
            {
                via->addDeniedVehicleType(from, to, VehicleType(tokens[i]));
            }
        }
    }
    fin.close();

    ss << "done";
    amu::msg::status(cout, ss.str());
    return true;
}