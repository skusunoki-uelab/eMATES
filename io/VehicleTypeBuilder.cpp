/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleTypeBuilder.cpp
 */
#include "VehicleTypeBuilder.hpp"
#include "../AppMates.hpp"
#include "../CustomMessage.hpp"
#include "../GVManager.hpp"
#include "../VehicleTypeManager.hpp"
#include "../VehicleEVTypeProperty.hpp"
#include <AmuStringOperator.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace amu::string_operator;

//======================================================================
void VehicleTypeBuilder::buildVehicleTypes(VehicleTypeManager* vtm)
{
    GVManager& gv    = AppMates::getGVManager();
    string     fname = gv.getString("VEHICLE_FAMILY_FILE");

    ostringstream ss;
    ss << "read vehicle type file (" << gv.stripDataDir(fname)
       << ") ... ";

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

        if (tokens.size() != 10 && tokens.size() != 11
            && tokens.size() != 16 && tokens.size() != 17) // [eMATES]
        {
            ostringstream ssw;
            ssw << "invalid vehicle type format - " << line;
            amu::msg::warn(ssw.str());
            continue;
        }

        int         index = 0;
        VehicleType type(tokens[index]);
        index++;
        double bodyLength = stof(tokens[index]);
        index++;
        double bodyWidth = stof(tokens[index]);
        index++;
        double bodyHeight = stof(tokens[index]);
        index++;
        int bodyArticulation = 1;
        if (tokens.size() == 11
            || tokens.size() == 17) // [eMATES]
        {
            bodyArticulation = stoi(tokens[index]);
            if (bodyArticulation < 0 || bodyArticulation > 3)
            {
                ostringstream ssw;
                ssw << "invalid vehicle type format - " << line << endl
                    << "bodyArticulation must be 1, 2, or 3";
                amu::msg::warn(ssw.str());
                bodyArticulation = 1;
            }
            index++;
        }

        // by abe 2025/04/21 有効化
        double bodyWeight = stof(tokens[index]);
        index++;

        double maxAcceleration = stof(tokens[index]);
        index++;
        double maxDeceleration = stof(tokens[index]);
        index++;

        double bodyColorR = stof(tokens[index]);
        index++;
        double bodyColorG = stof(tokens[index]);
        index++;
        double bodyColorB = stof(tokens[index]);
        index++;

        VehicleTypeProperty* property;
        if (gv.getFlag("FLAG_GEN_EV") && type.category() == VehicleCategory::EV)
        { // EV [eMATES]
          double batteryCapacityWs    = stof(tokens[index]) * 1000 * 3600; // kWh -> Ws
          index++;
          double frontalProjectedArea = stof(tokens[index]);
          index++;
          double coeffDrag             = stof(tokens[index]);
          index++;
          double coeffRollingFriction  = stof(tokens[index]);
          index++;
          double mechanicalLoss       = stof(tokens[index]);
          index++;
          double finalGearRatio       = stof(tokens[index]);
          index++;

          property = new VehicleEVTypeProperty(
              type, bodyLength, bodyWidth, bodyHeight, bodyWeight,
              bodyArticulation, maxAcceleration, maxDeceleration,
              bodyColorR, bodyColorG, bodyColorB,
              batteryCapacityWs,
              frontalProjectedArea,
              coeffDrag,
              coeffRollingFriction,
              mechanicalLoss,
              finalGearRatio);
        }
        else
        { // ガソリン車
          property = new VehicleTypeProperty(
              type, bodyLength, bodyWidth, bodyHeight, 0.0,
              bodyArticulation, maxAcceleration, maxDeceleration,
              bodyColorR, bodyColorG, bodyColorB);

        }
        vtm->addProperty(property);
    }
    fin.close();

    ss << "done";
    amu::msg::status(cout, ss.str());
    return;
}
