/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file SignalBuilder.cpp
 */
#include "SignalBuilder.hpp"
#include "../AppMates.hpp"
#include "../CustomMessage.hpp"
#include "../GVManager.hpp"
#include "../FileManager.hpp"
#include "../Intersection.hpp"
#include "../RoadMap.hpp"
#include "../Signal.hpp"
#include "../SignalCycle.hpp"
#include "../SignalSplit.hpp"
#include <AmuConverter.hpp>
#include <AmuStringOperator.hpp>
#include <cassert>
#include <cstdlib>

using namespace std;
using namespace amu::converter;
using namespace amu::string_operator;

//==============================================================================
bool SignalBuilder::buildSignals(RoadMap* roadMap)
{
    // FLAG_INPUT_SIGNAL が false の場合はすべての信号を設置しない
    // Any signals are not installed if FLAG_INPUT_SIGNAL is false.
    if (!(AppMates::getGVManager().getFlag("FLAG_INPUT_SIGNAL")))
    {
        for (auto itr_i : roadMap->intersections())
        {
            roadMap->addUnsignalizedIntersection(itr_i.second);
        }
        return true;
    }

    assert(roadMap);

    // 信号機を設定しない交差点
    // Unsignalized intersections
    _readUnsignalizedIntersectionFile(roadMap);

    for (auto itr_i : roadMap->intersections())
    {
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 次数1の交差点（ODノード）には信号機を設置しない
        // Not install traffic light at intersections of degree 1
        if (itr_i.second->numNexts() == 1)
        {
            roadMap->addUnsignalizedIntersection(itr_i.second);
            continue;
        }

        /*
         * 次数2であり，かつ現示パターン定義ファイルのない交差点には信号機を
         * 設置しない
         *
         * Mot install traffic lights at intersections that are degree 2 and
         * have no aspect pattern definition file.
         */
        else if (
            itr_i.second->numNexts() == 2
            && !(FileManager::exists(_aspectFileName(itr_i.second->id()))))
        {
            roadMap->addUnsignalizedIntersection(itr_i.second);
            continue;
        }

        /*
         * 無信号交差点として登録されていたら信号機を設置しない
         *
         * Not install traffic light at the intersection which is registered as
         * an unsignalized intersection.
         */
        auto itr_u = find(
            _unsignalizedIntersections.begin(),
            _unsignalizedIntersections.end(), itr_i.second);
        if (itr_u != _unsignalizedIntersections.end())
        {
            roadMap->addUnsignalizedIntersection(itr_i.second);
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        Signal* tmpSignal = _buildSignal(itr_i.second);
        tmpSignal->cycle(0)->activate();

        // RoadMapと該当する交差点に登録する
        // Register to RoadMap and corresponding intersection
        roadMap->addSignal(tmpSignal);
        itr_i.second->setSignal(tmpSignal);

        // サブセクションに登録する
        // Register to Subsection
        for (auto itr_s : itr_i.second->subLaneBundles())
        {
            if (itr_s.second->type() == SubsectionType::Crosswalk)
            {
                int dir = itr_i.second->direction(itr_s.second);
                if (dir != -1)
                {
                    itr_s.second->attachSignal(tmpSignal, dir);
                }
            }
        }
    }

    return true;
}

//==============================================================================
void SignalBuilder::_readUnsignalizedIntersectionFile(RoadMap* roadMap)
{
    assert(roadMap);
    GVManager& gv    = AppMates::getGVManager();
    string     fname = gv.getString("UNSIGNALIZED_INTERSECTION_FILE");

    ostringstream ss;
    ss << "read unsignalized intersection file (" << gv.stripDataDir(fname)
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

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 不正な行の処理
        // Invalid line handling
        if (tokens.size() != 1)
        {
            ostringstream ssw;
            ssw << "invalid unsignalized intersection file format - " << line;
            amu::msg::warn(ssw.str());
            continue;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 正常な行の処理
        // Valid line handling
        string id = formatId(tokens[0], NUM_FIGURE_FOR_INTERSECTION);

        Intersection* inter = roadMap->intersection(id);
        if (!inter)
        {
            ostringstream ssw;
            ssw << "intersection[" << id << "] are not found.";
            amu::msg::warn(ssw.str());
            continue;
        }
        _unsignalizedIntersections.emplace_back(inter);
    }
    fin.close();

    ss << "done";
    amu::msg::status(cout, ss.str());
    return;
}

//==============================================================================
Signal* SignalBuilder::_buildSignal(const Intersection* inter)
{
    assert(inter);
    Signal* signal = new Signal(inter);

    // ファイルからデータを読み込んで信号機に設定する
    // Load data from file and set to traffic light
    _setCycles(signal);
    _setAspectSet(signal);

    return signal;
}

//==============================================================================
void SignalBuilder::_setCycles(Signal* signal) const
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // ファイルを開く
    // Open file
    string   fname = _cycleFileName(signal->id());
    ifstream fin(fname.c_str(), ios::in);
    if (!fin)
    {
        // 入力ファイルが見つからない場合はデフォルトのファイルを開く
        // Open default file if input file not found
        fname = _defaultCycleFileName();
        fin.clear();
        fin.open(fname.c_str(), ios::in);
        if (!fin)
        {
            cerr << "[ERROR] Default timing file not found - " << fname << endl;
            exit(EXIT_FAILURE);
        }
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // ファイルの行の処理
    // File line processing
    unsigned int cycleId = 0;
    while (fin.good())
    {
        string line;

        // 区切り文字が' 'であるため，getAdjustString()を使えない
        // Because deliminater is ' ', getAdjustString() cannot be used.
        getline(fin, line);
        getSubstantialString(&line, '#');
        if (line.empty())
        {
            continue;
        }
        vector<string> tokens;
        getTokens(&tokens, line, " ");

        // stoul()は空白文字を期待したとおりに処理できない可能性がある
        // stoul() may not handle whitespace as expected
        ulint begin     = (unsigned long)atol(tokens[0].c_str());
        ulint end       = (unsigned long)atol(tokens[1].c_str());
        ulint cycleTime = (unsigned long)atol(tokens[2].c_str());

        unsigned int splitId  = 0;
        ulint        splitSum = 0;

        SignalCycle* cycle = new SignalCycle(signal, cycleId, begin, end);
        for (unsigned int i = 3; i < tokens.size(); i++)
        {
            // 最後が空白文字である場合への対処
            // Handling when the end is a blank character
            ulint splitTime;
            try
            {
                splitTime = stoul(tokens[i]);
            }
            catch (...)
            {
                continue;
            }

            SignalSplit* split
                = new SignalSplit(signal, cycle, splitId, splitTime);
            cycle->addSplit(split);
            splitId++;
            splitSum += splitTime;
        }
        cycle->makeSplitLoop();
        cycleId++;

        signal->addCycle(cycle);

        // スプリットの和はサイクルと等しくなければならない
        // Sum of splits must equal cycle
        if (splitSum != cycleTime)
        {
            cerr << "ERROR: Signal[" << signal->id()
                 << "] has inconsistent splits" << " - cycle=" << cycleTime
                 << ", sum of splits=" << splitSum << endl;
            exit(EXIT_FAILURE);
        }
    }
    fin.close();

    signal->makeCycleSequence();
}

//==============================================================================
void SignalBuilder::_setAspectSet(Signal* signal) const
{
    /*
     * .msaファイルのカラム数
     *   各方向にメイン，サブ，歩行者の3つの状態があるので，カラム数は隣接する
     *   交差点の数の3倍
     *
     * The number of columns in the .msa file
     *   There are 3 states in each direction (main, sub, pedestrian), so the
     *   number of columns is three times the number of adjacent intersections.
     */
    int numSides = signal->intersection()->numNexts();
    int numCols  = numSides * 3;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // ファイルを開いて1行目に書かれた行数を読む
    // Open file and read the number of lines written in the first line.
    string       fname = _aspectFileName(signal->id());
    unsigned int numLines;

    ifstream fin(fname.c_str(), ios::in);
    if (fin)
    {
        // 該当の信号用の入力ファイルが見つかった場合
        // If input file for specified traffic light is found
        fin >> numLines;
    }
    else
    {
        // 入力ファイルが見つからない場合はデフォルトのファイルを開く
        // Open default file if input file not found
        fname = _defaultAspectFileName(numSides);
        fin.clear();
        fin.open(fname.c_str(), ios::in);
        if (fin.good())
        {
            fin >> numLines;
        }
        else
        {
            cerr << "[ERROR] Default aspect file not found - " << fname << endl;
            exit(EXIT_FAILURE);
        }
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 2行目以降の処理
    // Processing from the second line onwards
    //vector<SignalCycle*>& cycles = signal->cycleTable().cycles();
    for (unsigned int line = 0; line < numLines; line++)
    {
        vector<SignalAspect::State> states;
        vector<int>                 buffers;
        buffers.resize(numCols);
        for (int i = 0; i < numCols && fin.good(); i++)
        {
            fin >> buffers[i];
        }
        for (int i = 0; i < numCols; i += 3)
        {
            SignalAspect::State state;
            state.main = static_cast<SignalColor::MainState>(buffers[i]);
            state.sub  = static_cast<SignalColor::SubState>(buffers[i + 1]);
            state.walker
                = static_cast<SignalColor::WalkerState>(buffers[i + 2]);
            states.push_back(state);
        }
        /*
         * 現状では現示の順序は固定であるため，すべてのサイクルパターンに
         * statesをコピーする．
         * 
         * Currently, the aspect order is fixed, so states are copied to all
         * cycle patterns.
         */
        for (auto itr : signal->cycles())
        {
            itr->split(line)->setAspectStates(states);
        }
    }
    fin.close();
}

//==============================================================================
string SignalBuilder::_cycleFileName(const string& id) const
{
    string defaultPath
        = AppMates::getGVManager().getString("SIGNAL_CONTROL_DIRECTORY");
    string extension
        = AppMates::getGVManager().getString("CONTROL_FILE_EXTENSION");
    return defaultPath + id + extension;
}

//==============================================================================
string SignalBuilder::_defaultCycleFileName() const
{
    string defaultPath
        = AppMates::getGVManager().getString("SIGNAL_CONTROL_DIRECTORY");
    string extension
        = AppMates::getGVManager().getString("CONTROL_FILE_EXTENSION");
    string defaultFilename
        = AppMates::getGVManager().getString("SIGNAL_CONTROL_FILE_DEFAULT");
    return defaultPath + defaultFilename + extension;
}

//==============================================================================
string SignalBuilder::_aspectFileName(const string& id) const
{
    string defaultPath
        = AppMates::getGVManager().getString("SIGNAL_CONTROL_DIRECTORY");
    string aspectExtension
        = AppMates::getGVManager().getString("ASPECT_FILE_EXTENSION");
    return defaultPath + id + aspectExtension;
}

//==============================================================================
string SignalBuilder::_defaultAspectFileName(int numSides) const
{
    string defaultPath
        = AppMates::getGVManager().getString("SIGNAL_CONTROL_DIRECTORY");
    string defaultAspectPrefix = AppMates::getGVManager().getString(
        "SIGNAL_ASPECT_FILE_DEFAULT_PREFIX");
    string aspectExtension
        = AppMates::getGVManager().getString("ASPECT_FILE_EXTENSION");
    return defaultPath + defaultAspectPrefix + to_string(numSides)
        + aspectExtension;
}