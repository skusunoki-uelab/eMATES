/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file AppCalc.cpp
 */
#include "AppCalc.hpp"
#include "CustomMessage.hpp"
#include "GVManager.hpp"
#include "ManagerBase.hpp"
#include "TimeManager.hpp"
#include "io/VehicleTripWriter.hpp"
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <unistd.h>
#ifndef USE_MINGW
#include <getopt.h>
#endif //USE_MINGW

using namespace std;

//==============================================================================
void AppCalc::initialize(int argc, char** argv, unsigned int loopNum)
{
    AppMates::initialize(argc, argv, loopNum);
}

//==============================================================================
void AppCalc::_parseArgument(int argc, char** argv)
{
#ifndef USE_MINGW
    int opt;

    // getoptのエラー出力を抑制する
    // Suppress error output of getopt
    opterr = 0;

    while ((opt = getopt_long(
                argc, argv, shortOptions.c_str(), longOptions, &optionIndex))
           != -1)
    {
        switch (opt)
        {
        case 2000:
            // シミュレーションの繰り返し回数を指定する
            // Set number of repetitions for the simulation
            {
                int nl = strtol(optarg, nullptr, 10);
                ASSERT_MSG(nl > 0);
                getGVManager().resetNumeric("NUM_LOOPS", nl);
                break;
            }
        default:
            break;
        }
    }

    // ループに備えてインデックスを元に戻す
    // Restore index in preparation for next loop
    optind = 1;

#endif //USE_MINGW
}

//==============================================================================
void AppCalc::_printUsage()
{
    amu::msg::message(cout, "Usage  : ./advmates-calc [Option list]");
    AppMates::_printUsage();
    exit(EXIT_SUCCESS);
}

//==============================================================================
int AppCalc::batchRun()
{
    ulint maxTime = AppMates::getGVManager().getMaxTime();

    ostringstream oss;
    oss << "Run advmates-calc: max time=" << maxTime
        << " (dt=" << AppMates::getTimeManager().unit() << ")";
    amu::msg::title(cout, oss.str());

#ifdef MEASURE_TIME
    AppMates::getClockerManager().startClock("TOTAL_RUN");
#endif //MEASURE_TIME

    _simulator->run(maxTime);

#ifdef MEASURE_TIME
    AppMates::getClockerManager().stopClock("TOTAL_RUN");
#endif //MEASURE_TIME

    if (AppMates::getGVManager().getFlag("FLAG_OUTPUT_TRIP_INFO"))
    {
        VehicleTripWriter writer;
        writer.writeAllVehiclesTrip();
    }

    return EXIT_SUCCESS;
}
