/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file ConvoyMonitorWriter.hpp
 */
#ifndef __CONVOY_MONITOR_WRITER_HPP__
#define __CONVOY_MONITOR_WRITER_HPP__
#include <string>

class ConvoyMonitor;

//######################################################################
/**
 * @~japanese 車列観測記録をファイル出力する
 * @~english  Output vehicle convoy record to file
 * @~ @ingroup IO Monitoring
 */
class ConvoyMonitorWriter
{
public:
    ConvoyMonitorWriter() {};
    ~ConvoyMonitorWriter() {};

    /**
     * @~japanese
     * 車列観測器 @p monitor に関する出力ファイルを準備する
     *
     * @~english
     * Prepare output file for vehicle convoy monitor @p monitor
     */
    void prepareFile(ConvoyMonitor* monitor) const;

    /**
     * @~japanese
     * 車列観測器 @p monitor の記録をファイル出力する
     *
     * @~english
     * Output record of vehicle convoy monitor @p monitor to file
     */
    void writeRecord(ConvoyMonitor* monitor) const;

private:
    /**
     * @~japanese
     * 車列観測器 @p monitor の出力ファイル名を戻す
     *
     * @~english
     * Return output file name of vehicle convoy monitor @p monitor
     */
    const std::string _filename(ConvoyMonitor* monitor) const;
};

#endif //__CONVOY_MONITOR_WRITER_HPP__
