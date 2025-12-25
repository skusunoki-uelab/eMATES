/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file IntersectionBuilder.cpp
 */
#include "IntersectionBuilder.hpp"
#include "RoadMapBuilder.hpp"
#include "SectionBuilder.hpp"
#include "../AppMates.hpp"
#include "../GVManager.hpp"
#include "../Intersection.hpp"
#include "../LaneInIntersection.hpp"
#include "../RelativeDirectionTable.hpp"
#include "../Section.hpp"
#include "../SubIntersection.hpp"
#include "../SubLaneBundle.hpp"
#ifdef INCLUDE_TRAMS
#include "../tram/BorderTram.hpp"
#include "../tram/IntersectionTramExt.hpp"
#endif //INCLUDE_TRAMS
#include <AmuLineSegment.hpp>
#include <AmuPoint.hpp>
#include <AmuStringOperator.hpp>
#include <algorithm>
#include <cassert>
#include <cctype>
#include <iostream>

using namespace std;
using namespace amu::geometry;
using namespace amu::string_operator;

//==============================================================================
IntersectionBuilder::IntersectionBuilder(RoadMapBuilder* roadMapBuilder)
    : LaneBundleBuilder(roadMapBuilder)
{
    _inter = nullptr;
    _nextIBuilders.clear();
    _nextSBuilders.clear();

    _rdTable = nullptr;
    _connectorIds.clear();
    _roadwayVertexes.clear();
    _borderPoints.clear();

    // デフォルト値であり上書きされうる
    // Default values, may be overwritten.
    _sidewalkWidth
        = AppMates::getGVManager().getNumeric("DEFAULT_SIDEWALK_WIDTH");
    _crosswalkWidth
        = AppMates::getGVManager().getNumeric("DEFAULT_CROSSWALK_WIDTH");

#ifdef INCLUDE_TRAMS
    _builderTramExt = nullptr;
#endif //INCLUDE_TRAMS
}

//==============================================================================
IntersectionBuilder::~IntersectionBuilder()
{
#ifdef INCLUDE_TRAMS
    delete _builderTramExt;
#endif //INCLUDE_TRAMS
}

//==============================================================================
Intersection* IntersectionBuilder::build(
    const std::string& fmId, const std::string& type, RoadMap* roadMap)
{
    _inter  = new Intersection(fmId, type, roadMap);
    _bundle = _inter;
#ifdef INCLUDE_TRAMS
    _builderTramExt = new IntersectionBuilderTramExt(_inter, _roadMapBuilder);
#endif //INCLUDE_TRAMS

    return _inter;
}

//==============================================================================
void IntersectionBuilder::setNextIntersectionBuilder(
    IntersectionBuilder* iBuilder)
{
    // 重複があればコメントする
    // Comment if there is a duplication
    if (find(_nextIBuilders.begin(), _nextIBuilders.end(), iBuilder)
        != _nextIBuilders.end())
    {
        cout << "WARNING: Builder of Intersection[" << _inter->id()
             << "] has duplicated adjacent Interseciton["
             << iBuilder->intersection()->id() << "]" << endl;
    }
    _nextIBuilders.emplace_back(iBuilder);
    _nextSBuilders.emplace_back(nullptr);
    _inter->setNext(iBuilder->intersection());
}

//==============================================================================
void IntersectionBuilder::setNextSectionBuilder(
    int dir, SectionBuilder* sBuilder)
{
    // 重複があればコメントする
    // Comment if there is a duplication
    if (find(_nextSBuilders.begin(), _nextSBuilders.end(), sBuilder)
        != _nextSBuilders.end())
    {
        cout << "WARNING: Builder of Intersection[" << _inter->id()
             << "] has duplicated incident Seciton["
             << sBuilder->section()->id() << "]" << endl;
    }
    _inter->setNextSection(dir, sBuilder->section());
    _nextSBuilders[dir] = sBuilder;
}

//==============================================================================
bool IntersectionBuilder::setInternalInfo(ifstream* fin)
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 相対方向テーブル (読み込みおよび生成)
    // Relative direction table (loading and generating)
    if (fin && _includesRDTable(fin))
    {
        _rdTable = _buildRDTableFromFile(fin);
    }
    if (!_rdTable)
    {
        _rdTable = _buildDefaultRDTable();
    }
    assert(_rdTable);
    _inter->setRDTable(_rdTable);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef INCLUDE_TRAMS
    /*
     * 路面電車レーンを持つ交差点の設定
     *   車道頂点よりも前に設定しなければならない
     *
     * Configuration an intersection with tram lane
     *   Must be set before roadway vertexes
     */
    _inter->tramExt()->setNumTramConnectors();
#endif //INCLUDE_TRAMS

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * レーン接続 (読み込みのみ)
     *   ファイル入力がない場合は後で自動的に生成する
     *
     * Lane connection (loading only)
     *   If no file input, generate automatically later.
     */
    bool isLaneConnectionValid = true;
    if (fin && _includesLaneConnection(fin))
    {
        isLaneConnectionValid = _readLaneConnectionFromFile(fin);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 車道頂点座標 (読み込みおよび生成)
    // Coordinates of roadway vertexes (loading and generating)
    bool isRoadwayVertexGenerated = false;
    if (fin && _includesRoadwayVertex(fin))
    {
        isRoadwayVertexGenerated = _generateRoadwayVertexesFromFile(fin);
    }
    if (!isRoadwayVertexGenerated)
    {
        isRoadwayVertexGenerated = _generateDefaultRoadwayVertexes();
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if (!_rdTable || !isLaneConnectionValid || !isRoadwayVertexGenerated)
    {
        cerr << "intersection[" << _inter->id()
             << "] - build internal structure failed." << endl;
        cerr << "  RDTable:          " << (_rdTable ? "valid" : "invalid")
             << endl;
        cerr << "  Lane Connection:  "
             << (isLaneConnectionValid ? "valid" : "invalid") << endl;
        cerr << "  Roadway Vertex:   "
             << (isRoadwayVertexGenerated ? "valid" : "invalid") << endl;
        return false;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 対象の交差点へ反映
    _inter->setRDTable(_rdTable);

    return true;
}

//==============================================================================
bool IntersectionBuilder::_includesRDTable(ifstream* fin)
{
    /*
     * 記述はかならず境界相対方向，レーン接続，頂点座標の順に並んでいる
     * ことを要請する．どれが省略されていてもよいが，順番を入れ替えては
     * ならない．
     *
     * The description must be arranged in the order of relative
     * directions, lane connections, and roadway vertexes. One of them
     * may be omitted, but the order must not be changed.
     */
    bool result = false;

    // 読み込み前の位置を保存する
    // Save position before loading
    skipEmptyLine(fin);
    ifstream::pos_type pos = fin->tellg();

    string line;
    getline(*fin, line);
    vector<string> tokens;
    getTokens(&tokens, line, ',');
    transform(tokens[0].begin(), tokens[0].end(), tokens[0].begin(), ::tolower);

    /*
     * 相対方向テーブルが指定されているのであれば，1カラム目が
     * {s,t,l,r} のいずれか
     *
     * If a relative direction table is given, the first column is
     * either {s,t,l,r}.
     */
    if (tokens[0] == "s" || tokens[0] == "t" || tokens[0] == "l"
        || tokens[0] == "r")
    {
        result = true;
    }

    // 読み込み前の位置に戻す
    // Restore position before loading
    fin->seekg(pos);
    return result;
}

//==============================================================================
bool IntersectionBuilder::_includesLaneConnection(ifstream* fin)
{
    bool result = false;

    // 読み込み前の位置を保存する
    // Save position before loading.
    ifstream::pos_type pos = fin->tellg();

    while ((*fin).good())
    {
        string         line;
        vector<string> tokens;
        if (!getTokens(fin, &line, &tokens, ','))
        {
            break;
        }
        transform(
            tokens[0].begin(), tokens[0].end(), tokens[0].begin(), ::tolower);

        // 相対方向テーブルの指定は読み飛ばす
        // Skip if relative direction table specification
        if (tokens[0] == "s" || tokens[0] == "t" || tokens[0] == "l"
            || tokens[0] == "r")
        {
            continue;
        }

        /*
         * レーン接続の指定であれば1カラム目が"vertex"「でない」
         *
         * If specifying a lane connection, the first column should
         * NOT be "vertex".
         */
        else if (tokens[0] == "vertex")
        {
            break;
        }

        /*
         * 2カラムで各カラムの数字のみで構成されていればレーン接続の
         * 指定とみなす
         *
         * If it consists of 2 columns and only the numbers in each
         * column, consider it as a lane connection specification.
         */
        else if (
            tokens.size() == 2
            && all_of(tokens[0].cbegin(), tokens[0].cend(), ::isdigit)
            && all_of(tokens[1].cbegin(), tokens[1].cend(), ::isdigit))
        {
            result = true;
            break;
        }
    }

    // 読み込み前の位置に戻す
    // Restore position before loading
    fin->seekg(pos);
    return result;
}

//==============================================================================
bool IntersectionBuilder::_includesRoadwayVertex(ifstream* fin)
{
    bool result = false;

    // 読み込み前の位置を保存する
    // Save position before loading
    ifstream::pos_type pos = fin->tellg();

    while (fin->good())
    {
        string         line;
        vector<string> tokens;
        if (!getTokens(fin, &line, &tokens, ','))
        {
            break;
        }
        transform(
            tokens[0].begin(), tokens[0].end(), tokens[0].begin(), ::tolower);
        getline(*fin, line);

        // 頂点座標が指定されているのであれば1カラム目が"vertex"である
        // If roadway vertexes are given, the first column is "vertex".
        if (tokens[0] == "vertex")
        {
            result = true;
            break;
        }
    }

    // 読み込み前の位置に戻す
    // Restore position before loading
    fin->seekg(pos);
    return result;
}

//==============================================================================
bool IntersectionBuilder::createInternalStructure()
{
    /*
     * 手順
     * 1. 歩道および横断歩道幅の設定
     * 2. _roadwayVertexesにもとづく頂点配列の生成
     * 3. 境界の生成
     * 4. サブセクションの生成
     * 5. コネクタの生成
     * 6. レーンの生成
     *
     * Procedure
     * 1. Set sidewalk and crosswalk width
     * 2. Generate vertex list based on _roadwayVertexes
     * 3. Generate boundaries
     * 4. Generate subsections
     * 5. Generate connectors
     * 6. Generate lanes
     */
    return (
        _setSidewalkAndCrosswalkWidth()  //
        && _generateVertexes()           //
        && _generateBorders()            //
        && _generateSubsections()        //
        && _generateInternalConnectors() //
        && _generateLanes());
}

//==============================================================================
bool IntersectionBuilder::_setSidewalkAndCrosswalkWidth()
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Sidewalk
    _inter->setSidewalkWidth(_sidewalkWidth);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Crosswalk
    /*
     * 実際に横断歩道を設置するかどうかかは接続する単路部に依存する
     *
     * @todo crosswalkWidthをIntersectionが持つ必要があるか
     *
     * Whether crosswalk actually installed depends on incident section
     */
    int numNext = _inter->numNexts();

    for (int i = 0; i < numNext; i++)
    {
        //----------------------------------------------------------------------
        // ODNodeおよび次数2の交差点には横断歩道なし
        // No crosswalk installed at ODNode and degree-2 intersections.
        if (numNext <= 2)
        {
            _inter->addCrosswalkWidth(0.0);
            continue;
        }

        int n = (i + 1) % numNext;
        int p = (i + numNext - 1) % numNext;

        //----------------------------------------------------------------------
        /*
         * 処理中の方向に接続する単路部およびその両隣の方向に接続する
         * 単路に歩道がなければ横断歩道なし
         *
         * @todo widthと存在の有無は別に管理すべきでは？
         * 
         * No crosswalk installed if there is no sidewalk on the
         * sections connecting to the direction in process and both
         * side directions.
         */
        if (_nextSBuilders[i]->sidewalkWidth(_inter, true) < 1e-6
            && _nextSBuilders[i]->sidewalkWidth(_inter, false) < 1e-6
            && _nextSBuilders[p]->sidewalkWidth(_inter, true) < 1e-6
            && _nextSBuilders[n]->sidewalkWidth(_inter, false) < 1e-6)
        {
            _inter->addCrosswalkWidth(0.0);
            continue;
        }

        //----------------------------------------------------------------------
        /*
         * 単路部にレーンがなければ横断歩道なし
         *   中央を通ってしまうが許容する．
         *
         * No crosswalk installed if no lanes on the incident section.
         *   Although pedestrian can walk center, it is allowed.
         */
        if (_inter->numIn(i) + _inter->numOut(i) < 1)
        {
            _inter->addCrosswalkWidth(0.0);
            continue;
        }

        //----------------------------------------------------------------------
        // 上記以外の場合に横断歩道が設置される
        // Crosswalks is installed in cases other than the above.
        _inter->addCrosswalkWidth(_crosswalkWidth);
    }
    return true;
}
