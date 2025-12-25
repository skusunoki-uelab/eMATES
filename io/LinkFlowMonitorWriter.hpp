/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file LinkFlowMonitorWriter.hpp
 */
#ifndef __LINK_FLOW_MONITOR_WRITER_HPP__
#define __LINK_FLOW_MONITOR_WRITER_HPP__
#include <string>

class LinkFlowMonitor;

//##############################################################################
/**
 * @~japanese リンク交通流をファイル出力する
 * @~english  Output link traffic flow to file
 * @~ @ingroup IO Monitoring
 */
class LinkFlowMonitorWriter
{
public:
    LinkFlowMonitorWriter() {}
    ~LinkFlowMonitorWriter() {}

    /**
     * @~japanese
     * リンク交通流観測器 @p monitor に関する出力ファイルを準備する
     *
     * @~english
     * Prepare output file for link traffic flow monitor @p monitor
     */
    void prepareFile(LinkFlowMonitor* monitor) const;

    /**
     * @~japanese
     * リンク交通流観測器 @p monitor の記録をファイルに出力する
     *
     * @~english
     * Output record of link traffic flow monitor @p monitor to files
     */
    void writeRecord(
        LinkFlowMonitor* monitor, //
        bool outputsDetailedFile, bool outputsAggregatedFile) const;

private:
    /**
     * @~japanese
     * リンク交通流観測器 @p monitor の詳細データ用出力ファイル名を戻す
     *
     * @~english
     * Return output file name for detailed data of link traffic flow monitor
     * @p monitor
     */
    const std::string _detailedFilename(LinkFlowMonitor* monitor) const;

    /**
     * @~japanese
     * リンク交通流観測器 @p monitor の集計データ用出力ファイル名を戻す
     *
     * @~english
     * Return output file name for aggregated data of link traffic flow monitor
     * @p monitor
     */
    const std::string _aggregatedFilename(LinkFlowMonitor* monitor) const;
};

#endif //__LINK_FLOW_MONITOR_WRITER_HPP__
