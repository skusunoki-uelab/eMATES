/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file TrafficCounterWriter.cpp
 */
#include "TrafficCounterWriter.hpp"
#include "../AppMates.hpp"
#include "../FileManager.hpp"
#include "../GVManager.hpp"
#include "../ObjectManager.hpp"
#include "../TimeManager.hpp"
#include "../TrafficCounter.hpp"
#include "../TrafficCounterComponent.hpp"
#include <AmuConverter.hpp>

#define TRUCK_2_PASSENGER 1.7

using namespace std;
using namespace amu::converter;

//==============================================================================
void TrafficCounterWriter::prepareFiles(TrafficCounter* counter)
{
    assert(counter);

    // 感知器1つにつき最大2つのファイルが必要
    // Up to 2 files required per 1 traffic counter
    {
        ofstream* fout = AppMates::getFileManager().getOFStream(
            _detailedFilename(counter).c_str());

        *fout << "#section[" << counter->section()->id() << "], is_up("
              << (counter->isUp() ? "1" : "0") << "), distance("
              << counter->distance() << "), num_of_components("
              << counter->components().size() << ")" << endl
              << "#time, lane, vehicle_id, vehicle_type,"
              << " origin, destination, speed" << endl;

        fout->close();
    }
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    {
        ofstream* fout = AppMates::getFileManager().getOFStream(
            _aggregatedFilename(counter).c_str());

        *fout << "#section[" << counter->section()->id() << "], is_up("
              << (counter->isUp() ? "1" : "0") << "), distance("
              << counter->distance() << "), interval(" << counter->interval()
              << "), num_of_components(" << counter->components().size() << ")"
              << endl;
        auto lanes = counter->lanes();
        for (unsigned int i = 0; i < lanes.size(); i++)
        {
            *fout << "#" << i << ": lane[" << lanes[i]->id() << "]";
            if (i < lanes.size() - 1)
            {
                *fout << ", ";
            }
            *fout << endl;
        }
        *fout << "#begin_time, end_time,"
              << " total(conv), total(simple), total_p, total_t,"
              << " volume_p,volume_t," << " volume0p,volume0t,"
              << " volume1p,volume1t, ..." << endl;

        fout->close();
    }
}

//==============================================================================
void TrafficCounterWriter::writeRecords(
    TrafficCounter* counter, //
    bool outputsDetailedFile, bool outputsAggregatedFile)
{
    if (!counter)
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
            _detailedFilename(counter).c_str());

        for (auto itr : counter->components())
        {
            // ここを変更したら_prepareDetailFile()も変更
            // If changed, need to change _prepareDetailFile()
            for (auto itr_r : itr->records())
            {
                *fout << time << "," << itr_r->laneId() << ","
                      << itr_r->vehicleId() << "," << itr_r->vehicleType()
                      << "," << itr_r->startId() << "," << itr_r->goalId()
                      << "," << itr_r->velocity() << endl;
            }
            // 出力した情報のクリア
            // Clear output records
            itr->clearRecords();
        }

        fout->close();
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 集計データの出力
    // Output aggregate data
    if (outputsAggregatedFile && time % (counter->interval()) == 0)
    {
        // ここをを変更したら_prepareDetailFile()も変更
        // If changed, need to change  _prepareAggregateFile()
        TrafficCounter::AggregatedRecord& record = counter->aggregatedRecord();

        ofstream* fout = AppMates::getFileManager().getOFStream(
            _aggregatedFilename(counter).c_str());

        *fout << record.beginTime() << ", " << time << ", "
              << (record.totalPassengers()
                  + record.totalTrucks() * TRUCK_2_PASSENGER)
              << ", " << (record.totalPassengers() + record.totalTrucks())
              << ", " << record.totalPassengers() << "," << record.totalTrucks()
              << ", " << record.sumPassengers() << "," << record.sumTrucks();

        for (unsigned int i = 0; i < counter->components().size(); i++)
        {
            *fout << ", " << record.numPassengers(i) << ","
                  << record.numTrucks(i);
        }
        *fout << endl;

        // 出力した情報のクリア
        // Clear output records
        counter->clearAggregatedRecord();

        fout->close();
    }
}

//==============================================================================
const string TrafficCounterWriter::_detailedFilename(
    TrafficCounter* counter) const
{
    GVManager& gv = AppMates::getGVManager();

    string outputDir = gv.getString("RESULT_INSTRUMENT_DIRECTORY");
    string prefixD   = gv.getString("RESULT_TRAFFIC_COUNTER_D_PREFIX");

    string fname = outputDir + prefixD + counter->id() + ".txt";
    return fname;
}

//==============================================================================
const string TrafficCounterWriter::_aggregatedFilename(
    TrafficCounter* counter) const
{
    GVManager& gv = AppMates::getGVManager();

    string outputDir = gv.getString("RESULT_INSTRUMENT_DIRECTORY");
    string prefixS   = gv.getString("RESULT_TRAFFIC_COUNTER_S_PREFIX");

    string fname = outputDir + prefixS + counter->id() + ".txt";
    return fname;
}
