/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file LinkFlowMonitorWriter.cpp
 */
#include "LinkFlowMonitorWriter.hpp"
#include "../AppMates.hpp"
#include "../FileManager.hpp"
#include "../GVManager.hpp"
#include "../LinkFlowMonitor.cpp"
#include "../LinkFlowRecord.hpp"
#include "../Section.hpp"
#include "../TimeManager.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>

using namespace std;

//==============================================================================
void LinkFlowMonitorWriter::prepareFile(LinkFlowMonitor* monitor) const
{
    assert(monitor);

    // 観測器1つにつき最大2つのファイルが必要
    // Up to 2 files required per 1 monitor
    {
        ofstream* fout = AppMates::getFileManager().getOFStream(
            _detailedFilename(monitor).c_str());

        *fout << "#section[" << monitor->section()->id() << "], is_up("
              << (monitor->isUp() ? "1" : "0") << ")" << endl
              << "#time, lane, vehicle_id, vehicle_type, "
              << "origin, destination, "
              << "link_travel_time, num_of_stops, stop_duration" << endl;

        fout->close();
    }
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    {
        ofstream* fout = AppMates::getFileManager().getOFStream(
            _aggregatedFilename(monitor).c_str());

        *fout << "#section[" << monitor->section()->id() << "], is_up("
              << (monitor->isUp() ? "1" : "0") << ")" << endl
              << "#time, volume, density, space_mean_speed" << endl;

        fout->close();
    }
}

//==============================================================================
void LinkFlowMonitorWriter::writeRecord(
    LinkFlowMonitor* monitor, //
    bool outputsDetailedFile, bool outputsAggregatedFile) const
{
    if (!monitor)
    {
        return;
    }

    ulint time = AppMates::getTimeManager().time();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 詳細データの出力
    // Output detail file
    if (outputsDetailedFile)
    {
        ofstream* fout = AppMates::getFileManager().getOFStream(
            _detailedFilename(monitor).c_str());

        for (auto itr : monitor->passRecords())
        {
            *fout << time << ","               //
                  << itr->laneId() << ","      //
                  << itr->vehicleId() << ","   //
                  << itr->vehicleType() << "," //
                  << itr->startId() << ","     //
                  << itr->goalId() << ","      //
                  << itr->travelTime() << ","  //
                  << itr->numStops() << ","    //
                  << itr->stopTime() << endl;
        }
        fout->close();
    }
    monitor->clearRecords();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 集計データの出力
    // Output aggregate data
    if (outputsAggregatedFile && monitor->outputsInCurrentStep())
    {
        ofstream* fout = AppMates::getFileManager().getOFStream(
            _aggregatedFilename(monitor).c_str());

        LinkFlowRecord* record
            = monitor->section()->linkFlowRecord(monitor->isUp());

        *fout << time << ", "
              << record->volumeInSection() * 3600000.0    // [veh/ms]->[veh/h]
              << ", "                                     //
              << record->densityInSection() * 1000.0      // [veh/m]->[veh/km]
              << ", "                                     //
              << record->meanVelocityInSection() * 3600.0 // [m/ms]->[km/h]
              << endl;                                    //
        fout->close();
        monitor->setOutputsInCurrentStep(false);
    }
}

//==============================================================================
const string LinkFlowMonitorWriter::_detailedFilename(
    LinkFlowMonitor* monitor) const
{
    string outputDir
        = AppMates::getGVManager().getString("RESULT_INSTRUMENT_DIRECTORY");
    string prefix = AppMates::getGVManager().getString(
        "RESULT_LINK_FLOW_MONITOR_D_PREFIX");
    string fname = outputDir + prefix + monitor->id() + ".txt";

    return fname;
}

//==============================================================================
const string LinkFlowMonitorWriter::_aggregatedFilename(
    LinkFlowMonitor* monitor) const
{
    string outputDir
        = AppMates::getGVManager().getString("RESULT_INSTRUMENT_DIRECTORY");
    string prefix = AppMates::getGVManager().getString(
        "RESULT_LINK_FLOW_MONITOR_S_PREFIX");
    string fname = outputDir + prefix + monitor->id() + ".txt";

    return fname;
}
