#include "EVCommIO.hpp"
#include "../AppMates.hpp"
#include "../Vehicle.hpp"
#include "../VehicleEV.hpp"
#include "../GVManager.hpp"
#include "../Charger.hpp"
#include "../CSNodeFast.hpp"
#include "../CSNodeNormal.hpp"
#include <AmuConverter.hpp>
#include <fstream>
#include <chrono>
#ifdef USE_MINGW
#include <io.h>
#else // USE_MINGW
#include <sys/types.h>
#include <sys/stat.h>
#endif // USE_MINGW
#include <cassert>

using namespace std;

//======================================================================
EVCommIO::EVCommIO()
{
    _roadMap = nullptr;
    string commonDir   = AppMates::getGVManager().getString("EV_COMM_COMMON_DIRECTORY");
    string scenario    = AppMates::getGVManager().getString("EV_COMM_SCENARIO_NAME");
    string ematesName  = AppMates::getGVManager().getString("EV_COMM_EMATES_DIRECTORY_NAME");
    string opendssName = AppMates::getGVManager().getString("EV_COMM_OPENDSS_DIRECTORY_NAME");
    _ematesDir  = commonDir + scenario + "/" + ematesName  + "/";
    _opendssDir = commonDir + scenario + "/" + opendssName + "/";

    _trafficPrefix = AppMates::getGVManager().getString("EV_COMM_TRAFFIC_PREFIX");
    _energyPrefix  = AppMates::getGVManager().getString("EV_COMM_ENERGY_PREFIX");
    _arrivalPrefix = AppMates::getGVManager().getString("EV_COMM_ARRIVAL_PREFIX");
    _statePrefix   = AppMates::getGVManager().getString("EV_COMM_STATE_PREFIX");

    string errorFile = AppMates::getGVManager().getString("EV_COMM_ERROR_FILE");
    _ematesError  = _ematesDir  + errorFile;
    _opendssError = _opendssDir + errorFile;
}


//======================================================================
void EVCommIO::setRoadMap(RoadMap* roadMap)
{
    _roadMap = roadMap;
}


//======================================================================
EVCommIO& EVCommIO::instance()
{
    static EVCommIO instance;
    return instance;
}


//======================================================================
bool EVCommIO::writeCSTrafficData() const
{
    assert(_roadMap);

    // 出力ファイルストリーム
    int second = static_cast<int>(AppMates::getTimeManager().time()/1000);
    auto strSecond = amu::converter::formatId(to_string(second), 6);
    string filePath = _ematesDir + _trafficPrefix + strSecond + ".csv";
    ofstream ofs(filePath);
    if(!ofs){
        cerr << "writeCSTrafficData(): " << filePath << " cannot be opened!" << endl;
        return false;
    }

    // 凡例行
    const int timeSlot = AppMates::getGVManager().getNumeric("EV_COMM_PREDICTION_TIME_SLOT");
    const int duration = AppMates::getGVManager().getNumeric("EV_COMM_PREDICTION_SLOT_TIME_MINUTE");
    ofs << "Type"
        << ",Csid"
        << ",Cgrid"
        << ",Cap_kW";
    for(int i=0; i<timeSlot; i++)
    {
        ofs << ",Vol_" << (duration * i) << "min";
    }
    ofs << endl;

    for (const CSNodeBase* csNode : _roadMap->csNodes())
    {
        auto csId = csNode->id();

        const CSNodeFast* csNodeFast = dynamic_cast<const CSNodeFast*>(csNode);
        if (csNodeFast)
        {
            // CSNodeFast -- 立ち寄り充電＝急速充電
            string type = "F";
            for (ChargerFast* cgr : csNodeFast->chargers())
            {
                int cgrId = cgr->id();
                // 2023/1/12 by uchida
                // dssの設定ファイルであるcsPosition_dss.csvの仕様に対応するため
                // T******.csvのCsidは本来のCsidとChgridを結合したものに変更している
                string combCsId = csId + amu::converter::formatId(to_string(cgrId), 2);

                // 出力制御値は充電施設に紐付く
                // by abe 2022/11/4 車両がいなければ出力ゼロ。制御値をOpenDSSから与える
                double cap_kW = cgr->isEmpty() ? 0 : csNode->outPower();
                ofs << type
                    << "," << combCsId
                    << "," << cgrId
                    << "," << cap_kW;
                for (int volume : cgr->volumesConst())
                {
                    ofs << "," << volume;
                }
                ofs << endl;
            }
        }
    } // for csNode

    for (const CSNodeBase* csNode : _roadMap->csNodes())
    {
        auto csId = csNode->id();

        const CSNodeFast* csNodeFast = dynamic_cast<const CSNodeFast*>(csNode);
        if (!csNodeFast)
        {
            // CSNodeNormal -- 目的地充電＝普通充電
            string type = "N";
            int cgrId = 0;
            ChargerBase* cgr = csNode->charger(cgrId);
            // by abe 2022/11/4 車両がいなければ出力ゼロ。制御値をOpenDSSから与える
            // 注：普通充電は充電器1台で複数車両を扱うため、丸め処理不要と認識
            double cap_kW = cgr->isEmpty() ? 0 : csNode->outPower();
            ofs << type
                << "," << csId
                << "," << cgrId
                << "," << cap_kW;
            for (int volume : cgr->volumesConst())
            {
                ofs << "," << volume;
            }
            ofs << endl;
        }

    } // for csNode

    return true;
}


//======================================================================
bool EVCommIO::readCSEnergyData()
{
    assert(_roadMap);

    // 入力ファイルストリーム
    int second = static_cast<int>(AppMates::getTimeManager().time()/1000);
    // 2022/12/26 by uchida
    // OpenDSSのファイル仕様（下一桁が1になる）に対応するため数字を加算
    // 仕様を統一した場合には元に戻すべし
    auto strSecond = amu::converter::formatId(to_string(second + 1), 6);
    string filePath = _opendssDir + _energyPrefix + strSecond + ".csv";
    ifstream ifs(filePath);
    if(!ifs){
        cerr << "readCSEnergyData(): " << filePath << " cannot be opened!" << endl;
        return false;
    }

    const int timeSlot = AppMates::getGVManager().getNumeric("EV_COMM_PREDICTION_TIME_SLOT");

    string line;
    getline(ifs, line); // 1行読み飛ばし
    while(getline(ifs, line))
    {
        if ( line == "" || line[0] == '#' )
        {
            continue;
        }
        vector<string> tokens;
        amu::string_operator::getTokens(&tokens, line, ",");
        // 2022/12/26 by uchida
        // E******.csvの先頭2列を読み飛ばす変更
        assert(tokens.size() == 4 + timeSlot*2);

        //string NodeType = tokens[0];
        //string openDssNodeId = tokens[1];
        string csId = tokens[2];
        int cgrId = stoi(tokens[3]);

        // 2023/1/12 by uchida
        // dssの設定ファイルであるcsPosition_dss.csvの仕様に対応するため
        // E******.csvのCsidは本来のCsidとChgridを結合したものに変更している
        // したがってCsidの上6桁をCsidに変換しなおす処理が必要
        csId = csId.substr(0,6);

        // 2023/1/13 by uchida
        // E******.csvの先頭2列を読み飛ばす変更
        double current_kW = stod(tokens[4]);
        auto* intersection = _roadMap->intersection(csId);
        auto* csNode = dynamic_cast<CSNodeBase*>(intersection);
        assert(csNode);
        ChargerBase* charger = csNode->charger(cgrId);
        csNode->setOutPower(current_kW); // by abe 2022/11/4

        // 各vectorは現在時刻を含む
        vector<double>& kW  = charger->outPowers();
        vector<double>& yen = charger->prices();
        kW.clear();
        yen.clear();
        for(size_t i=0; i<timeSlot; i++)
        {
            // 2022/12/26 by uchida
            // E******.csvの先頭2列を読み飛ばす変更
            kW.push_back(  stod(tokens[i + 4]           ) );
            yen.push_back( stod(tokens[i + 4 + timeSlot]) );
        }
    }
    return true;
}


//======================================================================
bool EVCommIO::writeCSArriveList(const std::vector<Vehicle*>* vehicles)
{
    assert(_roadMap && vehicles);

    // 出力ファイルストリーム
    int second = static_cast<int>(AppMates::getTimeManager().time()/1000);
    auto strSecond = amu::converter::formatId(to_string(second), 6);
    string filePath = _ematesDir + _arrivalPrefix + strSecond + ".csv";
    ofstream ofs(filePath);
    if(!ofs){
        cerr << "writeCSArriveList(): " << filePath << " cannot be opened!" << endl;
        return false;
    }

    // 凡例行
    ofs << "Type"
        << ",Csid"
        << ",Cgrid"
        << ",Evid"
        << ",SoC"
        << endl;

    for(Vehicle* vehicle : *vehicles){
        VehicleEV* ev = dynamic_cast<VehicleEV*>(vehicle);
        if (! ev)
        {
            continue;
        }

        // 充電中でなければskip
        if (! ev->isChargingInCS() )
        {
            continue;
        }

        string type {};
        string csId {};
        int cgrId {-1};
        auto evId = ev->id();
        double soc = ev->SOC();

        auto* intersection = const_cast<Intersection*>(ev->location()->intersection());
        csId = intersection->id();

        const auto* csNodeFast  = dynamic_cast<CSNodeFast*>( intersection );

        if ( csNodeFast ) // CSNodeFast
        {
            type = "F";
            // 充電中の充電器の探索
            const auto& cgr = csNodeFast->chargingVehicles();
            const auto itr = find(cgr.begin(), cgr.end(), ev);
            if ( itr == cgr.end() )
            {
                continue;
            }
            cgrId = distance(cgr.begin(), itr);
        }
        else // CSNodeNormal
        {
            type = "N";
            // 普通充電器は充電器を区別しない
            cgrId = 0;
        }

        ofs << type
            << "," << csId
            << "," << cgrId
            << "," << evId
            << "," << soc
            << endl;
    }
    return true;
}


//======================================================================
bool EVCommIO::readCSStateList(std::vector<Vehicle*>* vehicles)
{
    // 現状、eMATESでは使用しない
    return true;

    //assert(vehicles);

    //// 入力ファイルストリーム
    //int second = static_cast<int>(TimeManager::time()/1000);
    //auto strSecond = amu::converter::formatId(to_string(second), 6);
    //string filePath = _opendssDir + _statePrefix + strSecond + ".csv";
    //ifstream ifs(filePath);
    //if(!ifs){
    //    cerr << "readCSStateList(): " << filePath << " cannot be opened!" << endl;
    //    return false;
    //}

    //string line;
    //getline(ifs, line); // 1行読み飛ばし
    //while(getline(ifs, line))
    //{
    //    vector<string> tokens;
    //    amu::string_operator::getTokens(&tokens, line, ",");
    //    string type = tokens[0];
    //    string openDssNodeId = tokens[1];
    //    string csId = tokens[2];
    //    int cgrId  = stoi(tokens[3]);
    //    int evId   = stoi(tokens[4]);
    //    double soc = stod(tokens[5]);

    //    // NOTE ここで上記変数を用いた処理を書く想定。
    //}
    //return true;
}


//======================================================================
bool EVCommIO::waitAndReadOpenDSS(std::vector<Vehicle*>* vehicles)
{
    int second = static_cast<int>(AppMates::getTimeManager().time()/1000);
    // 2022/12/26 by uchida
    // OpenDSSのファイル仕様（下一桁が1になる）に対応するため数字を加算
    // 仕様を統一した場合には元に戻すべし
    auto strSecond = amu::converter::formatId(to_string(second + 1), 6);
    string ePath = _opendssDir + _energyPrefix + strSecond + ".csv";
    string sPath = _opendssDir + _statePrefix + strSecond + ".csv";

    // タイムアウト時間
    int timeout_s = AppMates::getGVManager().getNumeric("EV_COMM_TIMEOUT_SECOND");
    auto limitTime = chrono::system_clock::now() + std::chrono::seconds(timeout_s);

    cout << "Waiting openDSS... " << ePath << endl;
    while (true)
    {
        // error.csvが存在したら終了
        std::ifstream ife(_opendssError);
        if (ife.is_open())
        {
            return false;
        }

        // E******.csvが存在したらループを抜ける
        std::ifstream ifse(ePath);
        std::ifstream ifss(sPath);
        if (ifse.is_open() && ifss.is_open())
        {
            cout << "Found " << ePath << " / " << sPath << endl;
            break;
        }

        // 最後のE******.csvが存在したらerror.csvが出るまで待機
        // int lastSecond = second - AppMates::getGVManager().getNumeric("EV_COMM_PREDICTION_SLOT_TIME_MINUTE") * 
        //                  AppMates::getGVManager().getNumeric("EV_COMM_PREDICTION_TIME_SLOT") / 60;;
        // auto strLastSecond = amu::converter::formatId(to_string(lastSecond + 1), 6);
        // string lastEPath = _opendssDir + _energyPrefix + strLastSecond + ".csv";
        // std::ifstream ifslast(lastEPath);
        // if (ifslast.is_open())
        // {
        //     cout << "Waiting for error file..." << endl;
        //     // error.csvが存在したら終了
        //     std::ifstream ife2(_opendssError);
        //     if (ife2.is_open())
        //     {
        //         return false;
        //     }
        // }

        // タイムアウト判定
        auto now = chrono::system_clock::now();
        if ( now > limitTime )
        {
            cerr << "Waiting openDSS timed out." << endl;
            return false;
        }
    }

    // 読み込み処理
    bool ret;
    // E******.csv -- 上記待機で読める状態になっているはず
    ret = readCSEnergyData();
    assert(ret);

    // statelist_******.csv
    ret = readCSStateList(vehicles);
    if (! ret)
    {
        cerr << "Failed to read statelist." << endl;
        return false;
    }

    return true;

}


//======================================================================
void EVCommIO::writeErrorAndAbort() const
{
    int second = static_cast<int>(AppMates::getTimeManager().time()/1000);
    std::ofstream ofs(_ematesError);
    if (ofs)
    {
        ofs << "eMATES ERROR at " << second << " sec in simulation" << endl;
        cerr << "eMATES ERROR at " << second << " sec in simulation" << endl;
    }
    else
    {
        cerr << _ematesError << " cannot be opend!" << endl;
    }
    exit(EXIT_FAILURE);
}
