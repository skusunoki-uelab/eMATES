/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file TrafficCounterWriter.hpp
 */
#ifndef __TRAFFIC_COUNTER_WRITER_HPP__
#define __TRAFFIC_COUNTER_WRITER_HPP__
#include "../Config.hpp"
#include <string>

class TrafficCounter;

//##############################################################################
/**
 * @~japanese
 * 車両感知器が記録した情報をファイルに出力する
 *
 * @~english
 * Write information recorded by traffic counters to file
 *
 * @~
 * @ingroup IO Monitoring
 */
class TrafficCounterWriter
{
public:
    TrafficCounterWriter() {};
    ~TrafficCounterWriter() {};

    /**
     * @~japanese
     * 車両感知器 @p counter に関する出力ファイルを準備する
     *
     * @~english
     * Prepare output files for traffic counter @p counter
     */
    void prepareFiles(TrafficCounter* counter);

    /**
     * @~japanese
     * 車両感知器 @p counter の記録をファイル出力する
     * 
     * @~english
     * Output record of traffic counter @p counter observation result to files
     */
    void writeRecords(
        TrafficCounter* counter, //
        bool outputsDetailedFile, bool outputsAggregatedFile);

private:
    /**
     * @~japanese
     * 車両感知器 @p counter の詳細データ用出力ファイル名を戻す
     *
     * @~english
     * Return output file name for detailed data of traffic counter @p counter
     */
    const std::string _detailedFilename(TrafficCounter* counter) const;

    /**
     * @~japanese
     * 車両感知器 @p counter の集計データ用出力ファイル名を戻す
     *
     * @~english
     * Return output file name for aggregated data of traffic counter
     * @p counter
     */
    const std::string _aggregatedFilename(TrafficCounter* counter) const;
};

#endif //__TRAFFIC_COUNTER_WRITER_HPP__
