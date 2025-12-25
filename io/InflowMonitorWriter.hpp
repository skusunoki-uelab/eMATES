/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file InflowMonitorWriter.hpp
 */
#ifndef __INFLOW_MONITOR_WRITER_HPP__
#define __INFLOW_MONITOR_WRITER_HPP__
#include <string>

class InflowMonitor;

//######################################################################
/**
 * @~japanese 流入車両の記録をファイル出力する
 * @~english  Output inflow vehicle record to file 
 * @~ @ingroup IO Monitoring
 */
class InflowMonitorWriter
{
public:
    InflowMonitorWriter() {};
    ~InflowMonitorWriter() {};

    /**
     * @~japanese
     * 発生車両検出器 @p monitor に関する出力ファイルを準備する
     *
     * @~english
     * Prepare output file for inflow vehicle monitor @p monitor
     */
    void prepareFile(InflowMonitor* monitor) const;

    /**
     * @~japanese
     * 流入車両検知器 @p monitor の記録をファイル出力する
     *
     * @~english
     * Output record of inflow vehicle monitor @p monitor to file
     */
    void writeRecord(InflowMonitor* monitor);

private:
    /**
     * @~japanese
     * 発生車両検出器 @p monitor の出力ファイル名を戻す
     *
     * @~english
     * Return output file name of inflow vehicle monitor @p monitor
     */
    const std::string _filename(InflowMonitor* monitor) const;
};

#endif //__INFLOW_MONITOR_WRITER_HPP__
