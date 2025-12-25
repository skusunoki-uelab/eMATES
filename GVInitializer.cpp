/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file GVInitializer.cpp
 */
#include "GVInitializer.hpp"
#include "AppMates.hpp"
#include "GVManager.hpp"
#include <iostream>
#include <sstream>

using namespace std;

//==============================================================================
void GVInitializer::initialize(const string& dataPath, unsigned int loopNum)
{
    GVManager& gv = AppMates::getGVManager();

    gv.setNewString("DATA_DIRECTORY", dataPath);


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // フラグの定義
    // Define flags

    // 詳細情報を出力する FLAG_VERBOSE オプションは別に設定
    // FLAG_VERBOSE option to display detailed information is set separately
    // gv.setNewFlag("FLAG_VERBOSE", true);

    // さらに詳しい情報を出力するか
    // Whether to display more detailed information
    gv.setNewFlag("FLAG_MORE_VERBOSE", false);

    // 地図を入力するか
    // Whether to input map data
    gv.setNewFlag("FLAG_INPUT_MAP", true);

    // 信号情報を入力するか
    // Whether to input traffic light data
    gv.setNewFlag("FLAG_INPUT_SIGNAL", true);
    
    // 車両情報を入力するか
    // Whether to input vehicle data
    gv.setNewFlag("FLAG_INPUT_VEHICLE", true);

    /*
     * 発生交通量の指定されていないODノードにおいてランダムに車両を発生させるか
     *
     * Whether to generate vehicle randomly at OD nodes where generation volume
     * is not given
     */
    gv.setNewFlag("FLAG_GEN_RAND_VEHICLE", true);

    // 各ODノードで車両発生を等間隔に発生させるか
    // Whether to generate vehicles at equal intervals at each OD node
    gv.setNewFlag("FLAG_GEN_VEHICLE_EQUAL_INTERVAL", false);

    // ファイルにコメントを付して出力するか
    // Whether to output the file with comments
    gv.setNewFlag("FLAG_OUTPUT_COMMENT_IN_FILE", false);

    // 自動車の時系列詳細データを出力するか
    // Whether to output detailed car time series data
    gv.setNewFlag("FLAG_OUTPUT_TIMELINE_VEHICLE_D", true);

    // 歩行者の時系列詳細データを出力するか
    // Whether to output detailed pedestrian time series data
    gv.setNewFlag("FLAG_OUTPUT_TIMELINE_PEDESTRIAN_D", true);

    // 信号の時系列詳細データを出力するか
    // Whether to output detailed traffic light time series data
    gv.setNewFlag("FLAG_OUTPUT_TIMELINE_SIGNAL_D", true);

    // 時系列統計データを出力するか
    // Whether to output statistical time series data
    gv.setNewFlag("FLAG_OUTPUT_TIMELINE_S", true);

    // 車両感知器の詳細データを出力するか
    // Whether to output detailed observed data by traffic counters
    gv.setNewFlag("FLAG_OUTPUT_TRAFFIC_COUNTER_D", true);

    // 車両感知器の統計データを出力するか
    // Whether to output statistical observed data by traffic counters
    gv.setNewFlag("FLAG_OUTPUT_TRAFFIC_COUNTER_S", true);

    // リンク交通流観測器の詳細データを出力するか
    // Whether to output detailed observed data by link traffic flow monitors
    gv.setNewFlag("FLAG_OUTPUT_LINK_FLOW_MONITOR_D", true);

    // リンク交通流観測器の統計データを出力するか
    // Whether to output statistical observed data by link traffic flow monitors
    gv.setNewFlag("FLAG_OUTPUT_LINK_FLOW_MONITOR_S", true);

    // エージェント流入観測器のデータを出力するか
    // Whether to output inflow agent monitor
    gv.setNewFlag("FLAG_OUTPUT_INFLOW_MONITOR", true);

    // 車列最後尾情報を出力するか
    // Whether to output rearmost vehicle data in convoy
    gv.setNewFlag("FLAG_OUTPUT_CONVOY_MONITOR", true);

    // 旅行距離，旅行時間を出力するか
    // Whether to output trip-distance and trip time
    gv.setNewFlag("FLAG_OUTPUT_TRIP_INFO", true);

    // 経路探索のキャッシュを読み込むか
    // Whether to read cache files for route search results
    gv.setNewFlag("FLAG_CACHE_ROUTING_READ", true);

    // 経路探索のキャッシュを書き出すか
    // Whether to write cache files for route search results
    gv.setNewFlag("FLAG_CACHE_ROUTING_WRITE", false);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // フラグの定義 (デバッグ用)
    // Define flags (for debug)

    /*
     * 設定を無視しすべての単路部を片側1車線通行にするかどうか
     *
     * Whether to ignore settings and make all road segments one lane on each
     * side.
     */
    gv.setNewFlag("DEBUG_FLAG_ALL_SECTION_SINGLE_LANE_EACH_SIDE", false);

    /*
     * 発生時刻が指定された車両の設定を無視し，すべての車両を最初のステップに
     * 発生させるか．
     *
     * Whether to ignore the vehicles setting with the specified generation time
     * and generate all vehicles at the first step.
     */
    gv.setNewFlag("DEBUG_FLAG_GEN_FIXED_VEHICLE_ALL_AT_ONCE", false);

    /*
     * 交差点での衝突を無視し道を譲らないようにするか
     *
     * Whether to ignore collisions at intersections and not to yield to other
     * vehicles
     */
    gv.setNewFlag("DEBUG_FLAG_IGNORE_YIELDING", false);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 入出力ファイルの設定
    // Setting for input files

    // グローバル変数設定用ファイル
    // Input file for global variable setting
    gv.setNewString("GV_INIT_FILE", dataPath + "init.txt");

    //--------------------------------------------------------------------------
    // 地図に関する入力ファイル
    // Input files related to road map
    gv.setNewString("MAP_POSITION_FILE", dataPath + "mapPosition.txt");
    gv.setNewString("MAP_NETWORK_FILE", dataPath + "network.txt");
    gv.setNewString("OD_NODE_EXCLUSION_FILE", dataPath + "odNodeExclusion.txt");
    gv.setNewString(
        "TRAFFIC_CONTROL_SECTION_FILE", dataPath + "trafficControlSection.txt");
    gv.setNewString(
        "TRAFFIC_CONTROL_INTERSECTION_FILE",
        dataPath + "trafficControlIntersection.txt");
    gv.setNewString("SPEED_LIMIT_FILE", dataPath + "speedLimit.txt");
    // 2025/03/24 by abe [eMATES]
    gv.setNewString("CS_LIST_FILE", dataPath + "csList.txt");

    //--------------------------------------------------------------------------
    // 交差点属性指定ファイル用ディレクトリ
    // Directory for intersection attribute specification file
    gv.setNewString(
        "INTERSECTION_ATTRIBUTE_DIRECTORY", dataPath + "intersection/");

    //--------------------------------------------------------------------------
    // 道路形状に関するファイル
    // Files for intersection and section structures
    gv.setNewString(
        "INTERSECTION_STRUCT_FILE", dataPath + "intersectionStruct.txt");
    gv.setNewString("SECTION_STRUCT_FILE", dataPath + "sectionStruct.txt");

    //--------------------------------------------------------------------------
    // 信号に関するディレクトリorファイルor拡張子
    // Directories, files and extensions for traffic lights
    gv.setNewString("SIGNAL_CONTROL_DIRECTORY", dataPath + "signals/");
    gv.setNewString("SIGNAL_CONTROL_FILE_DEFAULT", "default");
    gv.setNewString("SIGNAL_ASPECT_FILE_DEFAULT_PREFIX", "defaultInter");
    gv.setNewString("CONTROL_FILE_EXTENSION", ".msf");
    gv.setNewString("ASPECT_FILE_EXTENSION", ".msa");
    gv.setNewString(
        "UNSIGNALIZED_INTERSECTION_FILE",
        dataPath + "unsignalizedIntersection.txt");

    //--------------------------------------------------------------------------
    // 車両発生に関する入力ファイル
    // Input file for vehicle generation
    gv.setNewString("GENERATE_TABLE", dataPath + "generateTable.txt");
    gv.setNewString(
        "DEFAULT_GENERATE_TABLE", dataPath + "defaultGenerateTable.txt");
    gv.setNewString(
        "FIXED_GENERATE_TABLE", dataPath + "fixedGenerateTable.txt");
    gv.setNewString(
        "GROUPED_GENERATE_TABLE", dataPath + "groupedGenerateTable.txt");

    // ODノードのグループを定義する入力ファイル
    // Input file to give OD node groups
    gv.setNewString("OD_GROUP_FILE", dataPath + "odGroup.txt");

    // 車種に関する入力ファイル
    // Input file to give vehicle type
    gv.setNewString("VEHICLE_FAMILY_FILE", dataPath + "vehicleFamily.txt");

    // 自動車の経路探索の効用のパラメータに関する入力ファイル
    // Input file for parameters of vehicle routing utility
    gv.setNewString(
        "VEHICLE_ROUTE_PARAM_FILE", dataPath + "vehicleRoutingParam.txt");

    // 自動車の経路探索の選好ランクに関するファイル
    // Input file for preference rank of vehicle routing
    gv.setNewString(
        "VEHICLE_ROUTE_PREFRANK_FILE", dataPath + "vehicleRoutingPrefRank.txt");

    // 自動車の経路選択確率に関するファイル
    // Input file for selection probabilities of vehicles routing
    gv.setNewString(
        "ROUTING_PROBABILITY_FILE", dataPath + "vehicleRoutingProbability.txt");

    //--------------------------------------------------------------------------
    // 検知器に関する入力ファイル
    // Input files for monitors
    gv.setNewString("TRAFFIC_COUNTER_FILE", dataPath + "trafficCounter.txt");
    gv.setNewString("LINK_FLOW_MONITOR_FILE", dataPath + "linkFlowMonitor.txt");
    gv.setNewString("INFLOW_MONITOR_FILE", dataPath + "inflowMonitor.txt");
    gv.setNewString("CONVOY_MONITOR_FILE", dataPath + "convoyMonitor.txt");

#ifdef INCLUDE_PEDESTRIANS
    gv.setNewString(
        "PEDESTRIAN_INFLOW_MONITOR_FILE",
        dataPath + "pedestrianInflowMonitor.txt");
#endif //INCLUDE_PEDESTRIANS

#ifdef INCLUDE_TRAMS
    //--------------------------------------------------------------------------
    // 路面電車路線を定義するファイル
    // File to give tram lines
    gv.setNewString("TRAM_LINE_FILE", dataPath + "tramLine.txt");
#endif //INCLUDE_TRAMS

#ifdef INCLUDE_PEDESTRIANS
    //--------------------------------------------------------------------------
    // 歩行者発生に関する入力ファイル
    // Input file for pedestrian generation
    gv.setNewString(
        "GENERATE_PEDESTRIAN_FILE", dataPath + "pedestrianGenerateTable.txt");
#endif //INCLUDE_PEDESTRIANS

    //--------------------------------------------------------------------------
    // 出力ディレクトリ，ファイル名，ファイルのプレフィクス，拡張子
    // Output directories, files, file prefixes, extensions
    string resultPath = dataPath + "result";
    if (loopNum > 1)
    {
        /*
         * loopNum (=1, 2, ...) 回目のループの結果を "result[loopNum]/"
         * ディレクトリに出力する．ただし1回目のループのみ番号を付けず
         * "result/" ディレクトリとする．
         *
         * Output the result of the loopNum-th (1, 2...) loop to the directory
         * "result[loopNum]/". However, only for the result of first loop is not
         * numbered and is output to the directory "result/".
         */
        resultPath += to_string(loopNum);
    }
    resultPath += "/";

    gv.setNewString("RESULT_OUTPUT_DIRECTORY", resultPath);
    gv.setNewString("RESULT_TIMELINE_DIRECTORY", resultPath + "timeline/");
    gv.setNewString("RESULT_IMG_DIRECTORY", resultPath + "img/");
    gv.setNewString("RESULT_INSTRUMENT_DIRECTORY", resultPath + "inst/");
    gv.setNewString("RESULT_TRAFFIC_COUNTER_D_PREFIX", "trcd");
    gv.setNewString("RESULT_TRAFFIC_COUNTER_S_PREFIX", "trcs");
    gv.setNewString("RESULT_LINK_FLOW_MONITOR_D_PREFIX", "lnfd");
    gv.setNewString("RESULT_LINK_FLOW_MONITOR_S_PREFIX", "lnfs");
    gv.setNewString("RESULT_INFLOW_MONITOR_PREFIX", "iflw");
    gv.setNewString("RESULT_CONVOY_MONITOR_PREFIX", "cnvy");
    gv.setNewString("RESULT_RUN_INFO_FILE", resultPath + "runInfo.txt");
    gv.setNewString("RESULT_NODE_SHAPE_FILE", resultPath + "nodeShape.txt");
    gv.setNewString("RESULT_LINK_SHAPE_FILE", resultPath + "linkShape.txt");
    gv.setNewString("RESULT_SIGNAL_COUNT_FILE", resultPath + "signalCount.txt");
    gv.setNewString(
        "RESULT_VEHICLE_ATTRIBUTE_FILE", resultPath + "vehicleAttribute.txt");
    gv.setNewString("RESULT_VEHICLE_TRIP_FILE", resultPath + "/emates/" + "vehicleTrip.txt");
    gv.setNewString("RESULT_WAITINGLINE_FILE", resultPath + "/emates/" + "waitingLine.txt");
    gv.setNewString(
        "RESULT_VEHICLE_COUNT_FILE", resultPath + "vehicleCount.txt");
    gv.setNewString("RESULT_PEDESTRIAN_INFLOW_MONITOR_PREFIX", "piflw");
    gv.setNewString(
        "RESULT_PEDESTRIAN_COUNT_FILE", resultPath + "pedestrianCount.txt");

    //--------------------------------------------------------------------------
    // ログデータ出力ディレクトリ
    // Directory for log data
    string logPath = dataPath + "log/";
    gv.setNewString("LOG_OUTPUT_DIRECTORY", logPath);
    gv.setNewString("LOG_FILE_EXTENSION", ".log");

    //--------------------------------------------------------------------------
    // キャッシュデータ入出力ディレクトリ
    // Directory for cache data

    // 前のシミュレーションの出力結果を次のシミュレーションの入力とする
    // Use output of the previous sim. as input for the next sim.
    string cachePath = dataPath + "cache/";
    gv.setNewString("CACHE_INOUT_DIRECTORY", cachePath);
    gv.setNewString("CACHE_ROUTING_FILE", cachePath + "route.txt");

    //--------------------------------------------------------------------------
    // [eMATES] openDSS連成用ディレクトリ
    gv.setNewString("EV_COMM_COMMON_DIRECTORY", resultPath);
    gv.setNewString("EV_COMM_SCENARIO_NAME", "20220101");
    gv.setNewString("EV_COMM_OPENDSS_DIRECTORY_NAME", "opendss");
    gv.setNewString("EV_COMM_EMATES_DIRECTORY_NAME", "emates");
    gv.setNewString("EV_COMM_TRAFFIC_PREFIX", "T");
    gv.setNewString("EV_COMM_ENERGY_PREFIX", "E");
    gv.setNewString("EV_COMM_ARRIVAL_PREFIX", "arrivelist_");
    gv.setNewString("EV_COMM_STATE_PREFIX", "statelist_");
    gv.setNewString("EV_COMM_ERROR_FILE", "error.csv");

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 定数
    // Constants

    // 繰り返し回数
    // Number of repetitions
    gv.setNewNumeric("NUM_LOOPS", 1);

    //--------------------------------------------------------------------------
    // エージェントに関する定数
    // Constants for agents

    // 自動車の反応遅れ [s]
    // Vehicles reaction delay [s]
    gv.setNewNumeric("REACTION_TIME_VEHICLE", 0.74);

    // 自動車転回時の対向車とのギャップアクセプタンス [s]
    // Vehicles gap acceptance with oncoming vehicle at turning [s]
    gv.setNewNumeric("GAP_ACCEPTANCE_VEHICLE_CROSS", 3.0);

    // 自動車が車線変更可能かどうか判断する閾値 [m]
    // Vehicles threshold for judging whether to shift lanes [m]
    gv.setNewNumeric("THRESHOLD_VEHICLE_LANESHIFT", 50);

    // 普通車(PASSENGER)の最大加速度，減速度 [m/(s^2)]
    // PASSENGERs maximum acceleration and deceleration [m/(s^2)]
    gv.setNewNumeric("MAX_ACCELERATION_PASSENGER", 3.0);
    gv.setNewNumeric("MAX_DECELERATION_PASSENGER", -5.0);

    // 普通車(PASSENGER)の停車時の先行車との車間距離 [m]
    // PASSENGERs gap from preceding car when stopped [m]
    gv.setNewNumeric("JAM_DISTANCE_PASSENGER", 2.0);

    // バス(BUS)の最大加速度，減速度 [m/(s^2)]
    // BUSes maximum acceleration and deceleration [m/(s^2)]
    gv.setNewNumeric("MAX_ACCELERATION_BUS", 3.0);
    gv.setNewNumeric("MAX_DECELERATION_BUS", -5.0);

    // バス(BUS)の停車時の先行車との車間距離 [m]
    // BUSess gap from preceding car when stopped [m]
    gv.setNewNumeric("JAM_DISTANCE_BUS", 4.0);

    // 大型車(TRUCK)の最大加速度，減速度[m/(sec^2)]
    // TRUCKs maximum acceleration and deceleration [m/(s^2)]
    gv.setNewNumeric("MAX_ACCELERATION_TRUCK", 3.0);
    gv.setNewNumeric("MAX_DECELERATION_TRUCK", -5.0);

    // 大型車(TRUCK)の停車時の先行車との車間距離 [m]
    // TRUCKs gap from preceding car when stopped [m]
    gv.setNewNumeric("JAM_DISTANCE_TRUCK", 4.0);

    // 車線変更時に与える横向きの速度 [km/h]
    // Vehicles lateral speed when shifting lanes [km/h]
    gv.setNewNumeric("ERROR_VELOCITY", 10.0);

    // 描画・計測対象としない車両の発生端点からの範囲 [m]
    // Range from the origin of vehicles not to be drawn/measured [m]
    gv.setNewNumeric("NO_OUTPUT_LENGTH_FROM_ORIGIN_NODE", 5);

    // 車両の時系列詳細データに追加する情報のレベル
    // Information level to add to vehicles detailed time series data
    gv.setNewNumeric("ADDITIONAL_VEHICLE_DATA_LEVEL", 0);

    // 普通車(PASSENGER)の標準サイズ [m]
    // PASSENGERs standard body size [m]
    gv.setNewNumeric("VEHICLE_LENGTH_PASSENGER", 4.400);
    gv.setNewNumeric("VEHICLE_WIDTH_PASSENGER", 1.830);
    gv.setNewNumeric("VEHICLE_HEIGHT_PASSENGER", 1.315);

    // バス(BUS)の標準サイズ [m]
    // BUSes standard body size [m]
    gv.setNewNumeric("VEHICLE_LENGTH_BUS", 8.465);
    gv.setNewNumeric("VEHICLE_WIDTH_BUS", 2.230);
    gv.setNewNumeric("VEHICLE_HEIGHT_BUS", 3.420);

    // 大型車(TRUCK)の標準サイズ [m]
    // TRUCKs standard body size
    gv.setNewNumeric("VEHICLE_LENGTH_TRUCK", 8.465);
    gv.setNewNumeric("VEHICLE_WIDTH_TRUCK", 2.230);
    gv.setNewNumeric("VEHICLE_HEIGHT_TRUCK", 3.420);

    // ランダムに発生させる車両の発生交通量の係数
    // Coefficient of volume to be generated randomly
    gv.setNewNumeric("RANDOM_OD_FACTOR", 1.0);

    // 設定ファイルで指定された車両の発生交通量の係数
    // Coefficient of generation volume given in configuration file
    gv.setNewNumeric("TABLED_OD_FACTOR", 1.0);

    // 交錯を厳密に評価
    // Whether to check vehicles collision
    gv.setNewFlag("STRICT_COLLISION_CHECK", true);

    // 速度履歴を保存するか
    // Whether to save speed history
    gv.setNewFlag("VEHICLE_VELOCITY_HISTORY_RECORD", true);
    // 速度履歴を保存するステップ数
    // Number of steps to save speed history
    gv.setNewNumeric("VEHICLE_VELOCITY_HISTORY_SIZE", 180);
    // 速度履歴を保存するステップ間隔
    // Step interval to save speed history
    gv.setNewNumeric("VEHICLE_VELOCITY_HISTORY_INTERVAL", 10);

    // 経路探索結果のキャッシュサイズ [lines]
    // Size of route search result cache [lines]
    gv.setNewNumeric("VEHICLE_CACHE_ROUTING_SIZE", 10000);
    // 経路探索結果のキャッシュを利用する確率
    // Probability of using route search result cache
    gv.setNewNumeric("VEHICLE_CACHE_ROUTING_PROBABILITY", 1.0);

    //--------------------------------------------------------------------------
    /* 道路に関するもの */

    // 右折専用レーンの標準長さ [m]
    // Standard length of right turn lane [m]]
    gv.setNewNumeric("RIGHT_TURN_LANE_LENGTH", 30);

    /*
     * 標準制限速度 [km/h]
     *   ただし SPEED_LIMIT_INTERSECTION が用いられることはほとんど無く，右左折
     *   時は下のVELOCITY_AT〜が使われ，直進時は次のセクションの SPEED_LIMITが
     *   参照される．
     *
     * Vehicles standard speed limit [km/h]
     *   However, Speed_LIMIT_INTERSECTION is rarely used. When turning left or
     *   right, VELOCITY_AT_... below is used, and when going straight,
     *   SPEED_LIMIT at next section is referenced.
     */
    gv.setNewNumeric("SPEED_LIMIT_SECTION", 60);
    gv.setNewNumeric("SPEED_LIMIT_INTERSECTION", 60);

    // 徐行速度 [km/h]
    // Vehicles creep speed [km/h]
    gv.setNewNumeric("VELOCITY_CREEP", 10);

    // 右左折時の速度 [km/h]
    // Vehicles speed when turning [km/h]
    gv.setNewNumeric("VELOCITY_AT_TURNING_RIGHT", 20);
    gv.setNewNumeric("VELOCITY_AT_TURNING_LEFT", 20);

    // 車両発生時の制限速度 [km/h]，負なら制限なし
    // Vehicles speed limit when generated [km/h], no limit if negative
    gv.setNewNumeric("GENERATE_VELOCITY_LIMIT", -1);

    // 右左折時の最小ヘッドウェイ [s]
    // Vehicles minimum headway when turning [s]
    gv.setNewNumeric("MIN_HEADWAY_AT_TURNING", 1.7);

    /*
     * 標準の発生交通量 [/h]
     *   WIDE: 3レーン以上, NORMAL:2レーン，NARROW:1レーン．基本交通容量の10%と
     *   している．
     *
     * Standard generation volume
     *   WIDE: 3 or more lanes, NORMAL: 2 lanes, NARROW: 1 lane. They are 10% of
     *   basic traffic capacity.
     */
    gv.setNewNumeric("DEFAULT_TRAFFIC_VOLUME_WIDE", 660);
    gv.setNewNumeric("DEFAULT_TRAFFIC_VOLUME_NORMAL", 440);
    gv.setNewNumeric("DEFAULT_TRAFFIC_VOLUME_NARROW", 125);

    // 標準のレーン幅、歩道幅、横断歩道幅、路肩幅 [m]
    // Standard lane, sidewalk, crosswalk, road-shoulder width [m]
    gv.setNewNumeric("DEFAULT_LANE_WIDTH", 3.5);
    gv.setNewNumeric("DEFAULT_SIDEWALK_WIDTH", 5.0);
    gv.setNewNumeric("DEFAULT_CROSSWALK_WIDTH", 5.0);
    gv.setNewNumeric("DEFAULT_ROADSIDE_WIDTH", 1.0);

    /*
     * セクションに歩道を自動設定する際のレーン数
     *   セクションの合計レーン数がこの値以上なら歩道を自動生成する．-1なら自動
     *   設定しない．
     *
     * Number of lanes when automatically setting sidewalks to sections
     *   Attach sidewalk automatically if the total number of lanes in section
     *   is greater than or equal to this value. Not attach automatically if
     *   negative.
     */
    gv.setNewNumeric("AUTO_SIDEWALK_SECTION_LANE", -1);

    /*
     * 道路エンティティの厳密な内外判定を行うか
     *   非凸の道路エンティティに対応するが処理速度は遅い．
     *
     * Whether to judge inside/outside of road entities strictly
     *   Valid if 1. It supports non-convex road entities, but  processing speed
     *   is slow.
     */
    gv.setNewFlag("ROAD_ENTITY_STRICT_JUDGE_INSIDE", true);

    // 交差点サイズの上限 (交差点からの距離) [m]
    // Upper limit of intersection size (distance from center) [m]
    gv.setNewNumeric("INTERSECTION_SIZE_LIMIT", 20);

    /*
     * 無信号交差点におけるリンク交通流観測間隔 [s]
     *   経路探索におけるリンクの予想旅行時間に反映される．信号交差点では
     *   1サイクルごとに更新される．
     *
     * Link traffic flow observation interval at unsignalized intersections [s]
     *   Reflected in the expected link travel time used in route searching. At
     *   signalized intersections, they are updated every cycle.
     */
    gv.setNewNumeric("INTERVAL_RENEW_LINK_FLOW", 120);

    // 最後尾情報の出力間隔 [s]
    // Output interval of rearmost vehicle data [s]
    gv.setNewNumeric("INTERVAL_CONVOY_MONITOR", 30);

#ifdef INCLUDE_TRAMS
    // 路面電車の制限速度 [km/h]
    // Trams speed limit [km/h]
    gv.setNewNumeric("TRAM_SPEED_LIMIT", 30);
#endif //INCLUDE_TRAMS

#ifdef _OPENMP
    /*
     * スレッド数
     *   0の場合はAppMates::initの中でomp_get_num_procs()で リセットする (使用
     *   可能な最大コア数を用いる)
     *
     * Number of threads
     *   If 0, reset with omp_get_num_procs() in AppMates::init() (use max cores
     *   available)
     */
    gv.setNewNumeric("NUM_THREAD", 0);
#else //_OPENMP not defined

    // シングルスレッドで実行するのでスレッド数は1
    // Set 1, since it is executed with a single thread.
    gv.setNewNumeric("NUM_THREAD", 1);

#endif //_OPENMP

    //------------------------------------------------------------------
    /* EVに関するもの  2025/03/24 by abe [eMATES] */

    // EVを発生するか
    // Whether to generate EVs
    gv.setNewFlag("FLAG_GEN_EV", false);

    // CSを配置するか
    // Whether to locate CSs
    gv.setNewFlag("FLAG_GEN_CS", false);

    // CS出力の集計間隔 [ms]
    // Aggregate interval of charge amount values in CSs [ms]
    gv.setNewNumeric("OUTPUT_CS_INTERVAL", 600 * 1000);

    // CSの待ち状況を確認できる車両の割合
    gv.setNewNumeric("RECEIVE_WAITING_INFO_RATE", 0.00);

    // by abe 2022/2/25
    // eMATEE-openDSS 連成用情報を入出力するか
    gv.setNewFlag("FLAG_EV_COMM", false);
    // eMATES->openDSS 出力間隔（msec）
    gv.setNewNumeric("EV_COMM_OUTPUT_INTERVAL", 60000);
    // openDSS->eMATES 入力間隔（msec）
    gv.setNewNumeric("EV_COMM_INPUT_INTERVAL", 60000);
    // eMATES-openDSS 連成時にやり取りするファイルの、時間スロットの数
    gv.setNewNumeric("EV_COMM_PREDICTION_TIME_SLOT", 3);
    // eMATES-openDSS 連成時にやり取りするファイルの、スロット時間（分）
    gv.setNewNumeric("EV_COMM_PREDICTION_SLOT_TIME_MINUTE", 30);
    // eMATES-openDSS 連成時のタイムアウト時間（秒）
    gv.setNewNumeric("EV_COMM_TIMEOUT_SECOND", 5*60);

}
