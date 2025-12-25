/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file GuiVehicleGeneration.cpp
 */
#include "GuiVehicleGeneration.hpp"
#include "Visualizer.hpp"
#include "../AppMates.hpp"
#include "../GVManager.hpp"
#include <AmuConverter.hpp>
#include <AmuStringOperator.hpp>
#include <autogl.h>
#include <cstring>

using namespace std;
using namespace amu::converter;
using namespace amu::string_operator;

char GuiVehicleGeneration::_vehicleType[16];
char GuiVehicleGeneration::_startId[16];
char GuiVehicleGeneration::_goalId[16];
char GuiVehicleGeneration::_gateId[256];
char GuiVehicleGeneration::_routingParams[256];
char GuiVehicleGeneration::_prefRank[16];

//======================================================================
GuiVehicleGeneration::GuiVehicleGeneration() {}

//======================================================================
void GuiVehicleGeneration::makePanel()
{
    AutoGL_AddGroup(" Vehicle Generation ");
    AutoGL_AddString(_vehicleType, "_vehicleType", 16);
    AutoGL_SetLabel("Type ID");
    AutoGL_AddString(_startId, "_startId", 16);
    AutoGL_SetLabel("Origin");
    AutoGL_AddString(_goalId, "_goalId", 16);
    AutoGL_SetLabel("Destination");
    AutoGL_AddString(_gateId, "_gateId", 256);
    AutoGL_SetLabel("Intersections to Pass");
    AutoGL_AddString(_routingParams, "_routingParams", 256);
    AutoGL_SetLabel("Routing Parameters");
    AutoGL_AddString(_prefRank, "_prefRank", 16);
    AutoGL_SetLabel("Preferred Network Rank");
    AutoGL_AddCallback(
        generateVehicleButtonCallback, "generateVehicleButtonCallback");
    AutoGL_SetLabel("Generate");
}

//======================================================================
void GuiVehicleGeneration::generateVehicleButtonCallback()
{
    // 交差点の存在は VehicleGenerator で確認する．
    // Check existence of intersections with VehicleGenerator .

    // _startId は必須
    // _startId is required.
    if (strcmp(_startId, "") == 0)
    {
        cout << "ERROR(" << __FUNCTION__
             << "): origin ID must be input." << endl;
        return;
    }
    string startId = formatId(_startId, NUM_FIGURE_FOR_INTERSECTION);

    // _goalIdは入力されなければランダム
    // _goalId is random if not input.
    string goalId;
    if (strcmp(_goalId, "") == 0)
    {
        goalId = "******";
    }
    else
    {
        goalId = formatId(_goalId, NUM_FIGURE_FOR_INTERSECTION);
    }

    // 経由地
    // Intersections to pass
    vector<string> gateIds;
    gateIds.clear();
    gateIds.push_back(startId);

    string         str = _gateId;
    vector<string> tokens;
    getAdjustString(&str);
    getTokens(&tokens, str, ',');
    if (str != "")
    {
        for (auto itr : tokens)
        {
            if (itr == startId || itr == goalId)
            {
                continue;
            }

            gateIds.push_back(
                formatId(itr, NUM_FIGURE_FOR_INTERSECTION));
        }
    }
    gateIds.push_back(goalId);
    tokens.clear();

    // 経路選択パラメータ
    // Routing parameters
    vector<double> params;
    params.resize(VEHICLE_ROUTING_PARAMETER_SIZE);
    params[0] = 1;
    for (unsigned int i = 1; i < VEHICLE_ROUTING_PARAMETER_SIZE; i++)
    {
        params[i] = 0;
    }
    str = _routingParams;
    getAdjustString(&str);
    getTokens(&tokens, str, ',');
    if (str != "")
    {
        for (auto itr : tokens)
        {
            params.push_back(stod(itr));
            if (params.size() >= VEHICLE_ROUTING_PARAMETER_SIZE)
            {
                break;
            }
        }
    }
    tokens.clear();

    // 選好するネットワークランク
    // Preferred network rank
    int prefRank;
    if (strcmp(_prefRank, "") == 0)
    {
        prefRank = INT_MAX;
    }
    else
    {
        prefRank = max(1, atoi(_prefRank));
    }

    // 車両を生成
    // Generate vehicle
    Visualizer::simulator()
        ->vehicleGenerator()
        ->generateVehicleManually(
            startId, goalId, &gateIds, VehicleType(_vehicleType),
            params, prefRank);
}
