/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file TramRouteManager.cpp
 */
#ifdef INCLUDE_TRAMS
#include "TramRouteManager.hpp"
#include "TramRoute.hpp"
#include "../AppMates.hpp"
#include "../Config.hpp"
#include "../CustomMessage.hpp"
#include "../GVManager.hpp"
#include "../Intersection.hpp"
#include "../RoadMap.hpp"
#include <AmuConverter.hpp>
#include <AmuStringOperator.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <typeinfo>

using namespace std;
using namespace amu::converter;
using namespace amu::string_operator;
using TL = TramLaneSide;

//======================================================================
TramRouteManager::TramRouteManager()
{
    _tramRoutes.clear();
    _className = typeid(this).name();
}

//======================================================================
TramRouteManager::~TramRouteManager()
{
    for (auto itr : _tramRoutes)
    {
        delete itr;
    }
    _tramRoutes.clear();
}

//======================================================================
void TramRouteManager::readTramRouteFile()
{
    GVManager& gv    = AppMates::getGVManager();
    string     fname = gv.getString("TRAM_LINE_FILE");

    ostringstream ss;
    ss << "read tram line file (" << gv.stripDataDir(fname) << ") ... ";

    // ファイルを読み込む
    // Load file
    fstream fin;
    fin.open(fname.c_str(), ios::in);
    if (!fin)
    {
        ss << "not found";
        amu::msg::status(cout, ss.str());
        return;
    }
    amu::msg::status(cout, ss.str());

    while (fin.good())
    {
        string         str;
        vector<string> tokens;

        getline(fin, str);
        getAdjustString(&str);
        if (str.empty())
        {
            continue;
        }

        TramRoute* tramRoute = new TramRoute();
        _tramRoutes.push_back(tramRoute);

        getTokens(&tokens, str, ',');
        for (unsigned int i = 0; i < tokens.size(); i += 3)
        {
            TL::Type      prevSide;
            TL::Type      nextSide;
            Intersection* inter = NULL;
            Intersection* prev  = NULL;
            Intersection* next  = NULL;

            // 3カラムごとのデータ

            // 第3nカラムは流入レーンの位置(prevSide)
            if (tokens[i] == "N" || tokens[i] == "n")
            {
                prevSide = TL::None;
            }
            else if (tokens[i] == "L" || tokens[i] == "l")
            {
                prevSide = TL::Left;
            }
            else if (tokens[i] == "C" || tokens[i] == "c")
            {
                prevSide = TL::Center;
            }
            else if (tokens[i] == "R" || tokens[i] == "r")
            {
                prevSide = TL::Right;
            }
            else
            {
                prevSide = TL::None;
            }

            // 第3n+1カラムは交差点ID
            inter = _roadMap->intersection(
                formatId(tokens[i + 1], NUM_FIGURE_FOR_INTERSECTION));

            // 第3n+2カラムは流出レーンの位置(nextSide)
            // 反転
            if (tokens[i + 2] == "N" || tokens[i + 2] == "n")
            {
                nextSide = TL::None;
            }
            else if (tokens[i + 2] == "L" || tokens[i + 2] == "l")
            {
                nextSide = TL::Right;
            }
            else if (tokens[i + 2] == "C" || tokens[i + 2] == "c")
            {
                nextSide = TL::Center;
            }
            else if (tokens[i + 2] == "R" || tokens[i + 2] == "r")
            {
                nextSide = TL::Left;
            }
            else
            {
                nextSide = TL::None;
            }

            // 第3n-2カラムは前の交差点ID
            if (static_cast<int>(i) - 2 > 0)
            {
                prev = _roadMap->intersection(formatId(
                    tokens[i - 2], NUM_FIGURE_FOR_INTERSECTION));
            }

            // 第3n+4カラムは次の交差点ID
            if (i + 4 < tokens.size())
            {
                next = _roadMap->intersection(formatId(
                    tokens[i + 4], NUM_FIGURE_FOR_INTERSECTION));
            }

            tramRoute->createTramRouteInter(
                inter, prev, prevSide, next, nextSide);
        }
    }
    fin.close();

    ss << "done";
    amu::msg::status(cout, ss.str());
    return;
}

//======================================================================
void TramRouteManager::getNumTramConnectors(
    Intersection* inter, int dir, array<int, 3>& result_numIn,
    array<int, 3>& result_numOut)
{
    for (auto itr : _tramRoutes)
    {
        itr->getNumTramConnectors(
            inter, dir, result_numIn, result_numOut);
    }
}

//======================================================================
bool TramRouteManager::hasTramRoute(
    Intersection* inter, int inDir, int outDir) const
{
    for (auto itr : _tramRoutes)
    {
        if (itr->connects(inter, inDir, outDir))
        {
            return true;
        }
    }
    return false;
}

//======================================================================
TramRoute* TramRouteManager::tramRouteWithDesignatedGates(
    const vector<const Intersection*>& gates)
{
    TramRoute* result = nullptr;

    for (auto itr : _tramRoutes)
    {
        vector<const Intersection*> tramRouteInters;
        itr->getIntersections(tramRouteInters);

        // 起点が一致するかどうか判定
        // Judge if the start intersections match
        if (tramRouteInters[0] != gates[0])
        {
            continue;
        }
        // 終点が一致するかどうか判定
        // Judge if the end intersections match
        if (tramRouteInters[tramRouteInters.size() - 1]
            != gates[gates.size() - 1])
        {
            continue;
        }
        if (gates.size() == 2)
        {
            /*
             * gatesのサイズが2である場合は起点・終点チェックのみで良い
             *
             * If the size of gate is 2, checking the start and end
             * intersections are only needed.
             */
            result = itr;
            break;
        }
        else
        {
            // 通過交差点が指定されている場合
            // If intersections to pass are specified
            unsigned int j              = 1;
            unsigned int k              = 1;
            bool         foundTramRoute = false;
            for (; j < tramRouteInters.size() - 1; j++)
            {
                if (tramRouteInters[j] == gates[k])
                {
                    k++;
                    if (k == gates.size() - 1)
                    {
                        foundTramRoute = true;
                        break;
                    }
                }
            }
            if (foundTramRoute)
            {
                result = itr;
                break;
            }
        }
    }
    return result;
}

//======================================================================
void TramRouteManager::print(ostream& out) const
{
    if (_tramRoutes.size() > 0)
    {
        amu::msg::title(out, "Tram Lines");
    }

    for (unsigned int i = 0; i < _tramRoutes.size(); i++)
    {
        ostringstream ss;
        ss << "Line " << i << ": ";
        _tramRoutes[i]->print(ss);
        amu::msg::message(out, ss.str());
    }
}

#endif //INCLUDE_TRAMS
