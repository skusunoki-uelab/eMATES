/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file AppMates.cpp
 */
#include "AppMates.hpp"
#include "ClockerManager.hpp"
#include "CustomMessage.hpp"
#include "FileManager.hpp"
#include "GVInitializer.hpp"
#include "GVManager.hpp"
#include "GVManager.hpp"
#include "LoggerManager.hpp"
#include "ManagerBase.hpp"
#include "ManagerPool.hpp"
#include "ObjectManager.hpp"
#include "RandomSeedManager.hpp"
#include "RandomSeedManager.hpp"
#include "ScheduleManager.hpp"
#include "Simulator.hpp"
#include "TimeManager.hpp"
#include "VehicleTypeManager.hpp"
#ifdef INCLUDE_PEDESTRIANS
#include "ped/InflowPedestrianMonitor.hpp"
#endif //INCLUDE_PEDESTRIANS
#ifdef INCLUDE_TRAMS
#include "tram/TramRouteManager.hpp"
#endif //INCLUDE_TRAMS
#include <AmuConverter.hpp>
#include <AmuStringOperator.hpp>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <random>
#include <string>
#include <unistd.h>
#ifdef _OPENMP
#include <omp.h>
#endif //_OPENMP

#define MATES_NDEBUG

using namespace std;
using namespace amu::string_operator;

// 静的変数の初期化
// Initialization of static variables
ManagerPool* AppMates::_managerPool = nullptr;

//==============================================================================
AppMates::AppMates(unsigned long maxTime) : _dataPath("./")
{
    _simulator   = nullptr;
    _managerPool = ManagerPool::instance();

#ifdef MATES_NDEBUG
    random_device dev;
    _key = dev();
#else
    _key = 2;
#endif
    getGVManager().resetMaxTime(maxTime);
}

//==============================================================================
AppMates::~AppMates()
{
    if (_simulator)
    {
        _simulator->writeResultAtExit();
    }

    // マネージャのfinalizeとdelete
    // Finalize and delete managers
    delete _managerPool;
    _managerPool = nullptr;

    if (_simulator)
    {
        delete _simulator;
        _simulator = nullptr;
    }
}

//==============================================================================
void AppMates::initialize(int argc, char** argv, unsigned int loopNum)
{
    /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * コマンドラインオプションの処理 1-1
     *   --no-verbose (-q) オプションのみ先に設定する．
     *
     * Command-line option processing 1-1
     *   Set only the --no-verbose (-q) option first.
     */
    _checkQuietOption(argc, argv);

    /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * コマンドラインオプションの処理 1-2
     *   _dataPathを先に設定する．
     *
     * Command-line option processing 1-2
     *   Set _dataPath first.
     */
    _setDataPath(argc, argv);
    {
        ostringstream ss;
        ss << "data directory: " << _dataPath;
        amu::msg::status(cout, ss.str());
    }

    /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * グローバル変数を初期化する
     *   _dataPathにデータディレクトリのパスが指定されている必要がある．
     *   以下の関数は_setDataPath()より後でなければならない．
     *
     * Initialize global variables.
     *   The path of the data directory must be specified in _dataPath.
     *   This must be run after _setDataPath().
     */
    GVInitializer::initialize(_dataPath, loopNum);

    /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * グローバル変数をGV_INIT_FILEファイルから読み込む
     *   変数の初期値を上書きする．
     *   GVInitializer::initialize()よりも後でなければならない．
     *
     * Read global variables from GV_INIT_FILE file
     *   Overwrite the initial value of global variables.
     *   This must be run after GVInitializer::initialize().
     */
    getGVManager().setVariablesFromFile();

    /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * コマンドラインオプションの処理 2
     *   ファイル指定された値を上書きする．
     *   getGVManager().setVariablesFromFile()よりも後でなければならない．
     *
     * Command-line option processing 2
     *   Overwrite values set in the GV_INIT_FILE file.
     *   This must be after GVManager::setVariablesFromFile().
     */
    AppMates::_parseArgument(argc, argv);

    /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * 必要な出力ディレクトリの作成
     *   setVariablesFromFile()より後でなければならない
     *
     * Prepare required directories for output
     *   This must be after GVManager::setVariablesFromFile().
     */
    FileManager::prepareResultDirectories();

    /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * 乱数の準備
     *   _keyに乱数の種が指定されている必要がある．
     *   _parseArgument()より後でなければならない．
     *
     * Initialize pseudo random number generator
     *   A random seed must be specified in _key.
     *   This must be after _parseArgument().
     */
    {
        ostringstream ss;
        ss << "random seed: " << _key;
        amu::msg::status(cout, ss.str());
    }
    srand(_key);
    getRandomSeedManager().setGlobalSeed(_key);

#ifdef _OPENMP
    /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * スレッド数の設定
     *   _parseArgument()よりも後でなければならない．
     *
     * Set thread number
     *   This must be after _parseArgument().
     */
    int numThread = static_cast<int>(getGVManager().getNumeric("NUM_THREAD"));
    if (numThread <= 0)
    {
        numThread = omp_get_num_procs();
        getGVManager().resetNumeric("NUM_THREAD", numThread);
    }
    {
        ostringstream ss;
        ss << "number of threads: " << numThread;
        amu::msg::status(cout, ss.str());
    }
    omp_set_num_threads(numThread);
#endif //_OPENMP
}

//==============================================================================
int    AppMates::optionIndex;
string AppMates::shortOptions = "HhD:d:R:r:T:t:C:c:SsLlMmGgQq"; // 2025/3/25 by abe

#ifndef USE_MINGW
struct option AppMates::longOptions[] = {
    {"help",                                     0, 0, 'h' },
    {"time",                                     1, 0, 't' },

    // 入力に関するフラグの処理
    // Handling flags about input.
    {"no-input",                                 0, 0, 300 },
    {"no-input-map",                             0, 0, 301 },
    {"no-input-signal",                          0, 0, 302 },
    {"no-input-vehicle",                         0, 0, 303 },

    // 出力に関するフラグの処理
    // Handling flags about output.
    {"output-timeline",                          0, 0, 'S' },
    {"no-output-timeline",                       0, 0, 's' },
    {"output-tripinfo",                          0, 0, 'L' },
    {"no-output-tripinfo",                       0, 0, 'l' },
    {"output-monitor",                           0, 0, 'M' },
    {"no-output-monitor",                        0, 0, 'm' },
    {"verbose",                                  0, 0, 'Q' },
    {"no-verbose",                               0, 0, 'q' },

    {"output-timeline-d",                        0, 0, 200 },
    {"no-output-timeline-d",                     0, 0, 201 },
    {"output-timeline-s",                        0, 0, 202 },
    {"no-output-timeline-s",                     0, 0, 203 },
    {"output-timeline-vehicle-d",                0, 0, 204 },
    {"no-output-timeline-vehicle-d",             0, 0, 205 },
    {"output-timeline-pedestrian-d",             0, 0, 206 },
    {"no-output-timeline-pedestrian-d",          0, 0, 207 },
    {"output-timeline-signal-d",                 0, 0, 208 },
    {"no-output-timeline-signal-d",              0, 0, 209 },

    {"output-monitor-d",                         0, 0, 210 },
    {"no-output-monitor-d",                      0, 0, 211 },
    {"output-monitor-s",                         0, 0, 212 },
    {"no-output-monitor-s",                      0, 0, 213 },

    {"more-verbose",                             0, 0, 214 },

    {"output-cs-interval",                       0, 0, 215 }, // [eMATES]
    {"no-output-cs-interval",                    0, 0, 216 }, // [eMATES]

    // シミュレーションの制御に関する設定
    // Setting about simulation control.
    {"num-cores",                                1, 0, 'c' },
    {"no-generate-random-vehicle",               0, 0, 400 },
    {"generate-vehicle-equal-interval",          0, 0, 401 },
    {"random-od-factor",                         1, 0, 402 },
    {"tabled-od-factor",                         1, 0, 403 },
    {"debug-ignore-yielding",                    0, 0, 404 },
    {"debug-all-section-single-lane-each-side",  0, 0, 405 },
    {"debug-generate-fixed-vehicle-all-at-once", 0, 0, 406 },
    {"gen-ev",                                   0, 0, 407 },

    // シミュレーションのキャッシュに関する設定
    // Setting about simulation cache.
    {"cache-routing-read",                       0, 0, 500 },
    {"no-cache-routing-read",                    0, 0, 501 },
    {"cache-routing-write",                      0, 0, 502 },
    {"no-cache-routing-write",                   0, 0, 503 },
    {"cache-routing-size",                       1, 0, 504 },
    {"cache-routing-probability",                1, 0, 505 },

    // 以下はAppCalcの処理対象
    // Followings are processed in AppCalc.
    {"num-loops",                                1, 0, 2000},

    // 以下はAppSimの処理対象
    // Followings are processed in AppSim.
    {"view-size",                                1, 0, 3000},
    {"view-center",                              1, 0, 3001},
    {"view-direction",                           1, 0, 3002},
    {"view-upvector",                            1, 0, 3003},
    {"show-analog-clock",                        1, 0, 3010},

    {0,                                          0, 0, 0   }
};
#endif //if USE_MINGW not defined

//==============================================================================
void AppMates::_parseArgument(int argc, char** argv)
{
    amu::msg::status(cout, "parse arguments ...");

    GVManager& gv = getGVManager();

    int opt;

    // getoptのエラー出力を抑制する
    // Suppress error output of getopt
    opterr = 0;

#ifdef USE_MINGW
    while ((opt = getopt(argc, argv, shortOptions.c_str())) != -1)
#else  //USE_MINGW not defined
    while ((opt = getopt_long(
                argc, argv, shortOptions.c_str(), longOptions, &optionIndex))
           != -1)
#endif //USE_MINGW
    {
        switch (opt)
        {
        case 'H':
        case 'h':
            // 説明を出力する
            // Print usage to stdout
            _printUsage();
            break;
        case 'R':
        case 'r':
            // 乱数の種を指定する
            // Set random seed
            _setRandomSeed(optarg);
            break;
        case 'T':
        case 't':
            // シミュレーション対象時間 [ms] を指定する
            // Set simulation target time [ms]
            gv.resetMaxTime(strtoul(optarg, nullptr, 10));
            break;

#ifdef _OPENMP
        case 'C':
        case 'c':
            // OpenMPで使用されるコア数を指定する
            // Set number of cores to be used by OpenMP
            {
                int nt = strtol(optarg, nullptr, 10);
                int np = omp_get_num_procs();
                if (nt > np)
                {
                    ostringstream ss;
                    ss << "designated number of threads (" << nt
                       << ") is reset to " << np << ", number of cores";
                    amu::msg::warn(ss.str());
                    nt = np;
                }
                gv.resetNumeric("NUM_THREAD", nt);
                break;
            }
#endif //_OPENMP

#ifndef USE_MINGW
        case 300:
            // すべての入力を無視する
            // Ignore all input
            gv.resetFlag("FLAG_INPUT_MAP", false);
            gv.resetFlag("FLAG_INPUT_SIGNAL", false);
            gv.resetFlag("FLAG_INPUT_VEHICLE", false);
            break;
        case 301:
            // 地図データ入力を無視
            // Ignore map data input
            gv.resetFlag("FLAG_INPUT_MAP", false);
            break;
        case 302:
            // 信号データ入力を無視
            // Ignore traffic light data input
            gv.resetFlag("FLAG_INPUT_SIGNAL", false);
            break;
        case 303:
            // 車両データ入力を無視
            // Ignore vehicle data input
            gv.resetFlag("FLAG_INPUT_VEHICLE", false);
            break;
#endif //USE_MINGW
        case 'S':
            /*
             * 時系列データ出力を有効化
             *   (advmates-calcのデフォルト設定)
             *
             * Enable time series data output
             *   (default setting for advmates-calc)
             */
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_VEHICLE_D", true);
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_PEDESTRIAN_D", true);
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_SIGNAL_D", true);
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_S", true);
            break;
        case 's':
            /*
             * 時系列データ出力を無効化
             *   (advmates-simのデフォルト設定)
             *
             * Disable time series data output
             *   (default setting for advmates-sim)
             */
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_VEHICLE_D", false);
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_PEDESTRIAN_D", false);
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_SIGNAL_D", false);
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_S", false);
            break;
        case 'L':
            /*
             * 旅行距離，旅行時間の出力を有効化
             *   (advmates-calcのデフォルト設定)
             *
             * Enable trip-distance and trip-time data output
             *   (default setting for advmates-calc)
             */
            gv.resetFlag("FLAG_OUTPUT_TRIP_INFO", true);
            break;
        case 'l':
            /*
             * 旅行距離，時間の出力を無効化
             *   (advmates-simのデフォルト設定)
             *
             * Disable trip-distance and trip-time data output
             *   (default setting for advmates-sim)
             */
            gv.resetFlag("FLAG_OUTPUT_TRIP_INFO", false);
            break;
        case 'M':
            /*
             * 検知器で観測されたデータの出力を有効化
             *   (advmates-calcのデフォルト設定)
             *
             * Enable data output observed by monitors
             *   (default setting for Hadvmates-calc)
             */
            gv.resetFlag("FLAG_OUTPUT_TRAFFIC_COUNTER_D", true);
            gv.resetFlag("FLAG_OUTPUT_TRAFFIC_COUNTER_S", true);
            gv.resetFlag("FLAG_OUTPUT_LINK_TRAFFIC_FLOW_D", true);
            gv.resetFlag("FLAG_OUTPUT_LINK_TRAFFIC_FLOW_S", true);
            gv.setNewFlag("FLAG_OUTPUT_INFLOW_MONITOR", true);
            gv.setNewFlag("FLAG_OUTPUT_CONVOY_MONITOR", true);
            break;
        case 'm':
            /*
             * 検知器で観測されたデータの出力を無効化
             *   (advmates-simのデフォルト設定)
             *
             * Disable data output observed by monitors
             *   (default setting for advmates-sim)
             */
            gv.resetFlag("FLAG_OUTPUT_TRAFFIC_COUNTER_D", false);
            gv.resetFlag("FLAG_OUTPUT_TRAFFIC_COUNTER_S", false);
            gv.resetFlag("FLAG_OUTPUT_LINK_TRAFFIC_FLOW_D", false);
            gv.resetFlag("FLAG_OUTPUT_LINK_TRAFFIC_FLOW_S", false);
            gv.setNewFlag("FLAG_OUTPUT_INFLOW_MONITOR", false);
            gv.setNewFlag("FLAG_OUTPUT_CONVOY_MONITOR", false);
            break;
#ifndef USE_MINGW
        case 200:
            // 時系列データのうち詳細データの出力を有効化
            // Enable detailed output in time series data
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_VEHICLE_D", true);
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_PEDESTRIAN_D", true);
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_SIGNAL_D", true);
            break;
        case 201:
            // 時系列データのうち詳細データの出力を無効化
            // Disable detailed output in time series data
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_VEHICLE_D", false);
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_PEDESTRIAN_D", false);
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_SIGNAL_D", false);
            break;
        case 202:
            // 時系列データのうち統計データの出力を有効化
            // Enable statistical output in time series data
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_S", true);
            break;
        case 203:
            // 時系列データのうち統計データの出力を無効化
            // Disable statistical output in time series data
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_S", false);
            break;
        case 204:
            // 時系列データのうち自動車詳細データの出力を有効化
            // Enable detailed car output in time series data
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_VEHICLE_D", true);
            break;
        case 205:
            // 時系列データのうち自動車詳細データの出力を無効化
            // Disable detailed car output in time series data
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_VEHICLE_D", false);
            break;
        case 206:
            // 時系列データのうち歩行者詳細データの出力を有効化
            // Enable detailed pedestrian output in time series data
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_PEDESTRIAN_D", true);
            break;
        case 207:
            // 時系列データのうち歩行者詳細データの出力を無効化
            // Disable detailed car output in time series data
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_PEDESTRIAN_D", false);
            break;
        case 208:
            // 時系列データのうち信号詳細データの出力を有効化
            // Enable detailed traffic light output in time series data
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_SIGNAL_D", true);
            break;
        case 209:
            // 時系列データのうち信号詳細データの出力を無効化
            // Disable detailed traffic light output in time series data
            gv.resetFlag("FLAG_OUTPUT_TIMELINE_SIGNAL_D", false);
            break;
        case 210:
            // 検知器で観測されたデータのうち詳細データの出力を有効化
            // Enable detailed output observed by monitors
            gv.resetFlag("FLAG_OUTPUT_TRAFFIC_COUNTER_D", true);
            gv.resetFlag("FLAG_OUTPUT_LINK_TRAFFIC_FLOW_D", true);
            gv.setNewFlag("FLAG_OUTPUT_INFLOW_MONITOR", true);
            break;
        case 211:
            // 検知器で観測されたデータのうち詳細データの出力を無効化
            // Disable detailed output observed by monitors
            gv.resetFlag("FLAG_OUTPUT_TRAFFIC_COUNTER_D", false);
            gv.resetFlag("FLAG_OUTPUT_LINK_TRAFFIC_FLOW_D", false);
            gv.setNewFlag("FLAG_OUTPUT_INFLOW_MONITOR", false);
            break;
        case 212:
            // 検知器で観測されたデータのうち統計データの出力を有効化
            // Enable statistical output observed by monitors
            gv.resetFlag("FLAG_OUTPUT_TRAFFIC_COUNTER_S", true);
            gv.resetFlag("FLAG_OUTPUT_LINK_TRAFFIC_FLOW_S", true);
            gv.setNewFlag("FLAG_OUTPUT_CONVOY_MONITOR", true);
            break;
        case 213:
            // 検知器で観測されたデータのうち統計データの出力を無効化
            // Disable statistical output observed by monitors
            gv.resetFlag("FLAG_OUTPUT_TRAFFIC_COUNTER_S", false);
            gv.resetFlag("FLAG_OUTPUT_LINK_TRAFFIC_FLOW_S", false);
            gv.setNewFlag("FLAG_OUTPUT_CONVOY_MONITOR", false);
            break;
#endif //USE_MINGW

        case 'Q':
            // 標準出力上の表示を有効化 (デフォルト設定)
            // Enable verbose output on stdout
            gv.resetFlag("FLAG_VERBOSE", true);
            break;

#ifndef USE_MINGW
        case 214:
            // 標準出力上のさらに詳しい表示を有効化
            // Enable more verbose output on stdout
            gv.resetFlag("FLAG_MORE_VERBOSE", true);
            gv.resetFlag("FLAG_VERBOSE", true);
            break;
        case 400:
            /*
             * 発生交通量の指定されていないODノードにおいてランダムに車両を
             * 発生させない
             *
             * Disable random vehicle generating at OD nodes where generation
             * volume is not given
             */
            gv.resetFlag("FLAG_GEN_RAND_VEHICLE", false);
            break;
        case 401:
            // 各ODノードで車両発生を等間隔に発生させる
            // Generate vehicles at equal intervals at each OD node
            gv.resetFlag("FLAG_GEN_VEHICLE_EQUAL_INTERVAL", true);
            break;
        case 402:
            // ランダムに発生させる車両の発生交通量の係数を指定する
            // Set coefficient of generation volume to be generated randomly
            gv.resetNumeric("RANDOM_OD_FACTOR", atof(optarg));
            break;
        case 403:
            /*
             * 設定ファイルで指定された車両の発生交通量の係数を指定する
             *
             * Set coefficient of generation volume specified in configuration
             * file
             */
            gv.resetNumeric("TABLED_OD_FACTOR", atof(optarg));
            break;
        case 404:
            /*
             * 交差点での衝突を無視し道を譲らないようにする
             *
             * Ignore collisions at intersections and do not yield to other
             * vehicles
             */
            gv.resetFlag("DEBUG_FLAG_IGNORE_YIELDING", true);
            break;
        case 405:
            /*
             * 設定を無視しすべての単路部を片側1車線通行にする．
             *   車線変更の必要がなく，すべての車両が予定経路を達成できる．
             *
             * Ignore settings and make all road segments one lane on each side.
             *   Needless to lane change, and all vehicles can follow the
             *   planned route.
             */
            gv.resetFlag("DEBUG_FLAG_ALL_SECTION_SINGLE_LANE_EACH_SIDE", true);
            break;
        case 406:
            /*
             * 発生時刻が指定された車両の設定を無視しすべての車両を最初の
             * ステップに発生させる．デバッグなどに用いる．
             *
             * Ignore the vehicles setting with the specified generation time
             * and generate all vehicles at the first step. This option can be
             * used for debugging.
             */
            gv.resetFlag("DEBUG_FLAG_GEN_FIXED_VEHICLE_ALL_AT_ONCE", true);
            break;
#endif //USE_MINGW

#ifndef USE_MINGW
        case 500:
            /*
             * 経路探索結果のキャッシュファイルの読み込みを有効化
             * (デフォルト設定)
             *
             * Enable reading of cache files for route search results
             * (default setting)
             */
            gv.resetFlag("FLAG_CACHE_ROUTING_READ", true);
            break;
        case 501:
            // 経路探索結果のキャッシュファイルの読み込みを無効化
            // Disable reading of cache files for route search results
            gv.resetFlag("FLAG_CACHE_ROUTING_READ", false);
            break;
        case 502:
            // 経路探索結果のキャッシュファイルの書き出しを有効化
            // Enable writing of cache files for route search results
            gv.resetFlag("FLAG_CACHE_ROUTING_WRITE", true);
            break;
        case 503:
            /*
             * 経路探索結果のキャッシュファイルの書き出しを無効化
             * (デフォルト設定)
             *
             * Disable writing of cache files for route search results
             * (default setting)
             */
            gv.resetFlag("FLAG_CACHE_ROUTING_WRITE", false);
            break;
        case 504:
            /*
             * 経路探索結果のキャッシュファイルの最大行数を設定する
             * (デフォルトで10,000行)
             *
             * Set the maximum number of lines in the cache file
             * for route search results (10,000 lines by default)
             *
             */
            gv.resetNumeric("VEHICLE_CACHE_ROUTING_SIZE", atof(optarg));
            break;
        case 505:
            /*
             * 経路探索のキャッシュを利用する確率を設定する
             * (デフォルトで1.0)
             *
             * Set the probability of using the cache for route search
             * (1.0 by default)
             */
            gv.resetNumeric("VEHICLE_CACHE_ROUTING_PROBABILITY", atof(optarg));
            break;

#endif //USE_MINGW

        // --------------------------------------------------
        // 2025/03/24 by abe [eMATES]
        case 215: // CSの充電量出力をonに(sim/calcデフォルトでoff)
            gv.resetNumeric("OUTPUT_CS_INTERVAL", true);
            break;

        case 216: // CSの充電量出力をoffに(sim/calcデフォルトでoff)
            gv.resetNumeric("OUTPUT_CS_INTERVAL", false);
            break;

        case 407: // EV発生およびCS設置をonに(sim/calcデフォルトでoff)
            gv.resetFlag("FLAG_GEN_EV", true);
            gv.resetFlag("FLAG_GEN_CS", true);
            break;

        // [eMATES] ここまで
        // --------------------------------------------------

            // 以下はここでは使用しない
            // followings are not used here
        case 'D':
        case 'd':
        case 'q':
        case 2000:
        case 3000:
        case 3001:
        case 3002:
        case 3003:
        case 3010:
        default:
            break;
        }
    }

    // 派生クラスの_parseArgument()を呼ぶ
    // call _parseArgument() in the derived class
    optind = 1;
    _parseArgument(argc, argv);

    amu::msg::status(cout, "parse arguments ... done");
}

//==============================================================================
void AppMates::_printUsage()
{
    ostringstream oss;
    oss << "Options:\n"
           " -t                 : "
           "time to stop calculation\n"
           "                      "
           "This must be multiple number of time step.\n"
           "                      "
           "(Time step default is 100 milliseconds.)\n"
           "                      "
           "If this isn't given, simulator will use default value.\n"
           " -d <DataDir>       : "
           "set root path of input and output directory.\n"
           "                      "
           "(default: current directory)\n"
           " -r <Number>        : "
           "set random seed.\n"
           "                      "
           "(default: variable number on account of present time)\n"
           " -s                 : "
           "do not output timeline data\n"
           " -m                 : "
           "do not output monitoring data\n"
#ifndef USE_MINGW
           " --no-output-monitor-d\n"
           "                    : "
           "do not output monitoring data(detail)\n"
           " --no-output-monitor-s\n"
           "                    : "
           "do not output monitoring data(statistic)\n"
#endif
           " -g                 : "
           "do not output generate counter data\n"
           " -l                 : "
           "do not output trip info\n"
#ifndef USE_MINGW
           " --no-input-signal  : "
           "do not read input files for signals.\n"
           "                      "
           "All signals show blue sign.\n"
           " --no-input-vehicle : "
           "do not read input files for vehicle generation.\n"
           " --no-generate-random-vehicle\n"
           "                    : "
           "do not generate vehicles without input data.\n"
#endif //USE_MINGW
#ifdef USE_MINGW
           " -q                 : "
           "hide detail information.\n"
#else  //USE_MINGW not defined
           " -q \n"
           " [or --no-verbose]  : "
           "hide detail information.\n"
#endif //USE_MINGW
        ;
    amu::msg::message(cout, oss.str());
}

//==============================================================================
bool AppMates::_checkQuietOption(int argc, char** argv)
{
    int opt;
    opterr = 0;
    getGVManager().setNewFlag("FLAG_VERBOSE", true);

#ifdef USE_MINGW
    while ((opt = getopt(argc, argv, shortOptions.c_str())) != -1)
#else  //USE_MINGW not defined

    while ((opt = getopt_long(
                argc, argv, shortOptions.c_str(), longOptions, &optionIndex))
           != -1)
#endif //USE_MINGW
    {
        switch (opt)
        {
            // 標準出力上の表示を無効化
            // Disable verbose output on stdout
        case 'q':
            getGVManager().resetFlag("FLAG_VERBOSE", false);
            break;
        default:
            break;
        }
    }
    optind = 1;
    return true;
}

//==============================================================================
bool AppMates::_setDataPath(int argc, char** argv)
{
    string path;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * ./dataPath.txtの読み込み
     * - コマンドラインから起動しない場合の代替措置として用意．
     * - このファイルのみ，実行ディレクトリ限定で読み込む．
     *
     * Read ./dataPath.txt
     * - This procedure is provided as an alternative when simulator is not
     *   invoked from the command line.
     * - Only this file is loaded from the execution directory.
     */
    ifstream ifs("./dataPath.txt", ios::in);
    if (ifs)
    {
        string str;
        while (ifs.good())
        {
            getline(ifs, str);
            getAdjustString(&str);
            if (!str.empty())
            {
                vector<string> tokens;
                getTokens(&tokens, str, '=');
                if (tokens.size() != 2)
                {
                    continue;
                }

                // キー"DATA_DIRECTORY"のみを処理する
                // Handle only the key "DATA_DIRECTORY.
                if (tokens[0] == "DATA_DIRECTORY" && !tokens[1].empty())
                {
                    if (tokens[1][tokens[1].length() - 1] != '/')
                    {
                        tokens[1] += '/';
                    }
                    _dataPath = tokens[1];
                }
            }
        }
        ifs.close();
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // コマンドラインオプションの処理 (_dataPathをこれにより上書きする)
    // Handle command line option (overwrite _dataPath by this)
    int opt;
    opterr = 0;

#ifdef USE_MINGW
    while ((opt = getopt(argc, argv, shortOptions.c_str())) != -1)
#else  //USE_MINGW not defined

    while ((opt = getopt_long(
                argc, argv, shortOptions.c_str(), longOptions, &optionIndex))
           != -1)
#endif //USE_MINGW
    {
        switch (opt)
        {
        case 'D':
        case 'd':
            path = optarg;
            if (!path.empty())
            {
                if (path[path.length() - 1] != '/')
                {
                    path += '/';
                }
                _dataPath = path;
            }
            break;
        default:
            break;
        }
    }
    optind = 1;
    return true;
}

//==============================================================================
bool AppMates::_setRandomSeed(const string& arg)
{
    if (!arg.empty())
    {
        _key = static_cast<unsigned int>(stoul(arg));
        return true;
    }
    else
    {
        return false;
    }
}

//==============================================================================
void AppMates::getReadySimulator()
{
    if (_simulator)
    {
        delete _simulator;
        _simulator = nullptr;
    }
    _simulator = new Simulator();

    if (getGVManager().getFlag("FLAG_INPUT_MAP"))
    {
        _simulator->getReadyRoadEnvironment();
        _simulator->getReadyMonitors();

#ifdef INCLUDE_VEHICLES
        _simulator->getReadyVehicles();
#endif //INCLUDE_VEHICLES

#ifdef INCLUDE_PEDESTRIANS
        _simulator->pedExt()->getReadyPedestrians();
#endif //INCLUDE_PEDESTRIANS
    }
    else
    {
        /**
         * @~japanese @todo サンプルシナリオの実装
         */
        cerr << "ERROR: sample scenario not implemented yet." << endl;
        exit(EXIT_FAILURE);
    }

    _simulator->writeInitializedRoadMap();
}

//==============================================================================
void AppMates::printGlobalSetting(bool isFlagOutput)
{
    GVManager& gv = getGVManager();
    if (!(gv.getFlag("FLAG_VERBOSE")))
    {
        return;
    }
    bool isMoreVerbose = gv.getFlag("FLAG_MORE_VERBOSE");

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 道路ネットワーク
    // Road network
    RoadMap* roadMap = _simulator->roadMap();
    roadMap->printMapSimple(cout);
    if (isMoreVerbose)
    {
        roadMap->printMapDetail(cout);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 経路探索用ネットワーク
    // Map for route search
    getRouterManager().printRoutingNetwork(cout);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 車種
    // Vehicle type
    getVehicleTypeManager().print(cout);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 車両生成器
    // Vehicle generator
    VehicleGenerator* generator = _simulator->vehicleGenerator();
    generator->printExcludedRandomStarts(cout);
    generator->printExcludedRandomGoals(cout);
    generator->printODGroup(cout);
    generator->printVehicleRoutingParams(cout);
    generator->printVehicleRoutingPrefRank(cout);

#ifdef INCLUDE_TRAMS
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 路面電車の路線
    // Tram lines
    getTramRouteManager().print(cout);
#endif //INCLUDE_TRAMS

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 観測機器
    // Observation device
    ObjectManager& obj = getObjectManager();
    obj.printTrafficCounters(cout);
    obj.printLinkFlowMonitors(cout);
    obj.printInflowMonitors(cout);
    obj.printConvoyMonitors(cout);
#ifdef INCLUDE_PEDESTRIANS
    obj.printInflowPedestrianMonitors(cout);
#endif //INCLUDE_PEDESTRIANS

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // グローバル変数
    // Global variables
    if (isMoreVerbose)
    {
        gv.print(cout);
    }

    if (isFlagOutput && gv.getFlag("FLAG_VERBOSE"))
    {
        ostringstream oss;
        oss << "time to stop calculation    : " << gv.getMaxTime() << " ms"
            << endl;
        oss << "output timeline car detail data           : "
            << (gv.getFlag("FLAG_OUTPUT_TIMELINE_VEHICLE_D") ? "true" : "false")
#ifdef INCLUDE_PEDESTRIANS
            << "\noutput timeline pedestrian detail data    : "
            << (gv.getFlag("FLAG_OUTPUT_TIMELINE_PEDESTRIAN_D") ? "true"
                                                                : "false")
#endif //INCLUDE_PEDESTRIANS
            << "\noutput timeline traffic light detail data : "
            << (gv.getFlag("FLAG_OUTPUT_TIMELINE_SIGNAL_D") ? "true" : "false")
            << "\noutput timeline statistic data            : "
            << (gv.getFlag("FLAG_OUTPUT_TIMELINE_S") ? "true" : "false")
            << "\noutput monitoring detail data             : "
            << (gv.getFlag("FLAG_OUTPUT_TRAFFIC_COUNTER_D") ? "true" : "false")
            << "\noutput monitoring statistic data          : "
            << (gv.getFlag("FLAG_OUTPUT_TRAFFIC_COUNTER_S") ? "true" : "false")
            << "\noutput inflow monitor data                : "
            << (gv.getFlag("FLAG_OUTPUT_INFLOW_MONITOR") ? "true" : "false")
            << "\noutput trip distance and trip time        : "
            << (gv.getFlag("FLAG_OUTPUT_TRIP_INFO") ? "true" : "false");
        amu::msg::message(cout, oss.str());
    }
}
