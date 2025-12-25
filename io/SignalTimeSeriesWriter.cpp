/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file SignalTimeSeriesWriter.cpp
 */
#include "SignalTimeSeriesWriter.hpp"
#include "../AppMates.hpp"
#include "../FileManager.hpp"
#include "../GVManager.hpp"
#include "../Intersection.hpp"
#include "../TimeManager.hpp"
#include <AmuConverter.hpp>

using namespace std;
using amu::converter::formatId;

//======================================================================
/**
 * @~japanese 内部リンケージのための無名名前空間
 * @~english  Anonymous namespace for internal linkage
 */
namespace
{
    /**
     * @~japanese
     * @name
     * zlib使用の有無の違いを吸収するためのエイリアス
     * 
     * @~english
     * @name
     * Aliases to absorb difference between using or not using zlib
     */
    ///@{
#ifdef USE_ZLIB
    constexpr auto fzopen   = gzopen;
    constexpr auto fzclose  = gzclose;
    constexpr auto fzprintf = gzprintf;
    constexpr auto fz_ok    = Z_OK;
    const string   fzmode   = "wb6f";
#else  //USE_ZLIB
    constexpr auto fzopen   = fopen;
    constexpr auto fzclose  = fclose;
    constexpr auto fzprintf = fprintf;
    constexpr auto fz_ok    = 0;
    const string   fzmode   = "w";
    ///@}
#endif //USE_ZLIB

    /**
     * @~japanese
     * ファイルの1行目に挿入するコメントのプリフィクス
     *
     * @~english
     * Prefix of comment to insert at the first line of the file
     */
    static const string TimePrefix = "#Time=";
}

//======================================================================
bool SignalTimeSeriesWriter::writeSignalsData(const RoadMap* roadMap)
{
    if (!(AppMates::getGVManager().getFlag(
            "FLAG_OUTPUT_TIMELINE_SIGNAL_D")))
    {
        return true;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // ファイルを開く
    // Open file
    ulint          time  = AppMates::getTimeManager().time();
    vector<string> paths = _decideFileName(time);
    string         pathStr;
    for (auto itr : paths)
    {
        pathStr += itr;
    }

    FILEOUT fout = fzopen(pathStr.c_str(), string(fzmode).c_str());
    if (!fout)
    {
        // ディレクトリが準備されていない場合には作成を試みる
        // Attempt to create directory if not prepared
        paths.pop_back();
        if (AppMates::getFileManager().makeDirectories(paths))
        {
            fout = fzopen(pathStr.c_str(), string(fzmode).c_str());
        }
    }
    // それでもファイルが開けない場合には異常終了
    // Abend if the file still cannot be opened
    if (!fout)
    {
        cerr << "[ERROR] cannot open file - " << pathStr << endl;
        exit(EXIT_FAILURE);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // ファイル出力
    // File output
    if (AppMates::getGVManager().getFlag("FLAG_OUTPUT_COMMENT_IN_FILE"))
    {
        string comment = TimePrefix + to_string(time);
        fzprintf(fout, "%s\n", comment.c_str());
    }

    for (auto itr : roadMap->signals())
    {
        // 信号データの出力
        // Output traffic light data
        _writeSignalData(fout, roadMap, itr.second);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // ファイルを閉じる
    // Close file
    int fzcl = fzclose(fout);
    if (fzcl != fz_ok)
    {
        cerr << "ERROR: cannot close file:" << pathStr << endl;
        exit(EXIT_FAILURE);
    }

    return true;
}

//======================================================================
vector<string> SignalTimeSeriesWriter::_decideFileName(ulint time)
{
    vector<string> paths;
    string         resultDir = AppMates::getGVManager().getString(
        "RESULT_TIMELINE_DIRECTORY");
    paths.push_back(resultDir + "signal/");

    // 時刻を表す文字列を3桁，3桁，4桁に分割する
    // Split time string into 3, 3, and 4 digits
    string strTime
        = formatId(to_string(time), NUM_FIGURE_FOR_TIMELINE_FILENAME);
    paths.push_back(strTime.substr(0, 3) + "/");
    paths.push_back(strTime.substr(3, 3) + "/");
#ifdef USE_ZLIB
    paths.push_back(strTime.substr(6, 4) + ".txt.gz");
#else  // USE_ZLIB
    paths.push_back(strTime.substr(6, 4) + ".txt");
#endif // USE_ZLIB
    return paths;
}

//======================================================================
bool SignalTimeSeriesWriter::_writeSignalData(
    FILEOUT fout, const RoadMap* roadMap, Signal* signal)
{
    assert(fout && roadMap && signal);

    for (int i = 0; i < roadMap->intersection(signal->id())->numNexts();
         i++)
    {
        stringstream ss("");

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        /*
         * 出力用信号機番号を決定する．
         *   Singalクラスのid()とは異なる．"dst(ID) src(ID)"の順に
         *   交差点IDを結合する．例えば交差点[2]から交差点[1]の方向の
         *   信号機番号は"000001000002"となる．
         *
         * Determine traffic light number for output.
         *   Different from id() in Signal class. Intersection IDs are
         *   combined in the order of "dst(ID) src(ID)". For example,
         *   traffic light number in the direction from intersection [2]
         *   to intersection[1] is "000001000002".
         */
        string signalID
            = signal->id()
              + roadMap->intersection(signal->id())->next(i)->id();
        ss << signalID << ",";

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 方向iの信号機の現示
        // Traffic light aspect in direction i
        SignalColor::MainState mainState = signal->mainColor(i);
        SignalColor::SubState  subState  = signal->subColor(i);

        /*
         * 現示によって出力値を決定する
         *   Tram及び歩行者用信号は出力しない．点滅は青扱い．
         *
         * Output value is determined by the aspect.
         *   Tram and pedestrian signals are not output. Blinking is
         *   treated as blue.
         */
        int signalColor = 0;
        if (mainState == SignalColor::MainState::BLUE
            || mainState == SignalColor::MainState::REDBLINK
            || mainState == SignalColor::MainState::YELLOWBLINK)
        {
            signalColor += 32;
        }
        if (mainState == SignalColor::MainState::YELLOW)
        {
            signalColor += 16;
        }
        if (mainState == SignalColor::MainState::RED)
        {
            signalColor += 8;
        }
        if (subState == SignalColor::SubState::ALL
            || subState == SignalColor::SubState::LEFT
            || subState == SignalColor::SubState::STRAIGHTLEFT
            || subState == SignalColor::SubState::LEFTRIGHT)
        {
            signalColor += 4;
        }
        if (subState == SignalColor::SubState::ALL
            || subState == SignalColor::SubState::STRAIGHT
            || subState == SignalColor::SubState::STRAIGHTLEFT
            || subState == SignalColor::SubState::STRAIGHTRIGHT)
        {
            signalColor += 2;
        }
        if (subState == SignalColor::SubState::ALL
            || subState == SignalColor::SubState::RIGHT
            || subState == SignalColor::SubState::STRAIGHTRIGHT
            || subState == SignalColor::SubState::LEFTRIGHT)
        {
            signalColor += 1;
        }
        ss << signalColor;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 文字列をファイルに書き込む
        // Write string to file
        fzprintf(fout, "%s\n", ss.str().c_str());
    }

    return true;
}
