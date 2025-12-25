/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file InflowPedestrianMonitorWriter.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "InflowPedestrianMonitorWriter.hpp"
#include "InflowPedestrianMonitor.hpp"
#include "../AppMates.hpp"
#include "../FileManager.hpp"
#include "../GVManager.hpp"
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
void InflowPedestrianMonitorWriter::prepareFile(
    InflowPedestrianMonitor* monitor)
{
    assert(monitor);

    ofstream* fout = AppMates::getFileManager().getOFStream(_filename(monitor));

    *fout << "#intersection[" << monitor->laneBundle()->id() << "], zebra["
          << monitor->zebra()->id() << "], cross_dir("
          << monitor->zebraODEdge()->crossingDirection() << ")" << endl
          << "#inflow_time, time_headway, "
          << "generation_time, generation_time_interval, " << "pedestrian_id"
          << endl;

    fout->close();
}

//==============================================================================
void InflowPedestrianMonitorWriter::writeRecord(
    InflowPedestrianMonitor* monitor)
{
    if (!monitor)
    {
        return;
    }

    ofstream* fout = AppMates::getFileManager().getOFStream(_filename(monitor));

    for (auto itr : monitor->records())
    {
        // ここを変更したら _prepareFile() の出力も変更
        // If changed, need to change the output of _prepareFile()
        *fout << AppMates::getTimeManager().time() << "," << itr->headway()
              << "," << itr->genTime() << "," << itr->genInterval() << ","
              << itr->pedestrianId() << endl;
    }
    fout->close();

    monitor->clearRecords();
}

//==============================================================================
const string InflowPedestrianMonitorWriter::_filename(
    InflowPedestrianMonitor* monitor) const
{
    string outputDir
        = AppMates::getGVManager().getString("RESULT_INSTRUMENT_DIRECTORY");
    string prefix = AppMates::getGVManager().getString(
        "RESULT_PEDESTRIAN_INFLOW_MONITOR_PREFIX");
    string fname = outputDir + prefix + monitor->id() + ".txt";

    return fname;
}

#endif //INCLUDE_PEDESTRIANS
