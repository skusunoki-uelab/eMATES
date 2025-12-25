/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file ObjectManager.hpp
 */
#ifndef __OBJECT_MANAGER_HPP__
#define __OBJECT_MANAGER_HPP__
#include "Connector.hpp"
#include "ConvoyMonitor.hpp"
#include "InflowMonitor.hpp"
#include "LinkFlowMonitor.hpp"
#include "ManagerBase.hpp"
#include "TrafficCounter.hpp"
#include "Vehicle.hpp"
#include "VehicleEV.hpp"
#ifdef INCLUDE_PEDESTRIANS
#include "ped/InflowPedestrianMonitor.hpp"
#include "ped/Pedestrian.hpp"
#endif //INCLUDE_PEDESTRIANS
#ifdef INCLUDE_TRAMS
#include "tram/VehicleTram.hpp"
#endif //INCLUDE_TRAMS
#include <vector>
#include <deque>
#include <iostream>
#include <string>

//##############################################################################
/**
 * @~japaneseVolumeMonitor
 * エージェントとオブジェクトの生成・消去および識別番号を管理する
 *
 * @todo
 * ConnectorとMonitorsはRoadMapに管理させてもいいのでは？
 * VehicleはLaneかLaneBundleに管理させてもいいのでは？
 *
 * @~english
 * Manage creation/deletion and global ID numbers of agents and objects
 *
 * @~
 * @ingroup Manager
 */
class ObjectManager : public ManagerBase
{
    friend class ManagerPool;

private:
    ObjectManager();
    ~ObjectManager()
    {
        deleteAll();
    }

    /**
     * @~japanese 親クラスの関数のオーバーライド
     * @~english  Override functions of parent class
     */
    void _finalizeInside() override {};

    //==========================================================================
private:
    /**
     * @~japanese コネクタのメインコンテナ
     * @~english  Main container of connectors
     */
    std::vector<Connector*> _connectors;

    /**
     * @~japanese 車両感知器のメインコンテナ
     * @~english  Main container of traffic counters
     */
    std::vector<TrafficCounter*> _trafficCounters;

    /**
     * @~japanese リンク交通流観測器のメインコンテナ
     * @~english  Main container of link traffic flow monitor
     */
    std::vector<LinkFlowMonitor*> _linkFlowMonitors;

    /**
     * @~japanese 流入車両検出器のメインコンテナ
     * @~english  Main container of inflow vehicle monitor
     */
    std::vector<InflowMonitor*> _inflowMonitors;

    /**
     * @~japanese 車列観測器のメインコンテナ
     * @~english  Main container of vehicle convoy monitor
     */
    std::vector<ConvoyMonitor*> _convoyMonitors;

#ifdef INCLUDE_PEDESTRIANS
    /**
     * @~japanese 流入歩行者検知器のメインコンテナ
     * @~english  Main container of inflow pedestrian monitor
     */
    std::vector<InflowPedestrianMonitor*> _inflowPedestrianMonitors;

#endif //INCLUDE_PEDESTRIANS

    /**
     * @~japanese 道路地図上に存在する車両のメインコンテナ
     * @~english  Main container of vehicles exists on road map
     */
    std::vector<Vehicle*> _vehicles;

    /**
     * @~japanese
     * 地図に追加された車両数

     * 車両の識別番号として使用．追加された順に付ける．
     * 消去される車両もあるので_vehicles.size()とは異なる．
     *
     * @~english
     * Number of vehicles added to road map
     *
     * Used to assign vehicle ID number. It is assigned in order of
     * being added. Different from _vehicles.size(),
     * because vehicles may be deleted.
     */
    unsigned int _numVehicles;

#ifdef INCLUDE_PEDESTRIANS
    /**
     * @~japanese 道路地図上に存在する歩行者のメインコンテナ
     * @~english  Main container of pedestrians exists on road map
     */
    std::vector<Pedestrian*> _pedestrians;

    /**
     * @~japanese
     * 地図に追加された歩行者数

     * 歩行者の識別番号として使用．追加された順に付ける．
     * 消去される歩行者もあるので_pedestrians.size()とは異なる．
     *
     * @~english
     * Number of pedestrians added to road map
     *
     * Used to assign pedestrian ID number. It is assigned in order of
     * being added. Different from _pedestrians.size(),
     * because pedestrians may be deleted.
     */
    unsigned int _numPedestrians;

#endif //INCLUDE_PEDESTRIANS
    /*
     * コネクタや感知器ユニットはシミュレーションの途中で
     * 消去されないのでnumConnectorsやnumDetectorUnitsは必要ない
     *
     * Not need numConnectors or numDetectorUnits because they are
     * not deleted during simulation.
     */

public:
    //==========================================================================
    /**
     * @~japanese @name オブジェクト全般に関する操作
     * @~english  @name General object operations
     */
    ///@{

    /**
     * @~japanese オブジェクトをすべて消去する
     * @~english  Delete all object
     */
    void deleteAll();

    /**
     * @~japanese エージェントをすべて消去する
     * @~english  Delete all agent
     */
    void deleteAllAgents();

    ///@}

    //==========================================================================
    /**
     * @~japanese @name コネクタに関する操作
     * @~english  @name Operations related to connectors
     */
    ///@{

    /**
     * @~japanese コネクタのコンテナへのポインタを返す
     * @~english  Return pointer to connectors' container
     */
    std::vector<Connector*>& connectors()
    {
        return _connectors;
    }

    /**
     * @~japanese
     * 点( @p x , @p y , @p z ) の位置にコネクタを生成する
     *
     * @return
     * 生成されたコネクタへのポインタ
     *
     * @~english
     * Generate connector at ( @p x , @p y , @p z )
     *
     * @return
     * Pointer to generated connector
     */
    Connector* createConnector(double x, double y, double z);

    /**
     * @~japanese すべてのコネクタを消去する
     * @~english  Delete all connectors
     */
    void deleteAllConnectors();

    ///@}

    //==========================================================================
    /**
     * @~japanese @name 観測機器に関する操作
     * @~english  @name Operations related to monitors
     */
    ///@{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
     * @~japanese 車両感知器のコンテナを戻す
     * @~english  Return traffic counter container
     */
    std::vector<TrafficCounter*>& trafficCounters()
    {
        return _trafficCounters;
    }

    /**
     * @~japanese 識別番号 @p id を持つ車両感知器を返す
     * @~english  Return traffic counter unit with the ID number @p id
     */
    TrafficCounter* trafficCounter(const std::string& id);

    /**
     * @~japanese 車両感知器 @p counter をコンテナに追加する
     * @return    正常に追加されたかどうか
     *
     * @~english  Add traffic counter @p counter
     * @return    Whether it was added successfully
     *
     * @~ @see TrafficCounterBuilder
     */
    bool addTrafficCounter(TrafficCounter* counter);

    /**
     * @~japanese すべての車両感知器を消去する
     * @~english  Delete all traffic counters
     */
    void deleteAllTrafficCounters();

    /**
     * @~japanese 車両感知器の属性を @p out に出力する
     * @~english  Output traffic counter attributes to @p out
     */
    void printTrafficCounters(std::ostream& out) const;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
     * @~japanese リンク交通流観測器のコンテナを戻す
     * @~english  Return link traffic flow monitor container
     */
    std::vector<LinkFlowMonitor*>& linkFlowMonitors()
    {
        return _linkFlowMonitors;
    }

    /**
     * @~japanese リンク交通流観測器 @p monitor をコンテナに追加する
     * @return    正常に追加されたかどうか
     *
     * @~english  Add link traffic flow monitor @p monitor
     * @return    Whether it was added successfully
     */
    bool addLinkFlowMonitor(LinkFlowMonitor* monitor);

    /**
     * @~japanese すべてのリンク交通流観測器を消去する
     * @~english  Delete all link traffic flow monitors
     */
    void deleteAllLinkFlowMonitors();

    /**
     * @~japanese リンク交通流観測器の属性を @p out に出力する
     * @~english  Output link traffic flow monitor attributes to @p out
     */
    void printLinkFlowMonitors(std::ostream& out) const;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
     * @~japanese 流入車両検出器のコンテナを戻す
     * @~english  Return inflow vehicle monitor container
     */
    std::vector<InflowMonitor*>& inflowMonitors()
    {
        return _inflowMonitors;
    }

    /**
     * @~japanese 流入車両検出器 @p monitor をコンテナに追加する
     * @return    正常に追加されたかどうか
     *
     * @~english  Add inflow vehicle monitor @p monitor
     * @return    Whether it was added successfully
     */
    bool addInflowMonitor(InflowMonitor* monitor);

    /**
     * @~japanese すべての流入車両検知器を消去する
     * @~english  Delete all inflow vehicle monitors
     */
    void deleteAllInflowMonitors();

    /**
     * @~japanese 流入車両検出器の属性を @p out に出力する
     * @~english  Output inflow vehicle monitor attributes to @p out
     */
    void printInflowMonitors(std::ostream& out) const;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
     * @~japanese 車列観測器のコンテナを戻す
     * @~english  Return vehicle convoy monitor container
     */
    std::vector<ConvoyMonitor*>& convoyMonitors()
    {
        return _convoyMonitors;
    }

    /**
     * @~japanese 車列観測器 @p monitor をコンテナに追加する
     * @return    正常に追加されたかどうか
     *
     * @~english  Add vehicle convoy monitor @p monitor
     * @return    Whether it was added successfully
     */
    bool addConvoyMonitor(ConvoyMonitor* monitor);

    /**
     * @~japanese すべての車列観測器を消去する
     * @~english  Delete all vehicle convoy monitors
     */
    void deleteAllConvoyMonitors();

    /**
     * @~japanese 車列観測器の属性を @p out に出力する
     * @~english  Output convoy monitor attributes to @p out
     */
    void printConvoyMonitors(std::ostream& out) const;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef INCLUDE_PEDESTRIANS
    /**
     * @~japanese 流入歩行者検知器のコンテナを戻す
     * @~english  Return inflow pedestrian monitor container
     */
    std::vector<InflowPedestrianMonitor*>& inflowPedestrianMonitors()
    {
        return _inflowPedestrianMonitors;
    }

    /**
     * @~japanese 流入歩行者検出器 @p monitor をコンテナに追加する
     * @return    正常に追加されたかどうか
     *
     * @~english  Add inflow pedestrian monitor @p monitor
     * @return    Whether it was added successfully
     */
    bool addInflowPedestrianMonitor(InflowPedestrianMonitor* monitor);

    /**
     * @~japanese すべての流入歩行者検知器を消去する
     * @~english  Delete all inflow pedestrian monitors
     */
    void deleteAllInflowPedestrianMonitors();

    /**
     * @~japanese
     * 流入歩行者検出器の属性を @p out に出力する
     *
     * @~english
     * Output inflow pedestrian monitor attributes to @p out
     */
    void printInflowPedestrianMonitors(std::ostream& out) const;

#endif //INCLUDE_PEDESTRIANS
    ///@}

    //==========================================================================
    /**
     * @~japanese @name 車両に関する操作
     * @~english  @name Operations related to vehicles
     */
    ///@{

    /**
     * @~japanese 車両のコンテナへのポインタを返す
     * @~english  Return pointer to vehicles' container
     */
    std::vector<Vehicle*>& vehicles()
    {
        return _vehicles;
    }

    /**
     * @~japanese 識別番号 @p id を持つ車両を返す
     * @~english  Return vehicle with the ID number @p id.
     */
    Vehicle* vehicle(const std::string& id);

    /**
     * @~japanese
     * 車両 (自動車) を生成する
     *
     * @note
     * この時点では識別番号を付与しない．渋滞が発生点まで延伸している
     * 場合，自動車は生成されるが地図に追加されない．
     *
     * @return
     * 生成された車両へのポインタ
     *
     * @~english
     * Generate vehicle (car)
     *
     * @note
     * No ID number given at this time. If traffic jams extend to
     * origin point, car is generated but not added to the road map.
     *
     * @return
     * Pointer to generated vehicle
     */
    Vehicle* createVehicle();

    VehicleEV* createVehicleEV(); // [eMATES]

#ifdef INCLUDE_TRAMS
    /**
     * @~japanese 路面電車を生成する
     * @~english  Generate tram
     */
    VehicleTram* createTram();
#endif //INCLUDE_TRAMS

    /**
     * @~japanese
     * 車両を地図に追加する
     *
     * @note
     * この段階で識別番号を付与する
     *
     * @~english
     * Add vehicle to road map
     *
     * @note
     * Give ID number at this time
     */
    bool addVehicleToRoadMap(Vehicle* vehicle);

    /**
     * @~japanese すべての車両を消去する
     * @~english  Delete all vehicles
     */
    void deleteAllVehicles();

    /**
     * @~japanese 車両 @p vehicle を消去する
     * @~english  Delete vehicle @p vehicle
     */
    void deleteVehicle(Vehicle* vehicle);

    ///@}

#ifdef INCLUDE_PEDESTRIANS
    //==========================================================================
    /**
     * @~japanese @name 車両に関する操作
     * @~english  @name Operations related to vehicles
     */
    ///@{

    /**
     * @~japanese 歩行者のコンテナへのポインタを返す
     * @~english  Return pointer to pedestrians' container
     */
    std::vector<Pedestrian*>& pedestrians();

    /**
     * @~japanese 識別番号 @p id を持つ歩行者を返す
     * @~english  Return pedestrian with the ID number @p id.
     */
    Pedestrian* pedestrian(const std::string& id);

    /**
     * @~japanese
     * 歩行者を生成する
     *
     * @note
     * Simulator から呼ばれる．この時点では識別番号を付与しない．
     *
     * @return
     * 生成された歩行者へのポインタ
     *
     * @~english
     * Generate pedestrian
     *
     * @note
     * No ID number given at this time. Called from Simulator.
     *
     * @return
     * Pointer to generated pedestrian
     */
    Pedestrian* createPedestrian();

    /**
     * @~japanese
     * 歩行者 @p pedestrian を地図に追加する
     *
     * @note
     * この段階で識別番号を付与する
     *
     * @~english
     * Add pedestrian @p pedestrian to road map
     *
     * @note
     * Give ID number at this time
     */
    bool addPedestrianToRoadMap(Pedestrian* pedestrian);

    /**
     * @~japanese すべての歩行者を消去する
     * @~english  Delete all pedestrians
     */
    void deleteAllPedestrians();

    /**
     * @~japanese 歩行者 @p pedestrian を消去する
     * @~english  Delete pedestrian @p pedestrian
     */
    void deletePedestrian(Pedestrian* pedestrian);

    ///@}

#endif //INCLUDE_PEDESTRIAN
};

#endif //__OBJECT_MANAGER_HPP__
