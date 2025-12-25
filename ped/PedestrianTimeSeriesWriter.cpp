/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file PedestrianTimeSeriesWriter.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "PedestrianTimeSeriesWriter.hpp"
#include "../AppMates.hpp"
#include "../FileManager.hpp"
#include "../GVManager.hpp"
#include "../ObjectManager.hpp"
#include "../VirtualLeader.hpp"
#include <AmuConverter.hpp>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;
using namespace amu::converter;

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
bool PedestrianTimeSeriesWriter::writePedestriansData()
{
    ulint time = AppMates::getTimeManager().time();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 各歩行者の時系列データの出力
    // Output time series data for each pedestrian
    if (AppMates::getGVManager().getFlag(
            "FLAG_OUTPUT_TIMELINE_PEDESTRIAN_D"))
    {
        //--------------------------------------------------------------
        // ファイルを開く
        // Open file
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

        //--------------------------------------------------------------
        // ファイル出力
        // File output
        if (AppMates::getGVManager().getFlag(
                "FLAG_OUTPUT_COMMENT_IN_FILE"))
        {
            string note = TimePrefix + to_string(time);
            fzprintf(fout, "%s\n", note.c_str());
        }

        for (auto itr : AppMates::getObjectManager().pedestrians())
        {
            _writePedestrianData(fout, itr);
        }

        //--------------------------------------------------------------
        // ファイルを閉じる
        // Close file
        int fzcl = fzclose(fout);
        if (fzcl != fz_ok)
        {
            cerr << "ERROR: cannot close file - " << pathStr << endl;
            exit(EXIT_FAILURE);
        }
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 歩行者数の時系列データの出力
    // Output time series data of the number of pedestrians
    if (AppMates::getGVManager().getFlag("FLAG_OUTPUT_TIMELINE_S"))
    {
        ofstream* fout = AppMates::getFileManager().getOFStream(
            AppMates::getGVManager().getString(
                "RESULT_PEDESTRIAN_COUNT_FILE"));

        *fout << time << ","
              << AppMates::getObjectManager().pedestrians().size()
              << endl;

        fout->close();
    }

    return true;
}

//======================================================================
vector<string> PedestrianTimeSeriesWriter::_decideFileName(ulint time)
{
    vector<string> paths;
    string         resultDir = AppMates::getGVManager().getString(
        "RESULT_TIMELINE_DIRECTORY");
    paths.push_back(resultDir + "pedestrian/");

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
bool PedestrianTimeSeriesWriter::_writePedestrianData(
    FILEOUT fout, Pedestrian* ped)
{
    assert(fout && ped);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 基本データの出力
    // output basic data
    const AmuPoint&  p = ped->position();
    const AmuVector& v = ped->velocity() * 1000; //[m/ms]->[m/s]
    stringstream     ss("");
    ss << ped->id() << "," << p.x() << "," << p.y() << "," << p.z()
       << "," << v.size() << "," << v.x() << "," << v.y() << ","
       << v.z() << "," << ped->location()->intersection()->id() << ","
       << ped->location()->zebra()->id() << ","
       << ped->behavior()->crossingDirection();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 文字列をファイルに書き込む
    // Write string to file
    fzprintf(fout, "%s\n", ss.str().c_str());

    return true;
}

#endif //INCLUDE_PEDESTRIANS
