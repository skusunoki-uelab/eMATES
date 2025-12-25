/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file InflowMonitorWriter.cpp
 */
#include "InflowMonitorWriter.hpp"
#include "../AppMates.hpp"
#include "../FileManager.hpp"
#include "../GVManager.hpp"
#include "../InflowMonitor.hpp"
#include "../Intersection.hpp"
#include "../Lane.hpp"
#include "../Section.hpp"
#include "../TimeManager.hpp"
#include "../Vehicle.hpp"
#include <AmuConverter.hpp>
#include <AmuStringOperator.hpp>
#include <vector>
#include <string>
#include <cassert>
#include <fstream>

using namespace std;
using namespace amu::converter;
using namespace amu::string_operator;

//==============================================================================
void InflowMonitorWriter::prepareFile(InflowMonitor* monitor) const
{
    assert(monitor);

    ofstream* fout
        = AppMates::getFileManager().getOFStream(_filename(monitor).c_str());

    *fout << "#od_node[" << monitor->odNode()->id() << "], incident_section["
          << monitor->section()->id() << "], num_of_lanes("
          << (monitor->section()->lanesFrom(monitor->odNode())).size() << ")"
          << endl
          << "#inflow_time, time_headway, generation_time, "
          << "generation_time_interval, " << "lane, vehicle_id, vehicle_type, "
          << "origin, destination" << endl;

    fout->close();
}

//==============================================================================
void InflowMonitorWriter::writeRecord(InflowMonitor* monitor)
{
    if (!monitor)
    {
        return;
    }

    ofstream* fout
        = AppMates::getFileManager().getOFStream(_filename(monitor).c_str());

    for (auto itr : monitor->records())
    {
        // ここを変更したら prepareFile() の出力も変更
        // If changed, need to change the output of prepareFile()
        *fout << AppMates::getTimeManager().time() << "," << itr->headway()
              << "," << itr->genTime() << "," << itr->genInterval() << ","
              << itr->laneId() << "," << itr->vehicleId() << ","
              << itr->vehicleType() << "," << itr->startId() << ","
              << itr->goalId() << endl;
    }
    fout->close();

    monitor->clearRecords();
}

//==============================================================================
const string InflowMonitorWriter::_filename(InflowMonitor* monitor) const
{
    string outputDir
        = AppMates::getGVManager().getString("RESULT_INSTRUMENT_DIRECTORY");
    string prefix
        = AppMates::getGVManager().getString("RESULT_INFLOW_MONITOR_PREFIX");
    string fname = outputDir + prefix + monitor->id() + ".txt";
    return fname;
}
