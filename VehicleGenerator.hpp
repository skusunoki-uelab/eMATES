/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleGenerator.hpp
 */
#ifndef __VEHICLE_GENERATOR_HPP__
#define __VEHICLE_GENERATOR_HPP__
#include "GeneratingTable.hpp"
#include "RandomNumberGenerator.hpp"
#include "VehicleType.hpp"
#include "VehicleTypeManager.hpp"
#include <AmuConverter.hpp>
#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#ifdef _OPENMP
#include <omp.h>
#endif //_OPENMP

class Simulator;
class RoadMap;
class Intersection;
class ODNode;
class Section;
class Vehicle;
class ODNodeGroup;

//######################################################################
/**
 * @~japanese 車両発生定義テーブルの種類
 * @~english  Types of vehicle generation definition table
 */
enum class GeneratingTableType : unsigned int
{
    NORMAL,
    DEFAULT,
    FIXED,
    GROUP,
    RANDOM
};

//######################################################################
/**
 * @~japanese ODのグループ化の種類
 * @~english  OD grouping type
 */
enum class GroupingType : unsigned int
{
    NONE,
    START,
    GOAL,
    PAIR
};

//######################################################################
/**
 * @~japanese 車両発生イベントキューのキー
 * @~english  Key for vehicle generation event queue
 */
struct GeneratingQueueKey
{
    //==================================================================
public:
    /**
     * @~japanese ソートに用いる比較演算子
     * @~english  Comparison operator for sorting
     */
    friend bool operator<(
        const GeneratingQueueKey& lhs, const GeneratingQueueKey& rhs)
    {
        return std::tie(lhs._time, lhs._cellId)
            < std::tie(rhs._time, rhs._cellId);
    }

    //==================================================================
public:
    GeneratingQueueKey(ulint time, ulint cellId) : _time(time), _cellId(cellId)
    {
    }
    ~GeneratingQueueKey() {}

private:
    /**
     * @~japanese 車両発生時刻
     * @~english  Vehicle generation time
     */
    const ulint _time;

    /**
     * @~japanese GeneratingTableCellの識別番号
     * @~english  GeneratingTableCell ID number
     */
    const ulint _cellId;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    ulint time() const
    {
        return _time;
    }

    ulint cellId() const
    {
        return _cellId;
    }

    ///@}
};

//######################################################################
/**
 * @~japanese 車両の発生を制御する
 * @~english  Control vehicle generation
 * @~ @ingroup Running
 */
class VehicleGenerator
{
public:
    explicit VehicleGenerator(RoadMap* roadMap);
    ~VehicleGenerator();

    //==================================================================
    /**
     * @~japanese @name ODNodeに関する関数
     * @~english  @name Functions for ODNodes
     */
    ///@{
public:
    /**
     * @~japanese originノード @p node を追加する
     * @~english  Add origin node @p node
     */
    void addStartNode(ODNode* node)
    {
        _startNodes.push_back(node);
    }

    /**
     * @~japanese
     * レベル @p level にoriginノード @p node を追加する
     *
     * @~english
     * Add origin node @p node to level @p level
     */
    void addStartLevel(int level, ODNode* node)
    {
        _startLevel[level].push_back(node);
    }

    /**
     * @~japanese ランダム選択から除外するoriginノードを追加する
     * @~english  Add origin node to exclude from random selection
     */
    void addExcludedRandomStartNode(ODNode* node);

    /**
     * @~japanese destinationノード @p node を追加する
     * @~english  Add destination node @p node
     */
    void addGoalNode(ODNode* node)
    {
        _goalNodes.push_back(node);
    }

    /**
     * @~japanese
     * レベル @p level に destinationノード @p node を追加する
     *
     * @~english
     * Add destination node @p node to level @p level 
     */
    void addGoalLevel(int level, ODNode* node)
    {
        _goalLevel[level].push_back(node);
        _totalDefaultGoalTrafficVolume += _defaultTrafficVolume[level];
    }

    /**
     * @~japanese ランダム選択から除外するdestinationノードを追加する
     * @~english  Add destination node to exclude from random selection
     */
    void addExcludedRandomGoalNode(ODNode* node);

    /**
     * @~japanese originノード @p node のレベルを戻す
     *
     * @note
     * sectionへの流入点の数で決定する．「sectionの」流入点・流出点は
     * 「originノードの」流入点・流出点とは逆なので注意．
     * 
     * @~english  Return the level of origin node @p node
     *
     * @note
     * Determined by the number of inflow points to section.
     * Note that inflow and outflow points of a section are the
     * opposite of inflow and outflow points of an origin node.
     */
    int getStartLevel(ODNode* node) const;

    /**
     * @~japanese destinationノード @p node のレベルを戻す
     *
     * @note
     * sectionからの流出点の数で決定する．
     * 
     * @~english  Return the level of destination node @p node
     *
     * @note
     * Determined by the number of outflow points from section.
     */
    int getGoalLevel(ODNode* node) const;

    /**
     * @~japanese
     * 種別が @p type であるODNodeのグループ @p group を追加する
     *
     * @~english
     * Add ODNode group @p group of type @p type
     */
    void addODGroup(GroupingType type, ODNodeGroup* group);

private:
    /**
     * @~japanese
     * @p node がランダム出発地選択から除外されているか
     *
     * @~english
     * Whether @p node is excluded from random origin selection
     */
    bool _excludesStartNode(ODNode* node) const
    {
        if (std::find(_startNodes.begin(), _startNodes.end(), node)
            == _startNodes.end())
        {
            return true;
        }
        return false;
    }

    /**
     * @~japanese
     * @p node がランダム目的地選択から除外されているか
     *
     * @~english
     * Whether @p node is excluded from random destination selection
     */
    bool _excludesGoalNode(ODNode* node) const
    {
        if (std::find(_goalNodes.begin(), _goalNodes.end(), node)
            == _goalNodes.end())
        {
            return true;
        }
        return false;
    }

    ///@}

    //==================================================================
    /**
     * @~japanese
     * @name 車両発生定義テーブルに関する関数
     *
     * @~english
     * @name Functions for vehicle generation definition table
     */
    ///@{
public:
    /**
     * @~japanese 種別 @p type のテーブルにセル @p cell を追加する
     * @~english  Add cell @p cell to table of type @p type
     */
    void addGTCellToTable(GeneratingTableType type, GeneratingTableCell* cell);

    /**
     * @~japanese 種別 @p type のテーブルをソートする
     * @~english  Sort table of type @p type
     */
    void sortGeneratingTable(GeneratingTableType type);

    ///@}

    //==================================================================
    /**
     * @~japanese @name エージェントの発生に関する関数
     * @~english  @name Functions for generating agents
     */
    ///@{
public:
    /**
     * @~japanese 車両を発生させる
     * @~english  Generate vehicle
     */
    void generateVehicle();

    /**
     * @~japanese 手動で車両を発生させる
     * @~english  Generate vehicle manually
     */
    void generateVehicleManually(
        const std::string& startId, const std::string& goalId,
        std::vector<std::string>* gateIds, VehicleType vehicleType,
        std::vector<double> params, unsigned int prefRank);

protected:
    /**
     * @~japanese
     * 現在時刻で適用可能なGeneratingTableCellを有効化する
     *
     * @note
     * generateTable，defaultGenerateTable，fixedGenerateTableのセルで
     * 現在時刻に適用可能なものを抽出し，最初の車両発生時刻を求める．
     * 各セルが有効化されるのは1回のみ．それ以降は，該当セルの指定により
     * 車両が発生するたびに次の車両発生時刻を求める．
     *
     * @~english
     * Activate GeneratingTableCell applicable at current time
     *
     * @note
     * Extract the cells in generateTable, defaultGenerateTable, and
     * fixedGenerateTable that are applicable to the current time,
     * and calculate the first vehicle generation time. Each cell is
     * activated only once. After that, the next vehicle generation
     * time is calculated every time a vehicle is generated by being
     * specified by the corresponding cell.
     */
    void _activatePresentGTCells();

    /**
     * @~japanese
     * 次の車両発生時刻を求め，車両発生イベントキューに追加する
     *
     * @note
     * 単位時間あたりの車両発生台数がポアソン分布に従うことを仮定する．
     * このとき，車両発生の時間間隔は指数分布にしたがう．
     *
     * @param startTime 時間間隔の基準となる時刻
     * @param cell 処理する対象のGeneratingTableCell
     * @return 有効な時刻を設定したかどうか
     *
     * @~english
     * Calculate the next vehicle generation time and add it to the
     * vehicle generation event queue
     *
     * @note
     * Assume that the number of vehicle generated per unit time follows
     * a Poisson distribution. At this time, the time interval between
     * vehicle generations follows an exponential distribution.
     *
     * @param startTime Base time for time interval
     * @param cell GeneratingTableCell to process
     * @return Whether a valid time has been set
     */
    bool _determineNextTimeOfGeneration(
        ulint startTime, const GeneratingTableCell* cell);

    /**
     * @~japanese
     * fixedGenerateTableのセルで指定された車両発生時刻を優先車両発生
     * キューに追加する
     *
     * @~english
     * Add the vehicle generation time specified the cell in
     * fixedGenerateTable to the priority vehicle generation event
     * queue
     */
    bool _determineFixedTimeOfGeneration(const GeneratingTableCell* cell);

    /**
     * @~japanese  車両発生イベントキューから車両を発生させる
     *
     * @attention
     * 車両生成順が固定できないと再現性が損なわれるため，かならず
     * シングルスレッド上で実行する
     *
     * @~english   Generate vehicle from vehicle generation event queue
     *
     * @attention
     * If the vehicle generation order cannot be fixed, reproducibility
     * will be lost, so always execute  on a single thread.
     */
    void _generateVehiclesFromQueue();

    /**
     * @~japanese
     * 優先車両発生イベントキューから車両を発生させる
     *
     * @attention
     * 車両生成順が固定できないと再現性が損なわれるため，かならず
     * シングルスレッド上で実行する
     *
     * @~english
     * Generate vehicle from priority vehicle generation event queue
     *
     * @attention
     * If the vehicle generation order cannot be fixed, reproducibility
     * will be lost, so always execute  on a single thread.
     */
    void _generateVehiclesFromPriorQueue();

    /**
     * @~japanese
     * 車両発生イベントキューから現時点で処理すべき要素を抽出する
     *
     * @note
     * generatesVehicleAllAtOnceがtrueの場合，現在時刻を無視して
     * キュー内のセルすべてを抽出する．fixedTableのデバッグ時に使用する
     * ことを想定している．
     *
     * @~english
     * Extract elements to be processed currently from the vehicle
     * generation event queue
     *
     * @note
     * If generatesVehicleAllAtOnce is true, extract all cells in the
     * queue, ignoring the current time. Assumed that is used when 
     * debugging fixedTable. 
     */
    void _extractValidCellFromEventQueue(
        std::multimap<GeneratingQueueKey, const GeneratingTableCell*>& queue,
        std::vector<const GeneratingTableCell*>& result_validCells,
        bool                                     generatesVehicleAllAtOnce);

    /**
     * @~japanese
     * GeneratingTableCell @p cell で指定された条件で車両を生成する
     *
     * @~english
     * Generate vehicle under the condition specified by Generating-
     * TableCell @p cell
     */
    Vehicle* _generateVehicleFromGTCell(const GeneratingTableCell* cell);

    /**
     * @~japanese 車両を生成する
     *
     * @param start 出発地
     * @param goal  目的地
     * @param rear  車両が配置される単路部の上流交差点
     * @param front 車両が配置される単路部の下流交差点
     * @param gateIds 経由地のIDの集合
     * @param vehicleType 車種
     *
     * @note
     * 今後，経路の途中での生成も可能となるように，@p rear ，@p front を
     * 与えられるようにしてある
     *
     * @~english  Create a vehicle
     *
     * @param start Origin
     * @param goal  Destination
     * @param rear  Upstream intersection the vehicle is placed
     * @param front Downstream intersection the vehicle is placed
     * @param gateIds Set of ID numbers of way-points
     * @param vehicleType Vehicle type
     *
     * @note
     * In order that a vehicle can be generated in the middle of the route
     * in the future, @p rear and @p front can be given.
     */
    Vehicle* _createVehicle(
        ODNode* start, ODNode* goal, Intersection* rear, Intersection* front,
        const std::vector<std::string>& gateIds, VehicleType vehicleType);

    /**
     * @~japanese
     * @p vehicle に @p type の車体属性を反映する
     *
     * @~english
     * Reflect the vehicle body attribute of @p type to @p vehicle
     */
    void _setVehicleBodyAttributes(Vehicle* vehicle, VehicleType type);

    /**
     * @~japanese 経路探索パラメータをランダムに設定する
     * @~english  Set routing parameters randomly
     */
    void _setRoutingParamsRandomly(Vehicle* vehicle);

    /**
     * @~japanese
     * 経路探索で選好するネットワークランクをランダムに設定する
     *
     * @~english
     * Set preferred network rank for routing randomly
     */
    void _setRoutingPrefRankRandomly(Vehicle* vehicle);

    /**
     * @~japanese 出発地と目的地をともに得る
     *
     * @note
     * OD pair 専用の処理
     *
     * @~english  Get both origin and destination
     *
     * @note
     *
     * Processing dedicated to OD pair
     */
    void _getStartAndGoalNodes(
        const GeneratingTableCell* cell, ODNode** result_start,
        ODNode** result_goal);

    /**
     * @~japanese 出発地を得る
     *
     * GeneratingTableCell が持つ識別番号から ODNode のポインタを得る．
     * ODNodeグループに関する処理も同時に行う．
     * 
     * @~english  Get origin
     *
     * Get the pointer of ODNode from the ID number of GeneratingTable-
     * Cell. At the same time, processing related to the ODNode group is
     * also performed.
     */
    ODNode* _getStartNode(const GeneratingTableCell* cell);

    /**
     * @~japanese 目的地を戻す
     *
     * GeneratingTableCell が持つ識別番号から ODNode のポインタを得る．
     * ODNodeグループに関する処理も同時に行う．
     * 
     * @~english  Get destination
     *
     * Get the pointer of ODNode from the ID number of GeneratingTable-
     * Cell. At the same time, processing related to the ODNode group is
     * also performed.
     */
    ODNode* _getGoalNode(const GeneratingTableCell* cell, ODNode* start);

    /**
     * @~japanese 目的地をランダムに決定する
     * @~english  Determine destination randomly
     */
    ODNode* _decideGoalRandomly(ODNode* start);

    /**
     * @~japanese
     * 経由地の識別番号を @p result_gateIdsに格納する
     *
     * @~english
     * Store the ID numbers of the way-points in @p result_gateIds
     */
    void _getGateIds(
        const GeneratingTableCell* cell, ODNode* start, ODNode* goal,
        std::vector<std::string>& result_gateIds);

    /**
     * @~japanese
     * 経由地のIDから経由地のポインタを得て @p result_gates に格納する
     *
     * @~english
     * Get the pointers of way-points from the way-point ID numbers
     * and store them in @p result_gates
     */
    void _convertGateIds2Gates(
        const std::vector<std::string>&   gateIds,
        std::vector<const Intersection*>& result_gates, ODNode* start,
        ODNode* goal);

    ///@}

    //==================================================================
    /**
     * @~japanese @name 経路探索に関する関数
     * @~english  @name Functions for routing
     */
    ///@{
public:
    /**
     * @~japanese 経路探索パラメータを追加する
     * @~english  Add routing parameters
     */
    void addRoutingParam(std::vector<double> param)
    {
        assert(param.size() == VEHICLE_ROUTING_PARAMETER_SIZE);
        _routingParams.emplace_back(param);
    }

    /**
     * @~japanese
     * 経路探索で選好するネットワークランクの選択確率を追加する
     *
     * @~english
     * Add selection probability of preferred network rank in routing
     */
    void addRoutingPrefRank(int rank, double prob)
    {
        if (_routingPrefRank.find(rank) != _routingPrefRank.end())
        {
            std::cerr << "WARNING: selection probability of rank[" << rank
                      << "] is already given" << std::endl;
            return;
        }
        _routingPrefRank.insert(std::make_pair(rank, prob));
    }

    ///@}

    //==================================================================
public:
    /**
     * @~japanese
     * ランダム選択から除外されている出発地を @p out に出力する
     *
     * @~english
     * Output origin nodes excluded from random selection to @p out
     */
    void printExcludedRandomStarts(std::ostream& out) const;

    /**
     * @~japanese
     * ランダム目的地選択から除外されているODNodeを @p out に出力する
     *
     * @~english
     * Output destination nodes excluded from random selection to @p out
     */
    void printExcludedRandomGoals(std::ostream& out) const;

    /**
     * @~japanese ODNodeのグループを @p out に出力する
     * @~english  Output ODNode group to @p out
     */
    void printODGroup(std::ostream& out) const;

    /**
     * @~japanese
     * 経路探索の効用関数に使用するパラメータセットを @p out に出力する
     *
     * @~english
     * Output parameter sets for routing utility function to @p out
     */
    void printVehicleRoutingParams(std::ostream& out) const;

    /**
     * @~japanese
     * 経路探索において選好するネットワークランクの選択確率を @p out に
     * 出力する
     *
     * @~english
     * Output selection probability of preferred network rank in routing
     * to @p out
     */
    void printVehicleRoutingPrefRank(std::ostream& out) const;

    //==================================================================
private:
    /**
     * @~japanese シミュレータオブジェクト
     * @~english  Simulator object
     */
    Simulator* _sim;

    /**
     * @~japanese 地図オブジェクト
     * @~english  Road map object
     */
    RoadMap* _roadMap;

    /**
     * @~japanese 乱数生成器
     * @~english  Random number generator
     */
    RandomNumberGenerator _rng;

#ifdef _OPENMP
    /**
     * @~japanese ロック変数
     * @~english  Lock variable
     */
    omp_lock_t _lock;
#endif //_OPENMP

    //==================================================================
    /**
     * @~japanese @name ODNodeに関する変数
     * @~english  @name Variables for ODNodes
     */
    ///@{
private:
    /**
     * @~japanese originノードの集合
     * @~english  Set of origin nodes
     */
    std::vector<ODNode*> _startNodes;

    /**
     * @~japanese ランダム選択から除外されたoriginノード
     * @~english  Origin nodes excluded from random selection
     */
    std::unordered_map<std::string, ODNode*> _excludedStartNodes;

    /**
     * @~japanese クラス分けされたoriginノード
     * @~english  Classified origin nodes
     */
    std::vector<ODNode*> _startLevel[3];

    /**
     * @~japanese destinationノードの集合
     * @~english  Set of destination nodes
     */
    std::vector<ODNode*> _goalNodes;

    /**
     * @~japanese ランダム選択から除外されたdestinationノード
     * @~english  Destination nodes excluded from random selection
     */
    std::unordered_map<std::string, ODNode*> _excludedGoalNodes;

    /**
     * @~japanese クラス分けされたdestinationノード
     * @~english  Classified destination nodes
     */
    std::vector<ODNode*> _goalLevel[3];

    /**
     * @~japanese グループ化されたoriginノード
     * @~english  Grouped origin nodes
     */
    std::unordered_map<std::string, ODNodeGroup*> _odStartGroup;

    /**
     * @~japanese グループ化されたdestinationノード
     * @~english  Grouped destination nodes
     */
    std::unordered_map<std::string, ODNodeGroup*> _odGoalGroup;

    /**
     * @~japanese グループ化されたorigin-destinationペア
     * @~english  Grouped origin-destination pair
     */
    std::unordered_map<std::string, ODNodeGroup*> _odPairGroup;

    ///@}

    //==================================================================
    /**
     * @~japanese @name 車両発生定義テーブル
     * @~english  @name Vehicle generation definition table
     */
    ///@{
private:
    /**
     * @~japanese 出発地，目的地の双方を規定するテーブル
     * @~english  Table specifying both origin and destination
     */
    GeneratingTable _table;

    /**
     * @~japanese 出発地のみを規定するテーブル
     * @~english  Table specifying only origin
     */
    GeneratingTable _defaultTable;

    /**
     * @~japanese
     * 発生時刻，出発地，目的地を規定するテーブル
     *
     * @~english
     * Table specifying generation time, origin and destination
     */
    GeneratingTable _fixedTable;

    /**
     * @~japanese グループ化された出発地と目的地を規定するテーブル
     * @~english  Table specifying grouped origin and destination
     */
    GeneratingTable _groupedTable;

    /**
     * @~japanese ランダム生成用テーブル
     * @~english  Table for random generation
     */
    GeneratingTable _randomTable;

    ///@}

    //==================================================================
    /**
     * @~japanese @name 車両発生プロセスに関する変数
     * @~english  @name Variables for vehicle generation process
     */
    ///@{
private:
    /**
     * @~japanese 車両発生を待つ出発地ノード
     * @~english  Origin nodes waiting for vehicle generation
     */
    std::vector<ODNode*> _waitingODNodes;

    /**
     * @~japanese 標準的な発生交通量 (単路ごと, 台/h) 
     * @~english  Standard generation volume (per section, /h)
     */
    int _defaultTrafficVolume[3];

    /**
     * @~japanese 標準的な発生交通量の総和
     * @note      ランダム目的地選択に用いる
     *
     * @~english  Sum of standard generation volume
     * @note      used for random destination selection
     */
    ulint _totalDefaultGoalTrafficVolume;

    /**
     * @~japanese 車両発生イベントキュー
     *
     * mapのキーは<車両発生時刻とGeneratingTableCellの識別番号>のペア，
     * 値は該当するGeneratingTableCell
     *
     * @~english  Vehicle generation event queue
     *
     * The map key is a pair of <vehicle generation time and
     * GeneratingTableCell ID number>, the value is the corresponding
     * GeneratingTableCell.
     */
    std::multimap<GeneratingQueueKey, const GeneratingTableCell*>
        _generatingQueue;

    /**
     * @~japanese 優先車両発生イベントキュー
     *
     * 発生時刻が厳密に指定されたバスなどはpush_backでなく
     * push_frontする必要があるため別に管理する
     *
     * @~english  Prior vehicle generation event queue
     *
     * Buses with strictly specified generation time must be push_front
     * instead of push_back, so should be managed separately.
     */
    std::multimap<GeneratingQueueKey, const GeneratingTableCell*>
        _generatingPriorQueue;

    ///@}

    //==================================================================
    /**
     * @~japanese @name 経路探索に関する変数
     * @~english  @name Routing variables
     */
    ///@{
private:
    /**
     * @~japanese 経路探索の効用関数に関するパラメータセット
     * @~english  Parameter set for routing utility function
     */
    std::vector<std::vector<double>> _routingParams;

    /**
     * @~japanese
     * 経路探索で選好するネットワークランクの選択確率
     *
     * @~english
     * Selection probability of preferred network rank in routing
     */
    std::map<unsigned int, double, std::greater<int>> _routingPrefRank;

    ///@}

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    void setSimulator(Simulator* sim)
    {
        _sim = sim;
    }

    RoadMap* roadMap() const
    {
        return _roadMap;
    }

    const std::vector<ODNode*>& startNodes() const
    {
        return _startNodes;
    }

    const std::vector<ODNode*>& goalNodes() const
    {
        return _goalNodes;
    }

    GeneratingTable& table()
    {
        return _table;
    }

    GeneratingTable& defaultTable()
    {
        return _defaultTable;
    }

    GeneratingTable& fixedTable()
    {
        return _fixedTable;
    }

    GeneratingTable& groupedTable()
    {
        return _groupedTable;
    }

    GeneratingTable& randomTable()
    {
        return _randomTable;
    }

    void setDefaultTrafficVolume(unsigned int level, int volume)
    {
        assert(level < 3);
        _defaultTrafficVolume[level] = volume;
    }

    int defaultTrafficVolume(unsigned int level) const
    {
        assert(level < 3);
        return _defaultTrafficVolume[level];
    }

    const std::vector<std::vector<double>>& routingParams()
    {
        return _routingParams;
    }

    const std::map<unsigned int, double, std::greater<int>>& routingPrefRank()
    {
        return _routingPrefRank;
    }

    ///@}
};

#endif //__VEHICLE_GENERATOR_HPP__
