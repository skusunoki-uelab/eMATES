/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file AppSim.cpp
 */
#include "AppSim.hpp"
#include "CustomMessage.hpp"
#include "GVManager.hpp"
#include "ManagerBase.hpp"
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <iostream>
#ifndef USE_MINGW
#include <getopt.h>
#endif

using namespace std;

//==============================================================================
void AppSim::initialize(int argc, char** argv, unsigned loopNum)
{
    GVManager& gv = getGVManager();

    /*
     * 出力の抑制
     * - 値はファイル/実行時オプションにより上書きされる可能性がある
     *
     * Suppress output
     * - Values may be overwritten by file/command-line options. 
     */
    gv.resetFlag("FLAG_OUTPUT_TIMELINE_VEHICLE_D", false);
    gv.resetFlag("FLAG_OUTPUT_TIMELINE_PEDESTRIAN_D", false);
    gv.resetFlag("FLAG_OUTPUT_TIMELINE_SIGNAL_D", false);
    gv.resetFlag("FLAG_OUTPUT_TIMELINE_S", false);
    gv.resetFlag("FLAG_OUTPUT_TRIP_INFO", false);
    gv.resetFlag("FLAG_OUTOUT_TRAFFIC_COUNTER_D", false);
    gv.resetFlag("FLAG_OUTOUT_TRAFFIC_COUNTER_S", false);
    gv.resetFlag("FLAG_OUTPUT_LINK_TRAFFIC_FLOW_D", false);
    gv.resetFlag("FLAG_OUTPUT_LINK_TRAFFIC_FLOW_S", false);
    gv.resetFlag("FLAG_OUTPUT_INFLOW_MONITOR", false);
    gv.resetFlag("FLAG_OUTPUT_CONVOY_MONITOR", false);

    AppMates::initialize(argc, argv, loopNum);
}

//==============================================================================
void AppSim::_parseArgument(int argc, char** argv)
{
#ifndef USE_MINGW
    double size = 200;
    double cx = 0, cy = 0, cz = 0;
    double dx = 0, dy = 0, dz = 1;
    double ux = 0, uy = 1, uz = 0;

    stringstream ss;

    GVManager& gv = getGVManager();

    int startTime = 0;
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
        case 3000:
            // ビューのサイズを指定する
            // Set the view size.
            size = atof(optarg);
            gv.resetNumeric("VIS_VIEW_SIZE", size);
            break;
        case 3001:
            // ビューの中心を指定する
            // Set the view center.
            {
                stringstream ss;
                ss << optarg;
                ss >> cx >> cy >> cz;
                gv.resetNumeric("VIS_VIEW_CENTER_X", cx);
                gv.resetNumeric("VIS_VIEW_CENTER_Y", cy);
                gv.resetNumeric("VIS_VIEW_CENTER_Z", cz);
                break;
            }
        case 3002:
            // ビューの視線方向を指定する
            // Set the view direction
            {
                stringstream ss;
                ss << optarg;
                ss >> dx >> dy >> dz;
                gv.resetNumeric("VIS_VIEW_DIRECTION_X", dx);
                gv.resetNumeric("VIS_VIEW_DIRECTION_Y", dy);
                gv.resetNumeric("VIS_VIEW_DIRECTION_Z", dz);
                break;
            }
        case 3003:
            // ビューの画面上向き方向を指定する
            // Set upward direction (up-vector) of the view
            {
                stringstream ss;
                ss << optarg;
                ss >> ux >> uy >> uz;
                gv.resetNumeric("VIS_VIEW_UPVECTOR_X", ux);
                gv.resetNumeric("VIS_VIEW_UPVECTOR_Y", uy);
                gv.resetNumeric("VIS_VIEW_UPVECTOR_Z", uz);
                break;
            }
        case 3010:
            // 時計を描画する
            // Draw analog clock
            {
                startTime       = atoi(optarg);
                int startMinute = startTime % 100;
                int startHour   = (startTime - startMinute) / 100;
                if (startMinute < 0 || startMinute >= 60 || startHour < 0
                    || startHour >= 24)
                {
                    ostringstream ssw;
                    ssw << "Invalid start time (" << startTime
                        << "). It must be 0000 - 2359 in HHMM format" << endl;
                    amu::msg::warn(ssw.str());
                    break;
                }
                gv.resetFlag("VIS_SHOW_ANALOG_CLOCK", true);
                gv.resetNumeric("VIS_CLOCK_START_HOUR", startHour);
                gv.resetNumeric("VIS_CLOCK_START_MINUTE", startMinute);
                break;
            }
        default:
            break;
        }
    }

#endif
}

//==============================================================================
void AppSim::_printUsage()
{
    cout << "Usage  : ./advmates-sim [Option list] \n" << endl;
    AppMates::_printUsage();
#ifndef USE_MINGW
    cout << " --view-size <Size> : set view size to <Size>.\n"
            " --view-center <Position>\n"
            "                    : "
            "set view center to <Position>.\n"
            "                      "
            "The position format is the concatenation of\n"
            "                      "
            "signed x, y and z-coordinate\n"
            "                      "
            "(like +10.0-5.0+0).\n"
            " --view-direction <Vector>\n"
            "                    : "
            "set view direction to <Vector>.\n"
            "                      "
            "The vector format is the same as the position format.\n"
            " --view-upvector <Vector>\n"
            "                    : "
            "set up vector to <Vector>.\n"
            "                      "
            "The vector format is the same as the position format.\n"
         << endl;
#endif
    exit(EXIT_SUCCESS);
}

//==============================================================================
void AppSim::getReadyVisualizer()
{
    /*
     * Visualizer用変数の設定
     * - 地図作成(AppMates::init)より後ろでなければならない．
     * - 既にファイル/実行時オプションにより指定されていれば上書きしない．
     *
     * Setting variables for Visualizer
     * - This must be run after AppMates::getReadySimulator().
     * - They will NOT be overwritten if set by file/command-line
     *   options.
     */
    double xmin, xmax, ymin, ymax;
    _simulator->roadMap()->getRegion(xmin, xmax, ymin, ymax);

    double xdiff = xmax - xmin;
    double ydiff = ymax - ymin;

    GVManager& gv = getGVManager();

    // ビューのサイズ (中心から端までの長さ)
    // The view size (length from the view center to the end of the screen)
    if (xdiff >= ydiff)
    {
        gv.setNewNumeric("VIS_VIEW_SIZE", xdiff / 2);
    }
    else
    {
        gv.setNewNumeric("VIS_VIEW_SIZE", ydiff / 2);
    }

    // ビューの中心
    // The view center
    gv.setNewNumeric("VIS_VIEW_CENTER_X", (xmax + xmin) / 2.0);
    gv.setNewNumeric("VIS_VIEW_CENTER_Y", (ymax + ymin) / 2.0);
    gv.setNewNumeric("VIS_VIEW_CENTER_Z", 0);

    // 視線方向
    // The view direction
    gv.setNewNumeric("VIS_VIEW_DIRECTION_X", 0);
    gv.setNewNumeric("VIS_VIEW_DIRECTION_Y", 0);
    gv.setNewNumeric("VIS_VIEW_DIRECTION_Z", 1);

    // 画面上方向
    // The upward direction (up-vector) of the screen
    gv.setNewNumeric("VIS_VIEW_UPVECTOR_X", 0);
    gv.setNewNumeric("VIS_VIEW_UPVECTOR_Y", 1);
    gv.setNewNumeric("VIS_VIEW_UPVECTOR_Z", 0);

    // アナログ時計の表示
    // Draw analog clock
    gv.setNewFlag("VIS_SHOW_ANALOG_CLOCK", false);
    gv.setNewNumeric("VIS_CLOCK_START_HOUR", 0);
    gv.setNewNumeric("VIS_CLOCK_START_MINUTE", 0);

    /*
     * 背景画像ファイル
     * - Visualizerでユーザが入力するため，ディレクトリ名とファイル名を分ける
     *
     * Background image file
     * - Separate directory and file name for user input in Visualizer.
     */
    gv.setNewString(
        "VIS_TEXTURE_DIR", gv.getString("DATA_DIRECTORY") + "texture/");
    gv.setNewString("VIS_BACKGROUND_TEXTURE_FILE", "map.jpg");

    // 背景画像の貼り付け範囲
    // Pasting area of the background image
    gv.setNewNumeric("VIS_BACKGROUND_TEXTURE_XMIN", -100.0);
    gv.setNewNumeric("VIS_BACKGROUND_TEXTURE_XMAX", 100.0);
    gv.setNewNumeric("VIS_BACKGROUND_TEXTURE_YMIN", -100.0);
    gv.setNewNumeric("VIS_BACKGROUND_TEXTURE_YMAX", 100.0);

    // 背景画像の著作権表記
    // Copyright notice of the background image
    gv.setNewString("VIS_BACKGROUND_CREDIT_STRING", "");

    // Visualizerの生成
    // Generating Visualizer
    _vis.reset(new Visualizer());
}

//==============================================================================
int AppSim::run()
{
    if (!_simulator)
    {
        cerr << "Simulator not found." << endl;
        exit(EXIT_FAILURE);
    }
    if (!(_vis.get()))
    {
        cerr << "Visualizer not found." << endl;
        exit(EXIT_FAILURE);
    }

    _vis->setSimulator(_simulator);
    _vis->startVisualization();
    return EXIT_SUCCESS;
}
