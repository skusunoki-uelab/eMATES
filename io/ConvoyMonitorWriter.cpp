/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file ConvoyMonitorWriter.cpp
 */
#include "ConvoyMonitorWriter.hpp"
#include "../AppMates.hpp"
#include "../ConvoyMonitor.hpp"
#include "../FileManager.hpp"
#include "../GVManager.hpp"
#include "../Lane.hpp"
#include "../ObjectInLane.hpp"
#include "../TimeManager.hpp"
#ifdef INCLUDE_TRAMS
#include "../tram/TramLaneInSection.hpp"
#endif //INCLUDE_TRAMS
#include <cstdlib>

using namespace std;

//======================================================================
void ConvoyMonitorWriter::prepareFile(ConvoyMonitor* monitor) const
{
    assert(monitor);

    ofstream* fout = AppMates::getFileManager().getOFStream(
        _filename(monitor).c_str());

    *fout << "#section[" << monitor->section()->id() << "], is_up("
          << (monitor->isUp() ? "1" : "0") << ")" << endl;
    *fout << "#time, lane, distance_to_downstream_intersection" << endl;

    fout->close();
}

//======================================================================
void ConvoyMonitorWriter::writeRecord(ConvoyMonitor* monitor) const
{
    if (!monitor)
    {
        return;
    }

    if (AppMates::getTimeManager().time()
            % (static_cast<int>(AppMates::getGVManager().getNumeric(
                   "INTERVAL_CONVOY_MONITOR"))
               * 1000)
        != 0)
    {
        return;
    }

    ofstream* fout = AppMates::getFileManager().getOFStream(
        _filename(monitor).c_str());

    *fout << AppMates::getTimeManager().time();
    for (auto itr : monitor->records())
    {
        *fout << "," << itr.second->laneId() << ","
              << itr.second->distance();
    }
    *fout << endl;

    fout->close();
}

//======================================================================
const string ConvoyMonitorWriter::_filename(
    ConvoyMonitor* monitor) const
{
    string outputDir = AppMates::getGVManager().getString(
        "RESULT_INSTRUMENT_DIRECTORY");
    string prefix = AppMates::getGVManager().getString(
        "RESULT_CONVOY_MONITOR_PREFIX");
    string fname = outputDir + prefix + monitor->id() + ".txt";
    return fname;
}
