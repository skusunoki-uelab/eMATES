/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleGeneratorBuilder.hpp
 */
#ifndef __VEHICLE_GENERATOR_BUILDER_HPP__
#define __VEHICLE_GENERATOR_BUILDER_HPP__
#include "../Config.hpp"
#include "../VehicleGenerator.hpp"
#include "../VehicleType.hpp"
#include <string>
#include <vector>

class RoadMap;
class GeneratingTable;

//######################################################################
/**
 * @~japanese 車両の発生に関する設定を入力する
 * @~english  Input vehicle generation setting
 * @~ @ingroup IO
 */
class VehicleGeneratorBuilder
{
public:
    VehicleGeneratorBuilder(RoadMap* roadMap)
    {
        _roadMap   = roadMap;
        _generator = nullptr;
        _numCells  = 0;
        _excludedStarts.clear();
        _excludedGoals.clear();
    }
    ~VehicleGeneratorBuilder() {};

    /**
     * @~japanese 車両生成器を生成して戻す
     * @~english  Build and return vehicle generator
     */
    VehicleGenerator* buildVehicleGenerator();

private:
    /**
     * @~japanese ランダム選択から除外されるODNodeを読み込む
     * @~english  Read ODNodes excluded from random selection
     */
    void _readExcludedODNodes();

    /**
     * @~japanese 車線数にもとづいてODNodeをクラス分けする
     * @~english  Classify ODNodes based on number of lanes
     */
    void _classifyODNodes();

    /**
     * @~japanese ODNodeのグループ設定を読み込む
     * @~english  Read group setting of ODNodes
     */
    void _readODGroupFile();

    /**
     * @~japanese 車両発生定義テーブルのセットアップ
     * @~english  Set up vehicle generation definition table
     */
    void _setUpGeneratingTables();

    /**
     * @~japanese ランダム車両発生定義テーブルのセットアップ
     * @~english  Set up random vehicle generation definition table
     */
    void _setUpRandomGeneratingTable();

    /**
     * @~japanese 経路コストに関するパラメータセットの読み込み
     * @~english  Read parameter set for route cost
     */
    void _readVehicleRoutingParams();

    /**
     * @~japanese
     * 経路探索で選好するネットワークランク選択確率の読み込み
     *
     * @~english
     * Read network rank selection probabilities preferred in routing
     */
    void _readVehicleRoutingPrefRank();

    //==================================================================
    /**
     * @~japanese
     * @name 車両発生定義テーブルの読み込みに使用される関数
     *
     * @~english
     * @name Functions used to load vehicle generation definition tables
     */
    ///@{
private:
    /**
     * @~japanese
     * 車両発生定義テーブルの設定を読み込む
     *
     * @p typeにより種別を分ける
     *
     * - NORMAL: generateTable, DEFAULT: defaultGeneratetable\n
     * (開始時刻, 終了時刻, 出発地ID, 目的地ID, 
     *  交通量, 車種ID, 経由地数, 経由地IDリスト) の順で指定する
     *
     * - FIXED: fixedGenerateTable\n
     * (出発時刻 (=開始時刻=終了時刻), 出発地ID, 目的地ID,
     *  車種ID, 経由地数, 経由地IDリスト) の順で指定する
     *
     * - GROUP: groupedGenerateTable\n
     * (開始時刻, 終了時刻,
     *  出発地グループ化フラグ, 出発地ID,
     *  目的地グループ化フラグ, 目的地ID,
     *  交通量, 車種, 経由地数, 経由地IDリスト) の順で指定する
     *
     * @~english
     * Load vehicle generation definition table settings
     *
     * Differentiate by @p type
     *
     * - NORMAL: generateTable, DEFAULT: defaultGeneratetable\n
     * Specify in the order of
     * (time to start applying, time to end applying,
     *  origin node ID, destination node ID, traffic volume,
     *  vehicle type ID, number of gates, list of gate IDs).
     *
     * - FIXED: fixedGenerateTable\n
     * Specify in the order of
     * (departure time (=time to start applying = time to end applying),
     *  origin node ID, destination node ID, 
     *  vehicle type ID, number of gates, list of gate IDs).
     *
     * - GROUP: groupedGenerateTable\n
     * Specify in the order of
     * (time to start applying, time to end applying,
     *  origin grouping flag, origin node (or group) ID,
     *  destination grouping flag, destination node (or group) ID,
     *  traffic volume, vehicle type ID, number of gates,
     *  list of gate IDs).
     */
    void _readGeneratingTableFile(
        GeneratingTableType type, const std::string& fileName);

    /**
     * @~japanese 文字列から交通量を得る
     * @note 係数を乗じる必要がある
     *
     * @~english  get traffic volume from string
     * @note Need to be multiplied by the given coefficient
     */
    double _str2volume(std::string str) const;

    /**
     * @~japanese トークンから経由地を得る
     *
     * posOrigin，posDestinationはvectorにおける出発地と目的地の位置，
     * posMidGatesは途中経由地の位置を示す
     *
     * @~english  get gates from token
     *
     * posOrigin, posDestination are positions of origin and destination
     * in the vector, posMidGates are positions of intermediate gates
     */
    void _getGates(
        std::vector<std::string>* result_gates,
        std::vector<std::string>* tokens, unsigned int posOrigin,
        unsigned int posDestination, unsigned int posNumGates,
        unsigned int posMidGates) const;

    ///@}

    //==================================================================
private:
    /**
     * @~japanese 車両生成器 
     * @~english  Vehicle generator
     */
    VehicleGenerator* _generator;

    /**
     * @~japanese 地図オブジェクト
     * @~english  Road map object
     */
    RoadMap* _roadMap;

    /**
     * @~japanese ランダムな起点選択から除外されるODNode ID
     * @~english  ODNode IDs excluded from random origin selection
     */
    std::vector<std::string> _excludedStarts;

    /**
     * @~japanese ランダムな終点選択から除外されるODNode ID
     * @~english  ODNode IDs excluded from random destination selection
     */
    std::vector<std::string> _excludedGoals;

    /**
     * @~japanese 生成した GeneratingTableCell の個数
     *
     * @note セルのIDに利用する
     *
     * @~english  Number of generated GeneratingTableCells
     *
     * @note Used for cell ID number
     */
    unsigned int _numCells;
};

#endif //__VEHICLE_GENERATOR_BUILDER_HPP__
