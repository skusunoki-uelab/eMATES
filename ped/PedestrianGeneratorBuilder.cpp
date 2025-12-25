/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file PedestrianGeneratorBuilder.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "PedestrianGeneratorBuilder.hpp"
#include "PedestrianGenerator.hpp"
#include "Zebra.hpp"
#include "ZebraODEdge.hpp"
#include "../AppMates.hpp"
#include "../CustomMessage.hpp"
#include "../GVManager.hpp"
#include "../Intersection.hpp"
#include "../RoadMap.hpp"
#include "../TimeManager.hpp"
#include <AmuStringOperator.hpp>
#include <AmuConverter.hpp>
#include "../Config.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <sstream>

using namespace std;
using namespace amu::converter;
using namespace amu::string_operator;

//======================================================================
PedestrianGenerator*
PedestrianGeneratorBuilder::buildPedestrianGenerator()
{
    _generator = new PedestrianGenerator(_roadMap);
    _readGeneratingVolume();

    return _generator;
}

//======================================================================
void PedestrianGeneratorBuilder::_readGeneratingVolume()
{
    assert(_generator);
    GVManager&    gv    = AppMates::getGVManager();
    string        fname = gv.getString("GENERATE_PEDESTRIAN_FILE");
    ostringstream ss;
    ss << "read pedestrian generation file (" << gv.stripDataDir(fname)
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

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 不正な行の処理
        // Invalid line handling
        if (tokens.size() != 4)
        {
            cerr << "WARNING:"
                 << " invalid pedestrian generation file format - "
                 << line << endl;
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling

        // 1番目のカラムは交差点の識別番号
        // First column is intersection ID number
        string interId
            = formatId(tokens[0], NUM_FIGURE_FOR_INTERSECTION);
        Intersection* inter = _roadMap->intersection(interId);
        if (!inter)
        {
            cerr << "ERROR: intersection[" << interId
                 << "] is not found." << endl;
            continue;
        }

        // 2番目のカラムは境界番号
        // Second column is border direction
        int    dir   = stoi(tokens[1]);
        Zebra* zebra = inter->pedExt()->zebra(dir);
        if (!zebra)
        {
            cerr << "ERROR: zebra[dir=" << dir << "] in intersection["
                 << interId << "] is not found." << endl;
            continue;
        }

        // 3番目のカラムは横断方向
        // Third column is the crossing direction
        int crossingDir = stoi(tokens[2]);
        assert(crossingDir == 0 || crossingDir == 1);

        // 4番目のカラムは発生交通量
        // Fourth column is the generation volume
        double v = stof(tokens[3]);

        // 横断歩道に発生交通量を設定する
        // Set the generation volume for crosswalk
        zebra->beginEdge(crossingDir)->setGenerationVolume(v);
    }
    fin.close();

    ss << "done";
    amu::msg::status(cout, ss.str());
    return;
}

#endif //INCLUDE_PEDESTRIANS
