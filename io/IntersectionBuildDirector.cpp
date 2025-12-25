/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file IntersectionBuildDirector.cpp
 */
#include "IntersectionBuildDirector.hpp"
#include "IntersectionBuilder.hpp"
#include "LaneBundleBuilder.hpp"
#include "ODNodeBuilder.hpp"
#include "CSNodeFastBuilder.hpp"
#include "CSNodeNormalBuilder.hpp"
#include "RoadMapBuilder.hpp"
#include "../AppMates.hpp"
#include "../Config.hpp"
#include "../CSNodeFast.hpp"
#include "../CSNodeNormal.hpp"
#include "../CustomMessage.hpp"
#include "../GVManager.hpp"
#include "../Intersection.hpp"
#include "../ODNode.hpp"
#include "../Section.hpp"
#include "../CSNodeBase.hpp"
#ifdef INCLUDE_PEDESTRIANS
#include "../ped/Zebra.hpp"
#endif //INCLUDE_PEDESTRIANS
#include <AmuConverter.hpp>
#include <AmuPoint.hpp>
#include <AmuStringOperator.hpp>
#include <AmuVector.hpp>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <typeinfo>
#include <vector>

using namespace std;
using namespace amu::converter;
using namespace amu::geometry;
using namespace amu::math;
using namespace amu::string_operator;

//#define IB_DEBUG

//==============================================================================
bool IntersectionBuildDirector::buildIntersections()
{
    assert(_roadMap);

    /*
     * 交差点を生成し，隣接交差点を設定し，座標を設定する．接続情報の設定は
     * 交差点生成後でなければならない.
     *
     * Create intersections, set adjacent intersections, and set coordinates.
     * Adjacency must be set after all intersections are created.
     */
    if (_generateCSList() // [eMATES]
        && _generateIntersections()
        && _setAdjacentIntersections()
        && _setPositions()
        && _setCSDeadend()) // [eMATES]
    {
        return true;
    }
    else
    {
        cerr << "Error: buildIntersections failed." << endl;
        return false;
    }
}

//======================================================================
bool IntersectionBuildDirector::_generateCSList()
{
    // eMATESモードでないときは読み込まない
    if (! AppMates::getGVManager().getFlag("FLAG_GEN_CS"))
    {
      return true;
    }

    string fname = AppMates::getGVManager().getString("CS_LIST_FILE");

    // ファイルを読み込む
    // Load file
    ifstream fin(fname.c_str(), ios::in);
    if (!fin)
    {
        cerr << "Error: cannot open file" << fname << "." << endl;
        exit(EXIT_FAILURE);
    }

    while (fin.good())
    {
        string line;
        vector<string> tokens;
        if (!getTokens(&fin, &line, &tokens, ','))
        {
            break;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 不正な行の処理
        // Invalid line handling
        // 修正251225：フィーダーIDを含む4列フォーマットに対応
        if (tokens.size() != 3 && tokens.size() != 4)
        {
            cerr << "ERROR: invalid CS list format - "
                 << line << endl;
            exit(EXIT_FAILURE);
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling

        // 1番目のカラムはCSの識別番号
        // First column is CS ID number
        string id = tokens[0];

        // 2番目のカラムはCSの収容台数
        // Second column is CS capacity
        int capacity = stoi(tokens[1]);

        // 3番目のカラムはCSの定格
        // Third column is CS rating power
        double ratingPower = stod(tokens[2]);

        // 4番目のカラムはフィーダーID（オプション）
        // Fourth column is Feeder ID (optional)
        if (tokens.size() == 4) {
            string feederID = tokens[3];
            _csFeederMap[id] = feederID;
        }

        // CSリストに登録する
        // Register to CS list
        string fmtId = formatId(id, NUM_FIGURE_FOR_INTERSECTION);
        _csList[fmtId] = {capacity, ratingPower};

    }
    fin.close();
    return true;

}

//==============================================================================
bool IntersectionBuildDirector::_generateIntersections()
{
    string fname = AppMates::getGVManager().getString("MAP_NETWORK_FILE");

    // ファイルを読み込む
    // Load file
    ifstream fin(fname.c_str(), ios::in);
    if (!fin)
    {
        cerr << "Error: cannot open file" << fname << "." << endl;
        exit(EXIT_FAILURE);
    }

    while (fin.good())
    {
        string         line;
        vector<string> tokens;
        if (!getTokens(&fin, &line, &tokens, ','))
        {
            break;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 不正な行の処理
        // Invalid line handling
        if (tokens.size() < 3)
        {
            cerr << "ERROR: invalid road network format - " << line << endl;
            exit(EXIT_FAILURE);
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling

        // 1番目のカラムは交差点の識別番号
        // First column is intersection ID number
        string id = tokens[0];

        // 2番目のカラムは交差点のタイプ
        // Second column is intersection type
        string type = tokens[1];

        // デバッグ用フラグが立っていたら交差点のタイプを上書き
        // Overwrite intersection type if debug flag is set
        if (AppMates::getGVManager().getFlag(
                "DEBUG_FLAG_ALL_SECTION_SINGLE_LANE_EACH_SIDE"))
        {
            type = type.replace(0, type.size(), type.size(), '1');
        }

        // Builderに交差点を生成させる
        // Let Builder generate intersection
        string               fmtId = formatId(id, NUM_FIGURE_FOR_INTERSECTION);
        IntersectionBuilder* builder;
        if (type.size() == 2)
        {
          if (_csList.count(fmtId) == 0)
          {
            // typeが2桁の場合はODNodeが生成される
            // ODNode will be generated if type is 2 digits
            builder = new ODNodeBuilder(_roadMapBuilder);
          }
          else
          {
            // [eMATES]
            builder = new CSNodeNormalBuilder(_roadMapBuilder);
          }
        }
        else
        {
          if (_csList.count(fmtId) == 0)
          {
            // 通常の交差点
            // Standard intersection
            builder = new IntersectionBuilder(_roadMapBuilder);
          }
          else
          {
            // [eMATES]
            builder = new CSNodeFastBuilder(_roadMapBuilder);
          }
        }
        Intersection* inter = builder->build(fmtId, type, _roadMap);
        assert(inter);

        // 生成した交差点がCSならば、容量と定格を設定する [eMATES]
        CSNodeBase* cs = dynamic_cast<CSNodeBase*>(inter);
        if (cs && _csList.count(fmtId) > 0)
        {
            // CSリストから該当するCSを検索し、容量と定格を代入する
            auto& csRecord = _csList.at(fmtId);
            int    capacity    = csRecord.capacity;
            double ratingPower = csRecord.ratingPower;
            cs->setCapacity(capacity);
            cs->setRatingPower(ratingPower);
        }

        _roadMap->addIntersection(inter);
        _roadMapBuilder->addIntersectionBuilder(fmtId, builder);
    }
    fin.close();

    // サブコンテナにコピー
    // Copy to subcontainer
    for (auto itr : _roadMapBuilder->intersectionBuilders())
    {
        _builders.emplace_back(itr.second);
    }

    return true;
}

//==============================================================================
bool IntersectionBuildDirector::_setAdjacentIntersections()
{
    string fname = AppMates::getGVManager().getString("MAP_NETWORK_FILE");

    // ファイルを読み込む
    // Load file
    ifstream fin(fname.c_str(), ios::in);
    if (!fin)
    {
        cerr << "Error: cannot open file" << fname << "." << endl;
        exit(EXIT_FAILURE);
    }

    while (fin.good())
    {
        string         line;
        vector<string> tokens;
        if (!getTokens(&fin, &line, &tokens, ','))
        {
            break;
        }

        string id        = tokens[0];
        string type      = tokens[1];
        string thisFmtId = formatId(id, NUM_FIGURE_FOR_INTERSECTION);

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        /*
         * 不正な行の処理
         *   次数がnの交差点のタイプは2n桁でなければならない
         *
         * Invalid line handling
         *   Type of intersection of degree n must be 2n digits
         */
        if (tokens.size() != type.size() / 2 + 2)
        {
            cerr << "ERROR: Intersection[" << thisFmtId << "] must have "
                 << type.size() / 2 << " adjacent Intersections, but there are "
                 << tokens.size() - 2 << " Intersections given." << endl
                 << " - " << line << endl;
            exit(EXIT_FAILURE);
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling
        IntersectionBuilder* builder
            = _roadMapBuilder->intersectionBuilder(thisFmtId);
        Intersection* inter = builder->intersection();
        if (!inter)
        {
            cerr << "ERROR(" << __FILE__ << ":" << __LINE__
                 << "): " << "Intersection[" << thisFmtId << "] not found."
                 << endl;
            exit(EXIT_FAILURE);
        }

        // 隣接交差点のポインタを設定する
        // Set pointers for adjacent intersections
        for (unsigned int i = 2; i < tokens.size(); i++)
        {
            string thatFmtId = formatId(tokens[i], NUM_FIGURE_FOR_INTERSECTION);
            IntersectionBuilder* nextBuilder
                = _roadMapBuilder->intersectionBuilder(thatFmtId);
            Intersection* nextInter = nextBuilder->intersection();
            if (!nextInter)
            {
                cerr << "ERROR: next Intersection[" << thatFmtId
                     << "] of Intersection[" << thisFmtId << "] not found."
                     << endl;
                exit(EXIT_FAILURE);
            }
            /*
             * Builder同士の隣接関係と同時に交差点同士の隣接関係を設定する
             *
             * Set the adjacency relationship between intersections at the same
             * time as the adjacency relationship between builders
             */
            builder->setNextIntersectionBuilder(nextBuilder);
        }
    }
    fin.close();
    return true;
}

//==============================================================================
bool IntersectionBuildDirector::_setPositions()
{
    string fname = AppMates::getGVManager().getString("MAP_POSITION_FILE");

    // ファイルを読み込む
    // Load file
    ifstream fin(fname.c_str(), ios::in);
    if (!fin)
    {
        cerr << "Error: cannot open file" << fname << "." << endl;
        exit(EXIT_FAILURE);
    }

    while (fin.good())
    {
        string         line;
        vector<string> tokens;
        if (!getTokens(&fin, &line, &tokens, ','))
        {
            break;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 不正な行の処理
        // Invalid line handling
        if (tokens.size() != 3 && tokens.size() != 4)
        {
            cerr << "ERROR: invalid map position format - " << line << endl;
            exit(EXIT_FAILURE);
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling
        double x, y, z = 0;
        string id = tokens[0];
        x         = stod(tokens[1]);
        y         = stod(tokens[2]);

        // z座標が指定されていたら設定する
        // Set z-coordinate if given
        if (tokens.size() == 4)
        {
            z = stod(tokens[3]);
        }

        // 座標を交差点にセットする
        // Set coordinates to intersection
        string        fmtId = formatId(id, NUM_FIGURE_FOR_INTERSECTION);
        Intersection* inter = _roadMap->intersection(fmtId);
        if (!inter)
        {
            cerr << "WARNING(" << __FILE__ << ":" << __LINE__
                 << "): " << "Intersection[" << fmtId << "] not found." << endl;
            continue;
        }
        inter->setCenter(AmuPoint(x, y, z));
    }
    fin.close();
    return true;
}

//==============================================================================
bool IntersectionBuildDirector::setIncidentSections()
{
    for (auto itr : _roadMap->sections())
    {
        Section*        section  = itr.second;
        SectionBuilder* sBuilder = _roadMapBuilder->sectionBuilder(section);
        for (int i = 0; i < 2; i++)
        {
            /*
             * 隣接交差点の方向を定め，その間の単路部も同じ方向に関連付けて登録
             *
             * Determine the direction of adjacent intersection, and register
             * the section between two intersections in association with the
             * same direction.
             */
            Intersection*        inter = section->intersection(i);
            IntersectionBuilder* iBuilder
                = _roadMapBuilder->intersectionBuilder(inter);

            int dir = inter->direction(section->anotherIntersection(inter));
            /*
             * Builder同士の隣接関係と同時に交差点同士の隣接関係を設定する
             *
             * Set the adjacency relationship between intersections at the same
             * time as the adjacency relationship between builders
             */
            iBuilder->setNextSectionBuilder(dir, sBuilder);

            // あわせて単路部と_linkFlowRecodsの関連付けもおこなう
            // Also, associate the section to the link travel record.
            LinkFlowRecord* record = inter->linkFlowRecord(dir);
            record->setSection(section);
            record->setDirection(dir);
            section->setLinkFlowRecord(i, record);
        }
    }
    return true;
}

//==============================================================================
bool IntersectionBuildDirector::setInternalInfo()
{
    // フラグが立っていたらファイルによる設定を無視する
    // Ignore file setting if flag is set
    bool ignoresFile = false;
    if (AppMates::getGVManager().getFlag(
            "DEBUG_FLAG_ALL_SECTION_SINGLE_LANE_EACH_SIDE"))
    {
        ignoresFile = true;
    }

    return (
        (ignoresFile || _setWalkWidth())
        && _setInternalStructure(!ignoresFile));
}

//==============================================================================
bool IntersectionBuildDirector::_setWalkWidth()
{
    GVManager&    gv    = AppMates::getGVManager();
    string        fname = gv.getString("INTERSECTION_STRUCT_FILE");
    ostringstream ss;
    ss << "read intersection struct file (" << gv.stripDataDir(fname)
       << ") ... ";

    // ファイルを読み込む
    // Load file
    ifstream fin(fname.c_str(), ios::in);
    if (!fin)
    {
        ss << "not found";
        amu::msg::status(cout, ss.str());
        return true;
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

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 不正な行の処理
        // Invalid line handling
        if (tokens.size() != 3)
        {
            cerr << "WARNING: intersection struct"
                 << " - invalid number of token" << endl;
            cerr << "  " << line << endl;
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling

        // 1番目のカラムは交差点の識別番号
        // First column is intersection ID number
        string id = formatId(tokens[0], NUM_FIGURE_FOR_INTERSECTION);
        IntersectionBuilder* builder = dynamic_cast<IntersectionBuilder*>(
            _roadMapBuilder->intersectionBuilder(id));
        if (!builder)
        {
            cerr << "WARNING: intersection struct" << " - IntersectionBuilder["
                 << id << "] does not exist." << endl;
            continue;
        }

        // 2番目のカラムは歩道幅
        // Second column is sidewalk width
        builder->setSidewalkWidth(atof(tokens[1].c_str()));

        // 3番目のカラムは横断歩道幅
        // Third column is crosswalk width
        builder->setCrosswalkWidth(atof(tokens[2].c_str()));
    }
    fin.close();

    ss << "done";
    amu::msg::status(cout, ss.str());
    return true;
}

//==============================================================================
bool IntersectionBuildDirector::_setInternalStructure(bool readsFile)
{
    /*
     * 交差点設定ファイルを読み，相対方向テーブル，多角形の頂点を作る．あわせて
     * レーン接続情報を読み，保存しておく(生成は後で行う)．全交差点の車道頂点が
     * ないと横断歩道を作れないので先に作る
     *
     * Read intersection configuration file and create relative direction table
     * and polygon vertexes. At the same time, read and save lane connection
     * configuration (creation will be done later). Because crosswalks cannot be
     * created without polygon vertexes at all intersections, define them first.
     */
    string ssPre("read intersection configuration file ... ");
    amu::msg::status(cout, ssPre);
    for (auto itr : _roadMapBuilder->intersectionBuilders())
    {
        IntersectionBuilder* internalBuilder
            = dynamic_cast<IntersectionBuilder*>(itr.second);
        if (!internalBuilder)
        {
            // ODNodeは処理の対象としない
            // ODNode is not processed
            if (dynamic_cast<ODNodeBuilder*>(itr.second))
            {
                amu::msg::warn(
                    "ODNode[" + itr.first
                    + "] is not needed to set internal structure");
                continue;
            }
            else
            {
                amu::msg::error(
                    "Builder[" + itr.first + "] is not an IntersectionBuilder");
                exit(EXIT_FAILURE);
            }
        }
        Intersection* inter = internalBuilder->intersection();
        assert(inter);

        bool     buildsFromFile = false;
        ifstream fin;
        if (readsFile)
        {
            string path = AppMates::getGVManager().getString(
                "INTERSECTION_ATTRIBUTE_DIRECTORY");
            string fname = path + inter->id() + ".txt";
            fin.open(fname.c_str(), ios::in);

            if (fin)
            {
                if (typeid(*inter) == typeid(ODNode)
                    || typeid(*inter) == typeid(CSNodeFast) // [eMATES]
                    || typeid(*inter) == typeid(CSNodeNormal)) // [eMATES]
                {
                    // ODNode ならファイルを無視する
                    // Ignore configuration file
                    ostringstream ss;
                    ss << "ignore ODNode[" << inter->id()
                       << "] configuration file";
                    amu::msg::warn(ss.str());
                }
                else
                {
                    ostringstream ss;
                    ss << ssPre << "Intersection[" << inter->id() << "]";
                    amu::msg::status(cout, ss.str());
                    buildsFromFile = true;
                }
            }
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 内部構造を決定する
        // Define internal structure
        bool result;
        if (buildsFromFile)
        {
            result = internalBuilder->setInternalInfo(&fin);
            fin.close();
        }
        else
        {
            result = internalBuilder->setInternalInfo(nullptr);
        }
        if (!result)
        {
            return false;
        }
    }

    amu::msg::status(cout, (ssPre + "done"));
    return true;
}

//==============================================================================
bool IntersectionBuildDirector::_setCSDeadend()
{
    // eMATESモードでないときは実行しない
    if (! AppMates::getGVManager().getFlag("FLAG_GEN_CS"))
    {
      return true;
    }

    for (CSNodeBase* cs : _roadMap->csNodesFast())
    {
        bool deadend = cs->next(0)->numNexts() == 1 || cs->next(1)->numNexts() == 1;
        cs->setDeadend(deadend);
    }
    return true;
}
