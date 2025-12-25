/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleGeneratorBuilder.cpp
 */
#include "VehicleGeneratorBuilder.hpp"
#include "../AppMates.hpp"
#include "../Config.hpp"
#include "../CustomMessage.hpp"
#include "../GVManager.hpp"
#include "../ODNodeGroup.hpp"
#include <AmuConverter.hpp>
#include <AmuStringOperator.hpp>
#include <cassert>
#include <climits>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace amu::converter;
using namespace amu::string_operator;

//======================================================================
VehicleGenerator* VehicleGeneratorBuilder::buildVehicleGenerator()
{
    _generator = new VehicleGenerator(_roadMap);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 標準的な発生交通量(単路ごと, 台/h) を設定する
    // Set standard generation volume (per section, /h)
    _generator->setDefaultTrafficVolume(
        0, static_cast<int>(AppMates::getGVManager().getNumeric(
               "DEFAULT_TRAFFIC_VOLUME_WIDE")));
    _generator->setDefaultTrafficVolume(
        1, static_cast<int>(AppMates::getGVManager().getNumeric(
               "DEFAULT_TRAFFIC_VOLUME_NORMAL")));
    _generator->setDefaultTrafficVolume(
        2, static_cast<int>(AppMates::getGVManager().getNumeric(
               "DEFAULT_TRAFFIC_VOLUME_NARROW")));

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * ODノードの設定
     *   _readODGroupFile()の呼び出しは標準的な発生交通量を設定した後で
     *   なければならない．
     *
     * ODNode settings
     *   Calling _readODGroupFile() must be after setting the standard
     *   generation volume.
     */
    _readExcludedODNodes();
    _classifyODNodes();
    _readODGroupFile();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 車両発生テーブルの読み込み
     *   ODノードグループの定義後でなければならない
     *
     * Load vehicle generation tables
     *   This must be after definition of ODNode groups.
     */
    if (AppMates::getGVManager().getFlag("FLAG_INPUT_VEHICLE"))
    {
        _setUpGeneratingTables();
    }

    // 車両発生が定義されていない交差点のテーブルの作成
    // Define generation table for intersections without specification
    if (AppMates::getGVManager().getFlag("FLAG_GEN_RAND_VEHICLE"))
    {
        _setUpRandomGeneratingTable();
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 経路探索パラメータの設定
    // Set routing parameters
    _readVehicleRoutingParams();
    _readVehicleRoutingPrefRank();

    return _generator;
}


//======================================================================
void VehicleGeneratorBuilder::_readExcludedODNodes()
{
    assert(_generator);
    GVManager&    gv    = AppMates::getGVManager();
    string        fname = gv.getString("OD_NODE_EXCLUSION_FILE");
    ostringstream ss;
    ss << "read ODNode exclusion file (" << gv.stripDataDir(fname)
       << ") ... ";

    // ファイルを読み込む
    // Load file
    ifstream fin(fname.c_str(), ios::in);
    if (!fin)
    {
        ss << "not found";
        amu::msg::status(cout, ss.str());
        return;
    }
    amu::msg::status(cout, ss.str());

    while (fin.good())
    {
        string         line;
        vector<string> tokens;
        if (!getTokens(&fin, &line, &tokens, ','))
        {
            break;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 不正な行の処理
        // Invalid line handling
        if (tokens.size() != 2)
        {
            cerr << "WARNING: invalid ODNode exclusion format - "
                 << line << endl;
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling

        // 第1カラムは"O", "D", "OD"のいずれか，第2カラムはノードID
        // 1st column is either "O", "D" or "OD", 2nd column is node ID
        if (tokens[0] == "O" || tokens[0] == "OD")
        {
            _excludedStarts.push_back(
                formatId(tokens[1], NUM_FIGURE_FOR_INTERSECTION));
        }
        if (tokens[0] == "D" || tokens[0] == "OD")
        {
            _excludedGoals.push_back(
                formatId(tokens[1], NUM_FIGURE_FOR_INTERSECTION));
        }
    }
    fin.close();

    ss << "done";
    amu::msg::status(cout, ss.str());
    return;
}

//======================================================================
void VehicleGeneratorBuilder::_classifyODNodes()
{
    assert(_generator);

    /*
     * 除外ファイルで指定されていないODノードをレベル分けして格納する
     *
     * Store ODNodes not specified in the exclusion file by dividing
     * them into levels
    */
    for (auto itr : _roadMap->odNodes())
    {
        int startLevel = _generator->getStartLevel(itr);
        if (startLevel >= 0 && startLevel < 3)
        {
            if (find(
                    _excludedStarts.begin(), _excludedStarts.end(),
                    itr->id())
                == _excludedStarts.end())
            {
                _generator->addStartLevel(startLevel, itr);
                _generator->addStartNode(itr);
            }
            else
            {
                _generator->addExcludedRandomStartNode(itr);
            }
        }

        int goalLevel = _generator->getGoalLevel(itr);
        if (goalLevel >= 0 && goalLevel < 3)
        {
            if (find(
                    _excludedGoals.begin(), _excludedGoals.end(),
                    itr->id())
                == _excludedGoals.end())
            {
                _generator->addGoalLevel(goalLevel, itr);
                _generator->addGoalNode(itr);
            }
            else
            {
                _generator->addExcludedRandomGoalNode(itr);
            }
        }
    }
}

//======================================================================
void VehicleGeneratorBuilder::_readODGroupFile()
{
    assert(_generator);
    GVManager&    gv    = AppMates::getGVManager();
    string        fname = gv.getString("OD_GROUP_FILE");
    ostringstream ss;
    ss << "read ODNode group file (" << gv.stripDataDir(fname)
       << ") ... ";

    // ファイルを読み込む
    // Load file
    ifstream fin(fname.c_str(), ios::in);
    if (!fin)
    {
        ss << "not found";
        amu::msg::status(cout, ss.str());
        return;
    }
    amu::msg::status(cout, ss.str());

    // 行を越え状態を保持する必要がある
    // Need to keep state across lines
    ODNodeGroup* group;
    GroupingType mode = GroupingType::NONE;
    int          numMembers;

    while (fin.good())
    {
        string         line;
        vector<string> tokens;
        if (!getTokens(&fin, &line, &tokens, ','))
        {
            break;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        /* グループタイプの指定
         *   第1カラムはO, D, ODのいずれか，第2カラムはグループID，
         *   第3カラムはグループメンバの個数
         *
         * Group type specification
         *   1st column is "O", "D" or "OD", 2nd column is group ID,
         *   and 3rd column is number of group members
         */
        if (tokens.size() == 3
            && (tokens[0] == "O" || tokens[0] == "D"
                || tokens[0] == "OD"))
        {
            string groupId
                = formatId(tokens[1], NUM_FIGURE_FOR_OD_GROUP);
            group = new ODNodeGroup(groupId);
            if (tokens[0] == "O")
            {
                mode = GroupingType::START;
            }
            else if (tokens[0] == "D")
            {
                mode = GroupingType::GOAL;
            }
            else
            {
                mode = GroupingType::PAIR;
            }
            _generator->addODGroup(mode, group);
            numMembers = stoi(tokens[2]);
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // STARTまたはGOALグループに含まれるODNodeの指定
        // Specifying ODNodes included in START or GOAL Groups
        else if (
            (mode == GroupingType::START || mode == GroupingType::GOAL)
            && tokens.size() == 1 && numMembers > 0)
        {
            numMembers--;
            string id
                = formatId(tokens[0], NUM_FIGURE_FOR_INTERSECTION);
            ODNode* node
                = dynamic_cast<ODNode*>(_roadMap->intersection(id));
            if (!node)
            {
                cerr << "ERROR: non ODNode[" << id
                     << "] cannot be added to OD group." << endl;
                continue;
            }

            int level = -1;
            if (mode == GroupingType::START)
            {
                level = _generator->getStartLevel(node);
            }
            else
            {
                level = _generator->getGoalLevel(node);
            }
            if (level == -1)
            {
                cerr << "ERROR: origin node[" << id << "] not found"
                     << endl;
                continue;
            }
            group->addODSolo(
                node, _generator->defaultTrafficVolume(level));
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // ODNodeのペアを指定する
        // Specifying pair of ODNodes
        else if (
            mode == GroupingType::PAIR && tokens.size() == 2
            && numMembers > 0)
        {
            numMembers--;
            string ids
                = formatId(tokens[0], NUM_FIGURE_FOR_INTERSECTION);
            string idg
                = formatId(tokens[1], NUM_FIGURE_FOR_INTERSECTION);
            ODNode* nodes
                = dynamic_cast<ODNode*>(_roadMap->intersection(ids));
            ODNode* nodeg
                = dynamic_cast<ODNode*>(_roadMap->intersection(idg));
            if (!nodes || !nodeg)
            {
                if (!nodes)
                {
                    cerr << "ERROR: non ODNode[" << ids
                         << "] cannot be added to OD group." << endl;
                }
                if (!nodeg)
                {
                    cerr << "ERROR: non ODNode[" << idg
                         << "] cannot be added to OD group." << endl;
                }
                continue;
            }

            int levels = _generator->getStartLevel(nodes);
            int levelg = _generator->getGoalLevel(nodeg);
            if (levels == -1 || levelg == -1)
            {
                if (levels == -1)
                {
                    cerr << "ERROR: origin node[" << ids
                         << "] not found" << endl;
                }
                if (levelg == -1)
                {
                    cerr << "ERROR: destination node[" << idg
                         << "] not found" << endl;
                }
                continue;
            }

            double weight = sqrt(
                _generator->defaultTrafficVolume(levels)
                * _generator->defaultTrafficVolume(levelg));
            group->addODPair(nodes, nodeg, weight);
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 不正な行の処理
        // Invalid line handling
        else
        {
            cerr << "WARNING: invalid ODNode grouping format - " << line
                 << endl;
            group      = nullptr;
            mode       = GroupingType::NONE;
            numMembers = 0;
            continue;
        }
    }
    fin.close();

    ss << "done";
    amu::msg::status(cout, ss.str());
    return;
}

//======================================================================
void VehicleGeneratorBuilder::_setUpGeneratingTables()
{
    assert(_generator);

    {
        // generateTable
        string fname
            = AppMates::getGVManager().getString("GENERATE_TABLE");
        if (!fname.empty())
        {
            _readGeneratingTableFile(
                GeneratingTableType::NORMAL, fname);
        }
    }
    {
        // defaultGenerateTable
        string fname = AppMates::getGVManager().getString(
            "DEFAULT_GENERATE_TABLE");
        if (!fname.empty())
        {
            _readGeneratingTableFile(
                GeneratingTableType::DEFAULT, fname);
        }
    }
    {
        // fixedGenerateTable
        string fname = AppMates::getGVManager().getString(
            "FIXED_GENERATE_TABLE");
        if (!fname.empty())
        {
            _readGeneratingTableFile(GeneratingTableType::FIXED, fname);
        }
    }
    {
        // groupedGenerateTable
        string fname = AppMates::getGVManager().getString(
            "GROUPED_GENERATE_TABLE");
        if (!fname.empty())
        {
            _readGeneratingTableFile(GeneratingTableType::GROUP, fname);
        }
    }
}

//======================================================================
void VehicleGeneratorBuilder::_setUpRandomGeneratingTable()
{
    for (auto itr : _generator->startNodes())
    {
        vector<string> dummyGates;
        dummyGates.clear();
        dummyGates.emplace_back(itr->id());
        dummyGates.emplace_back("******");

        int startLevel = _generator->getStartLevel(itr);
        if (startLevel < 0)
        {
            // ノードからの流出車線がない -> originにならない
            // No outflow lane from node -> not be an origin
            continue;
        }

        /*
         * 発生交通量がgenerateTable, defaultGenerateTableによって
         * 指定されている交差点はランダム発生の対象から除外する．
         * fixedTable, groupedTableによる指定は除外の根拠としない．
         *
         * Intersections whose generation volume is given by
         * generateTable or defaultGenerateTable are excluded from
         * random generation. On the other hand, specification by
         * fixedTable and groupedTable are not grounds for exclusion.
         */
        vector<const GeneratingTableCell*> validGTCells;
        _generator->table().getValidGTCells(itr->id(), validGTCells);
        _generator->defaultTable().getValidGTCells(
            itr->id(), validGTCells);

        double volume
            = _generator->defaultTrafficVolume(startLevel)
              * AppMates::getGVManager().getNumeric("RANDOM_OD_FACTOR");

        if (validGTCells.empty())
        {
            /*
             * 有効なGeneratingTableCellが見つからなかった場合，常に
             * デフォルト交通量を適用．うち90%を普通車(PASSENGER)，
             * 10%を大型車(TRUCK)とする．
             * 
             * In the case no valid GeneratingTableCells were found,
             * always apply default generation volume. Of these,
             * 90% are PASSENGER, 10% are TRUCK.
             */
            GeneratingTableCell* cell1
                = new GeneratingTableCell(_numCells);
            _numCells++;
            if (cell1->setValues(
                    0, 86400000, volume * 0.9,
                    VehicleType(VehicleCategory::PASSENGER, 0),
                    dummyGates))
            {
                _generator->addGTCellToTable(
                    GeneratingTableType::RANDOM, cell1);
            }
            else
            {
                delete cell1;
            }
            GeneratingTableCell* cell2
                = new GeneratingTableCell(_numCells);
            _numCells++;
            if (cell2->setValues(
                    0, 86400000, volume * 0.1,
                    VehicleType(VehicleCategory::TRUCK, 0), dummyGates))
            {
                _generator->addGTCellToTable(
                    GeneratingTableType::RANDOM, cell2);
            }
            else
            {
                delete cell2;
            }
        }
    }

    // テーブルをソートする
    // sort table
    _generator->sortGeneratingTable(GeneratingTableType::RANDOM);
}

//======================================================================
void VehicleGeneratorBuilder::_readVehicleRoutingParams()
{
    /*
     * 経路探索の効用関数の計算に用いられる重みを定義する
     *   vector<double>で定義し，順に距離，時間，直進回数，左折回数，
     *   右折回数にかかる重みを表す
     *
     * Define weights used to calculate utility function of routing
     *   Defined as vector<double> and represent weights for distance,
     *   time, number of straight driving, number of left turns, and
     *   number of right turns, orderly. 
     */
    GVManager&    gv    = AppMates::getGVManager();
    string        fname = gv.getString("VEHICLE_ROUTE_PARAM_FILE");
    ostringstream ss;
    ss << "read vehicle routing parameter file ("
       << gv.stripDataDir(fname) << ") ... ";

    ifstream fin(fname.c_str(), ios::in);
    if (!fin)
    {
        ss << "not found";
        amu::msg::status(cout, ss.str());
    }
    else
    {
        amu::msg::status(cout, ss.str());
    }

    while (fin.good())
    {
        string         line;
        vector<string> tokens;
        if (!getTokens(&fin, &line, &tokens, ','))
        {
            break;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 不正な行の処理
        // Invalid line handling
        if (tokens.size() != VEHICLE_ROUTING_PARAMETER_SIZE)
        {
            cerr << "WARNING: invalid routing parameter file format - "
                 << line << endl;
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling
        vector<double> param;
        param.resize(VEHICLE_ROUTING_PARAMETER_SIZE);
        for (unsigned int i = 0; i < tokens.size(); i++)
        {
            param[i] = stod(tokens[i]);
        }
        _generator->addRoutingParam(param);
    }
    if (fin)
    {
        fin.close();
        ss << "done";
        amu::msg::status(cout, ss.str());
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 有効なパラメータが1つもない場合はデフォルト値を用いる
    // Use default values ​​if no valid parameters are available
    if (_generator->routingParams().size() == 0)
    {
        {
            vector<double> param(VEHICLE_ROUTING_PARAMETER_SIZE, 0);
            param[toUnderlying(RoutingParamIndex::DISTANCE)] = 1.0;
            _generator->addRoutingParam(param);
        }
        {
            vector<double> param(VEHICLE_ROUTING_PARAMETER_SIZE, 0);
            param[toUnderlying(RoutingParamIndex::TIME)] = 1.0;
            _generator->addRoutingParam(param);
        }
    }

    return;
}

//======================================================================
void VehicleGeneratorBuilder::_readVehicleRoutingPrefRank()
{
    GVManager&    gv    = AppMates::getGVManager();
    string        fname = gv.getString("VEHICLE_ROUTE_PREFRANK_FILE");
    ostringstream ss;
    ss << "read vehicle routing preferred network rank file ("
       << gv.stripDataDir(fname) << ") ... ";

    // ファイルを読み込む
    // Load file
    ifstream fin(fname.c_str(), ios::in);
    if (!fin)
    {
        ss << "not found";
        amu::msg::status(cout, ss.str());

        unsigned int rank
            = AppMates::getRouterManager().highestNetworkRank();
        double remain = 1.0;
        double prob;
        double except = 0.05;
        for (; rank >= 2; rank--)
        {
            // rank>=2
            except = remain * except;
            prob   = remain - except;
            _generator->addRoutingPrefRank(rank, prob);
            remain -= prob;
        }
        // rank==1
        _generator->addRoutingPrefRank(rank, remain);
        return;
    }
    amu::msg::status(cout, ss.str());

    while (fin.good())
    {
        string         line;
        vector<string> tokens;
        if (!getTokens(&fin, &line, &tokens, ','))
        {
            break;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 不正な行の処理
        // Invalid line handling
        if (tokens.size() != 2)
        {
            cerr << "WARNING: invalid vehicle routing preferred network"
                 << " rank file format - " << line << endl;
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling
        int    rank = stoi(tokens[0]);
        double prob = stof(tokens[1]);
        _generator->addRoutingPrefRank(rank, prob);
    }
    fin.close();

    ss << "done";
    amu::msg::status(cout, ss.str());
    return;
}

//======================================================================
void VehicleGeneratorBuilder::_readGeneratingTableFile(
    GeneratingTableType type, const string& fname)
{
    GVManager&    gv = AppMates::getGVManager();
    ostringstream ss;
    ss << "read GeneratingTable type [type=" << toUnderlying(type)
       << "] (" << gv.stripDataDir(fname) << ") ... ";

    // ファイルを読み込む
    // Load file
    ifstream fin(fname.c_str(), ios::in);
    if (!fin.good())
    {
        ss << "not found";
        amu::msg::status(cout, ss.str());
        return;
    }
    amu::msg::status(cout, ss.str());

    while (fin.good())
    {
        string         line;
        vector<string> tokens;
        if (!getTokens(&fin, &line, &tokens, ','))
        {
            break;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 不正な行の処理
        // Invalid line handling
        if ((type == GeneratingTableType::NORMAL && tokens.size() < 6)
            || (type == GeneratingTableType::DEFAULT
                && tokens.size() < 6)
            || (type == GeneratingTableType::FIXED && tokens.size() < 4)
            || (type == GeneratingTableType::GROUP
                && tokens.size() < 8))
        {
            cerr << "WARNING: invalid vehicle GeneratingTable[type="
                 << toUnderlying(type) << "] format" << endl;
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling
        GeneratingTableCell* cell = new GeneratingTableCell(_numCells);
        _numCells++;
        vector<string> gates;
        bool           setValueResult = false;

        if (type == GeneratingTableType::NORMAL
            || type == GeneratingTableType::DEFAULT)
        {
            /*
             * [0]  適用開始時刻    time to start applying
             * [1]  適用終了時刻    time to end applying
             * [2]  出発地ID        origin node ID   
             * [3]  目的地ID        destination node ID
             * [4]  交通量          generation volume
             * [5]  車種ID          vehicle type ID
             * [6]  経由地の数      number of gates
             * [7+] 経由地IDの列挙  list of gate IDs
             */
            _getGates(&gates, &tokens, 2, 3, 6, 7);
            setValueResult = cell->setValues(
                stoul(tokens[0]), stoul(tokens[1]),
                _str2volume(tokens[4]), VehicleType(tokens[5]), gates);
        }
        else if (type == GeneratingTableType::FIXED)
        {
            /*
             * [0]  適用開始時刻    time to start applying
             *   適用終了時刻は開始時刻と同一
             *   Time to end applying is the same as the start
             * [1]  出発地ID        origin node ID   
             * [2]  目的地ID        destination node ID
             *   交通量は指定しない
             *   Generation volume is not given
             * [3]  車種ID          vehicle type ID
             * [4]  経由地の数      number of gates
             * [5+] 経由地IDの列挙  list of gate IDs
             */
            _getGates(&gates, &tokens, 1, 2, 4, 5);
            setValueResult = cell->setValues(
                stoul(tokens[0]), stoul(tokens[0]), 1,
                VehicleType(tokens[3]), gates);
        }
        else if (type == GeneratingTableType::GROUP)
        {
            /*
             * [0]  適用開始時刻            time to start applying
             * [1]  適用終了時刻            time to end applying
             * [2]  出発地グループ化フラグ  origin grouping flag
             * [3]  出発地ID                origin node ID
             * [4]  目的地グループ化フラグ  destination grouping flag   
             * [5]  目的地ID                destination node ID
             * [6]  交通量                  generation volume
             * [7]  車種ID                  vehicle type ID
             * [8]  経由地の数              number of gates
             * [9+] 経由地IDの列挙          list of gate IDs
             */
            _getGates(&gates, &tokens, 3, 5, 8, 9);
            setValueResult = cell->setValues(
                stoul(tokens[0]), stoul(tokens[1]),
                _str2volume(tokens[6]), VehicleType(tokens[7]), gates);
            if (tokens[2] == "P" || tokens[2] == "p")
            {
                cell->setHasPairedODs(true);
            }
            else
            {
                if (tokens[2] == "G" || tokens[2] == "g")
                {
                    cell->setHasGroupedOrigins(true);
                }
                if (tokens[4] == "G" || tokens[4] == "g")
                {
                    cell->setHasGroupedDestinations(true);
                }
            }
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 作成したcellをtableに追加する
        // Add generated cell to table
        if (setValueResult)
        {
            _generator->addGTCellToTable(type, cell);
        }
        else
        {
            cerr << "WARNING: Unknown error occurred in GeneratingTable"
                 << "[type=" << toUnderlying(type) << "]" << endl
                 << "line: " << endl
                 << "  " << line << endl;
            delete cell;
        }
    }
    fin.close();

    // テーブルををソートする
    // sort table
    _generator->sortGeneratingTable(type);

    ss << "done";
    amu::msg::status(cout, ss.str());
    return;
}

//======================================================================
double VehicleGeneratorBuilder::_str2volume(string str) const
{
    return stof(str)
           * AppMates::getGVManager().getNumeric("TABLED_OD_FACTOR");
}

//======================================================================
void VehicleGeneratorBuilder::_getGates(
    vector<string>* result_gates, vector<string>* tokens,
    unsigned int posOrigin, unsigned int posDestination,
    unsigned int posNumGates, unsigned int posMidGates) const
{
    assert(
        posOrigin < tokens->size() && posDestination < tokens->size()
        && posNumGates < tokens->size());
    result_gates->clear();

    // origin
    result_gates->push_back(
        formatId((*tokens)[posOrigin], NUM_FIGURE_FOR_INTERSECTION));

    // mid gates
    int numGates = stoi((*tokens)[posNumGates]);
    if (numGates > 0)
    {
        for (unsigned int i = posMidGates; i < (*tokens).size(); i++)
        {
            result_gates->push_back(
                formatId((*tokens)[i], NUM_FIGURE_FOR_INTERSECTION));
        }
    }

    /* destination
     *   "******"の場合もあるのでformatIdでなくformatIdExtを使う
     *
     * destination
     *   Since there is a case of "******", use formatIdExt instead
     *   of formatId
     */
    result_gates->push_back(formatIdExt(
        (*tokens)[posDestination], NUM_FIGURE_FOR_INTERSECTION));
}
