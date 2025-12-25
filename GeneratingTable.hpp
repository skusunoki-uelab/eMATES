/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file GeneratingTable.hpp
 */
#ifndef __GENERATING_TABLE_HPP__
#define __GENERATING_TABLE_HPP__
#include "Config.hpp"
#include "CustomMessage.hpp"
#include "VehicleType.hpp"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <deque>

//##############################################################################
/**
 * @~japanese GeneratingTableに格納される要素
 * @~english  Element stored in GeneratingTable
 */
struct GeneratingTableCell
{
public:
    explicit GeneratingTableCell(ulint id)
        : _id(id), _vehicleType(VehicleType())
    {
        _begin  = 0;
        _end    = 0;
        _volume = 0;

        _hasPairedODs           = false;
        _hasGroupedOrigins      = false;
        _hasGroupedDestinations = false;
        _numGeneratedVehicles   = 0;
    }
    ~GeneratingTableCell() {};

    /**
     * @~japanese 値を一括してコピーする
     * @~english  Copy values collectively
     */
    bool setValues(
        ulint begin, ulint end, double volume, VehicleType type,
        const std::vector<std::string>& gates)
    {
        if (begin <= end && volume >= 0)
        {
            _begin       = begin;
            _end         = end;
            _volume      = volume;
            _vehicleType = type;
        }
        else
        {
            _begin = 0;
            _end   = 0;
            _gates.clear();
            return false;
        }
        copy(gates.begin(), gates.end(), back_inserter(_gates));
        return true;
    }

    /**
     * @~japanese 発生済み車両台数をインクリメントする
     * @~english  Increment the number of generated vehicles
     */
    void incrementGeneratedVehicles()
    {
        _numGeneratedVehicles++;
    }

    /**
     * @~japanese 車両発生パラメータを @p out に出力する
     * @~english  Output vehicle generation parameters to @p out 
     */
    void print(std::ostream& out) const
    {
        std::ostringstream oss;
        oss << "ID: " << _id << ", beginTime: " << _begin
            << ", endTime: " << _end << ", volume: " << _volume
            << ", vehicleType: " << _vehicleType << std::endl;
        oss << "origin: " << origin() << ", destination: " << destination()
            << std::endl;
        oss << "gates: ";

        if (_gates.size() == 2)
        {
            oss << "none";
        }
        else
        {
            for (unsigned int i = 1; i < _gates.size() - 1; i++)
            {
                oss << _gates[i] << " ";
            }
        }
        amu::msg::message(out, oss.str());
    }

protected:
    /**
     * @~japanese 識別番号
     * @~english  ID number
     */
    ulint _id;

    /**
     * @~japanese 適用開始時刻
     * @~english  Start time of application
     */
    ulint _begin;

    /**
     * @~japanese 適用終了時刻
     * @~english  End time of application
     */
    ulint _end;

    /**
     * @~japanese 発生交通量 [veh./h]
     * @~english  Generated traffic volume [veh./h]
     */
    double _volume;

    /**
     * @~japanese 発生済み車両台数 [veh./h]
     * @~english  Number of vehicles already generated [veh./h]
     */
    ulint _numGeneratedVehicles;

    /**
     * @~japanese 車種
     * @~english  Vehicle type
     */
    VehicleType _vehicleType;

    /**
     * @~japanese 通過交差点のID
     * @note 最初の要素が出発地，最後の要素が目的地をあらわす
     *
     * @~english  ID numbers of way-points
     * @note The first component is the origin and the last is the destination.
     */
    std::vector<std::string> _gates;

    //==========================================================================
    /**
     * @~japanese @name グループ化されたODをあつかうためのフラグ
     * @~english  @name Flags for handling grouped ODs 
     */
    ///@{
private:
    /**
     * @~japanese ODペアかどうか
     * @~english  Whether OD pair or not
     */
    bool _hasPairedODs;

    /**
     * @~japanese 出発地がグループ化されているかどうか
     * @~english  Whether origins are grouped
     */
    bool _hasGroupedOrigins;

    /**
     * @~japanese 目的地がグループ化されているかどうか
     * @~english  Whether destinations are grouped
     */
    bool _hasGroupedDestinations;

    ///@}

    //==========================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    ulint id() const
    {
        return _id;
    }

    ulint begin() const
    {
        return _begin;
    }

    ulint end() const
    {
        return _end;
    }

    double volume() const
    {
        return _volume;
    }

    unsigned int numGeneratedVehicles() const
    {
        return _numGeneratedVehicles;
    }

    VehicleType vehicleType() const
    {
        return _vehicleType;
    }

    const std::string& origin() const
    {
        assert(_gates.size() >= 2);
        return *(_gates.begin());
    }

    const std::string& destination() const
    {
        assert(_gates.size() >= 2);
        return *(_gates.rbegin());
    }

    const std::vector<std::string>& gates() const
    {
        return _gates;
    }

    bool hasPairedODs() const
    {
        return _hasPairedODs;
    }

    void setHasPairedODs(bool hasPairedODs)
    {
        _hasPairedODs = hasPairedODs;
    }

    bool hasGroupedOrigins() const
    {
        return _hasGroupedOrigins;
    }

    void setHasGroupedOrigins(bool hasGroupedOrigins)
    {
        _hasGroupedOrigins = hasGroupedOrigins;
    }

    bool hasGroupedDestinations() const
    {
        return _hasGroupedDestinations;
    }

    void setHasGroupedDestinations(bool hasGroupedDestinations)
    {
        _hasGroupedDestinations = hasGroupedDestinations;
    }

    ///@}
};

//######################################################################
/**
 * @~japanese 車両発生データを管理する
 *
 * @note 初期設定は VehicleGeneratorBuilder でおこなう
 *
 * @~english  Manage vehicle generation data
 *
 * @note Initial setting are done with VehicleGeneratorBuilder
 *
 * @~ @ingroup Vehicle Running
 */
class GeneratingTable
{
public:
    GeneratingTable()
    {
        _table.clear();
        _activatedTable.clear();
    }

    ~GeneratingTable()
    {
        for (auto itr : _table)
        {
            delete itr;
        }
        _table.clear();
        for (auto itr : _activatedTable)
        {
            delete itr;
        }
        _activatedTable.clear();
        ;
    }

    /**
     * @~japanese GeneratingTableCell @p cell を追加する
     * @~english  Add GeneratingTableCell @p cell
     */
    void addGTCell(GeneratingTableCell* cell)
    {
        _table.emplace_back(cell);
    }

    /**
     * @~japanese
     * 現在時刻に適用可能なセルを抽出し @p result_cells に格納する
     *
     * @note
     * 抽出されたセルは _table から取り除かれて _activatedTable に追加される
     *
     * @~english
     * Extract the cells applicable to the current time and store them
     * in @p result_cells
     *
     * @note
     * Extracted cells are removed from _table and added to _activatedCells.
     */
    void extractActiveGTCells(
        std::vector<const GeneratingTableCell*>& result_cells);

    /**
     * @~japanese 有効なセルを一度にすべて抽出する
     * @note fixedGenerateTableのデバッグ用に用いる
     *
     * @~english  Extract all valid cells at one once
     * @note Used for debugging fixedGenerateTable.
     */
    void extractActiveGTCellsAllAtOnce(
        std::vector<const GeneratingTableCell*>& result_cells);

    /**
     * @~japanese
     * 識別番号 @p id で指定した出発地を持つセルを @p result_cells に
     * 格納する
     *
     * @~english
     * Store the cells with origin specified by the ID number @p id in
     * @p result_cells  
     */
    void getValidGTCells(
        const std::string&                       id,
        std::vector<const GeneratingTableCell*>& result_cells) const;

    /**
     * @~japanese
     * 識別番号 @p id で指定した出発地を持つ最初のセルを戻す
     *
     * @~english
     * Return the first cell with origin specified by the ID number @p id.
     */
    const GeneratingTableCell* validGTCell(const std::string& id) const;

    /**
     * @~japanese セルを整列する
     * @~english  Sort cells
     */
    void sortGTCells();

    //==========================================================================
public:
    /**
     * @~japanese 格納しているセルを @p out に出力する
     * @~english  Output the containing cell to @p out
     */
    void print(std::ostream& out) const;

    //==========================================================================
protected:
    /**
     * @~japanese 有効化前の GeneratingTableCell のコンテナ
     *
     * @note
     * 要素は適用開始時刻でソートする．先頭の要素を頻繁に削除する必要があるが，
     * 挿入するのは稀．
     *
     * @~english  Container for GeneratingTableCell before activation
     *
     * @note
     * Components are sorted by start time of application. Necessary to delete
     * the head frequently, but rare for inserting.
     */
    std::deque<GeneratingTableCell*> _table;

    /**
     * @~japanese 有効化後の GeneratingTableCell のコンテナ
     * @~english  Container for GeneratingTableCell after activation
     */
    std::vector<GeneratingTableCell*> _activatedTable;
};

#endif // __GENERATING_TABLE_HPP__
