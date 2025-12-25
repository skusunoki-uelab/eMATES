/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleTimeSeriesWriter.cpp
 */
#include "VehicleTimeSeriesWriter.hpp"
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
bool VehicleTimeSeriesWriter::writeVehiclesData()
{
    ulint time = AppMates::getTimeManager().time();

    // (出力対象の)車両のカウンター
    // (Output target) vehicle counter
    unsigned int nOutput = 0;
    unsigned int nAll;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 各車両の時系列データの出力
    // Output time series data for each vehicle
    if (AppMates::getGVManager().getFlag(
            "FLAG_OUTPUT_TIMELINE_VEHICLE_D"))
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

        auto vehicles = AppMates::getObjectManager().vehicles();
        nAll          = vehicles.size();
        for (auto itr : vehicles)
        {
            /*
             * 車両データの出力
             *   発生直後の自動車は速度が非現実的であるという理由から
             *   出力されない場合がある．
             *
             * Output vehicle data
             *   Vehicle immediately after its generation may not be
             *   output because its speed is unrealistic
             */
            if (itr->isAwayFromOriginNode())
            {
                _writeVehicleData(fout, itr);
                nOutput++;
            }
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
    // 車両台数の時系列データの出力
    // Output time series data of the number of vehicles
    if (AppMates::getGVManager().getFlag("FLAG_OUTPUT_TIMELINE_S"))
    {
        ofstream* fout = AppMates::getFileManager().getOFStream(
            AppMates::getGVManager().getString(
                "RESULT_VEHICLE_COUNT_FILE"));

        /*
         * 車両ごとの時系列データを出力していなければ，
         * ここでnOutputをカウントする
         *
         * If not outputting time series data for each vehicle,
         * count nOutput here
         */
        if (!(AppMates::getGVManager().getFlag(
                "FLAG_OUTPUT_TIMELINE_VEHICLE_D")))
        {
            auto vehicles = AppMates::getObjectManager().vehicles();
            nAll          = vehicles.size();
            for (auto itr : vehicles)
            {
                if (itr->isAwayFromOriginNode())
                {
                    nOutput++;
                }
            }
        }
        *fout << time << "," << nAll << "," << nOutput << endl;

        fout->close();
    }

    return true;
}

//======================================================================
vector<string> VehicleTimeSeriesWriter::_decideFileName(ulint time)
{
    vector<string> paths;
    string         resultDir = AppMates::getGVManager().getString(
        "RESULT_TIMELINE_DIRECTORY");
    paths.push_back(resultDir + "vehicle/");

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
bool VehicleTimeSeriesWriter::_writeVehicleData(
    FILEOUT fout, Vehicle* vehicle)
{
    assert(fout && vehicle);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 角度の計算 [deg]
     *   theta = xy平面に投影したときの(0,1,0)方向との間の角
     *          (反時計回りを正とする)
     *   phi   = 仰角 (上り勾配を正，下り勾配を負とする)
     *
     * Calculate angle [deg]
     *   theta = angle between (0,1,0) directions when projected
     *           onto the xy plane * (counterclockwise is positive)
     *   phi   = elevation angle (upward positive, downward negative)
     */
    double theta
        = vehicle->directionVector().calcAngle(AmuVector(0, 1, 0)) * 180
          * M_1_PI;
    // 戻り値は[-180,180)であるので，[0, 360)に変更
    // Return value is [-180,180), so change it to [0, 360)
    if (theta < 0)
    {
        theta += 360;
    }

    double phi = atan(vehicle->location()->lane()->gradient() / 100.0)
                 * 180 * M_1_PI;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 基本データの出力
    // output basic data
    AmuPoint     p = vehicle->location()->position();
    stringstream ss("");
    ss << vehicle->id() << "," << *(vehicle->body()->type()) << ","
       << p.x() << "," << p.y() << "," << p.z() << "," << theta << ","
       << phi << "," << vehicle->velocity() * 1000 << ","
       << vehicle->accel() * 1.0e+6 << ","
       << (vehicle->location()->intersection()
               ? vehicle->location()->intersection()->id()
               : "NULL")
       << ","
       << (vehicle->location()->section()
               ? vehicle->location()->section()->id()
               : "NULL");

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 追加情報出力
     *   0: なし，1: ウィンカ，2: 仮想先行車ID
     *
     * Additional data output
     *   0: none, 1: blinker, 2: virtual leader ID
     */
    static unsigned int extension
        = static_cast<unsigned int>(AppMates::getGVManager().getNumeric(
            "ADDITIONAL_VEHICLE_DATA_LEVEL"));
    if (extension >= 1)
    {
        ss << "," << vehicle->blinker()->direction();
        if (extension >= 2)
        {
            const vector<const VirtualLeader*>& leaders
                = vehicle->scene()->leaders();
            ss << "," << leaders.size();
            for (auto itr : leaders)
            {
                ss << "," << itr->vlTypeString() << "/"
                   << itr->annotation() << "/" << itr->distance();
            }
        }
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 文字列をファイルに書き込む
    // Write string to file
    fzprintf(fout, "%s\n", ss.str().c_str());

    return true;
}

