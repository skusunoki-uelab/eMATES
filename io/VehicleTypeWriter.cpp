/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleTypeWriter.cpp
 */
#include "VehicleTypeWriter.hpp"
#include "../AppMates.hpp"
#include "../FileManager.hpp"
#include "../GVManager.hpp"
#include <cstdlib>

using namespace std;

//======================================================================
void VehicleTypeWriter::writeVehicleProperty(Vehicle* vehicle)
{
    ofstream* fout = AppMates::getFileManager().getOFStream(
        AppMates::getGVManager().getString(
            "RESULT_VEHICLE_ATTRIBUTE_FILE"));

    *fout << vehicle->id() << "," << *(vehicle->body()->type()) << ","
          << vehicle->bodyLength() << "," << vehicle->bodyWidth() << ","
          << vehicle->bodyHeight() << endl;

    fout->close();
}
