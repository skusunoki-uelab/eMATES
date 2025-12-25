/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file SectionBuildDirector.cpp
 */
#include "SectionBuildDirector.hpp"
#include "RoadMapBuilder.hpp"
#include "SectionBuilder.hpp"
#include "../AppMates.hpp"
#include "../Config.hpp"
#include "../CustomMessage.hpp"
#include "../GVManager.hpp"
#include "../Intersection.hpp"
#include "../Lane.hpp"
#include "../LaneBundle.hpp"
#include "../LaneInSection.hpp"
#include "../RoadMap.hpp"
#include "../Section.hpp"
#include "../SubLaneBundle.hpp"
#include "../SubSection.hpp"
#ifdef INCLUDE_TRAMS
#include "../tram/SectionTramExt.hpp"
#endif //INCLUDE_TRAMS
#include <AmuConverter.hpp>
#include <AmuLineSegment.hpp>
#include <AmuPoint.hpp>
#include <AmuStringOperator.hpp>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;
using amu::converter::formatId;
using amu::string_operator::getAdjustString;
using amu::string_operator::getTokens;

//==============================================================================
bool SectionBuildDirector::buildSections()
{
    assert(_roadMap);

    /*
     * 交差点の情報から単路部を生成するとともに接続交差点を設定し，
     * 境界上の流出入コネクタ数を決定する
     *
     * Generate sections from intersection information, set incident
     * intersections, and determine the number of inflow and outflow
     * connectors on the boundary
     */
    if (_generateSections())
    {
        return true;
    }
    else
    {
        cerr << "Error: buildSections failed." << endl;
        return false;
    }
}

//==============================================================================
bool SectionBuildDirector::_generateSections()
{
    // 接続関係を持つ交差点間を結ぶ単路部を作成する
    // Generate sections that connect intersections
    for (auto itr : _roadMap->intersections())
    {
        int numNext = itr.second->numNexts();
        for (int i = 0; i < numNext; i++)
        {
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            // IDを決定する
            // Determine ID number
            Intersection* begin = NULL;
            Intersection* end   = NULL;
            string        id    = "";

            /*
             * 処理対象の交差点と隣接交差点のIDを比べ， 前者のIDが
             * 大きければ単路部を生成しない．隣接交差点から対象交差点に
             * 向けて生成される．
             *
             * Compare IDs of the intersections to be processed and the
             * the adjacent intersection, and if the ID of former is
             * larger, not generate a section. It is generated in the
             * adjacent intersection toward the intersection to be
             * processed now.
             */
            if (itr.second->id() > itr.second->next(i)->id())
            {
                continue;
            }

            id    = itr.second->id() + itr.second->next(i)->id();
            begin = const_cast<Intersection*>(itr.second);
            end   = const_cast<Intersection*>(itr.second->next(i));
            assert(begin != NULL && end != NULL);

            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            /*
             * Builderに単路部を生成させる
             *   既に同じIDの単路部が作られていないかチェックする
             *
             * Let Builder generate section
             *   Check if the section with the same ID has already been
             *   generated.
             */
            if (_roadMap->section(id))
            {
                cerr << "ERROR: section from intersection[" << itr.second->id()
                     << "] to intersection[" << itr.second->next(i)->id()
                     << "] already exists." << endl;
                return false;
            }

            SectionBuilder* builder = new SectionBuilder(_roadMapBuilder);
            Section*        section = builder->build(id, begin, end, _roadMap);

            _roadMap->addSection(section);
            _roadMapBuilder->addSectionBuilder(id, builder);
        }
    }

    // サブコンテナにコピー
    // Copy to subcontainer
    for (auto itr : _roadMapBuilder->sectionBuilders())
    {
        _builders.emplace_back(itr.second);
    }
    return true;
}

//==============================================================================
bool SectionBuildDirector::setInternalInfo()
{
    // 交差点と同じく_setInternalStructureが必要になればここに追加する
    // If _setInternalStructure needed like intersections, add it here.
    return _setUpWidthAndPermission(true);
}

//==============================================================================
bool SectionBuildDirector::_setUpWidthAndPermission(bool readsFile)
{
    GVManager&    gv    = AppMates::getGVManager();
    string        fname = gv.getString("SECTION_STRUCT_FILE");
    ostringstream ss;

    // ファイルを読み込む
    // Load file
    ifstream fin;
    if (readsFile)
    {
        ss << "read section struct file (" << gv.stripDataDir(fname)
           << ") ... ";

        fin.open(fname.c_str(), ios::in);
        if (!fin)
        {
            ss << "not found";
            amu::msg::status(cout, ss.str());
            return true;
        }
        amu::msg::status(cout, ss.str());
    }
    if (!readsFile || !fin.good())
    {
        return true;
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
        if (tokens.size() < 7
            || static_cast<signed int>(tokens.size()) != 7 + stoi(tokens[6]))
        {
            cerr << "WARNING: section struct" << " invalid number of token"
                 << endl;
            cerr << "  " << line << endl;
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling

        // 1番目および2番目のカラムは交差点の識別番号
        // First and second columns are intersection ID numbers
        string beginInterId = formatId(tokens[0], NUM_FIGURE_FOR_INTERSECTION);
        string endInterId   = formatId(tokens[1], NUM_FIGURE_FOR_INTERSECTION);

        // 単路部の識別番号を決定してBuilderオブジェクトを取得
        // Determine section ID number and get Builder object
        string id;
        if (stoi(beginInterId) < stoi(endInterId))
        {
            id = beginInterId + endInterId;
        }
        else
        {
            id = endInterId + beginInterId;
        }

        SectionBuilder* builder = dynamic_cast<SectionBuilder*>(
            _roadMapBuilder->sectionBuilder(id));
        if (!builder)
        {
            cerr << "Error: section struct" << " - section builder[" << id
                 << "] does not exist." << endl;
            continue;
        }

        // 3番目のカラムはレーン幅
        // Third column is lane width
        builder->setLaneWidth(stod(tokens[2]));

        // 4番目のカラムは路肩幅
        // Forth column is shoulder width
        builder->setRoadsideWidth(stod(tokens[3]));

        // 5番目および6番目のカラムは右および左歩道幅
        // Fifth and sixth columns are right and left sidewalk width
        builder->setSidewalkWidth(false, stod(tokens[4]));
        builder->setSidewalkWidth(true, stod(tokens[5]));

        // 以下は現在無効化されている
        // The following are currently disabled
        /*
         * 7番目のカラムは車道に進入可能な歩行者タイプの数
         *
         * Seventh column is number of pedestrian types that can enter
         * the roadway.
         */
        // int numTypes = stoi(tokens[6]);

        /*
         * 8番目以降のカラムは車道を通行可能な歩行者タイプ
         *
         * Eighth or later columns are pedestrian types that can enter
         * the roadway.
         */
        /*
        for (int i=0; i<numTypes; i++)
        {
            if (tokens[7 + i] == "*")
            {
                builder->addRoadwayPedestrianType(
                    TRAFFIC_WALKER_TYPE_ANY);
            }
            else
            {
                builder->addRoadwayPedestrianType(
                    static_cast<TrafficCategory>(stoi(tokens[7+i])));
            }
        }
        */
    }
    fin.close();

    ss << "done";
    amu::msg::status(cout, ss.str());
    return true;
}
