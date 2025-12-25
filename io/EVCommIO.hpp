#ifndef __EV_COMM_IO_HPP__
#define __EV_COMM_IO_HPP__

#include <string>
#include <vector>

class RoadMap;
class Vehicle;
class VehicleEV;

/// EV, CSの充電状況の読み書きを行うクラス
/**
 * Singleton
 */
class EVCommIO
{
private:
    EVCommIO();
    ~EVCommIO(){};

public:
    /// RoadMapをセット
    void setRoadMap(RoadMap* roadMap);

    /// 唯一のインスタンスを返す
    static EVCommIO& instance();

    /// 充電器到着状況(T******.csv)を書き出す
    bool writeCSTrafficData() const;

    /// 充電器出力・料金(E******.csv)を読み込む
    bool readCSEnergyData();

    /// 充電中車両SoC(arrivelist_******.csv)を書き出す
    // SOC取得関数が非constなため、const関数にできない
    // vehicles は全車両のリストを指定
    bool writeCSArriveList(const std::vector<Vehicle*>* vehicles);

    /// 充電中車両SoC(statelist_******.csv)を読み込む
    // vehicles は全車両のリストを指定
    bool readCSStateList(std::vector<Vehicle*>* vehicles);

    /// OpenDSSの出力を待機し、準備ができたら読み込む
    bool waitAndReadOpenDSS(std::vector<Vehicle*>* vehicles);

    /// エラー出力して終了する
    void writeErrorAndAbort() const;

private:
    /// RoadMap
    RoadMap* _roadMap;

    /// ディレクトリ
    std::string _ematesDir;
    std::string _opendssDir;

    /// prefix
    std::string _trafficPrefix;
    std::string _energyPrefix;
    std::string _arrivalPrefix;
    std::string _statePrefix;

    std::string _ematesError;
    std::string _opendssError;

};

#endif //__EV_COMM_IO_H__
