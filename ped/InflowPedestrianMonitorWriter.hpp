/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file InflowPedestrianMonitorWriter.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __INFLOW_PEDESTRIAN_MONITOR_WRITER_HPP__
#define __INFLOW_PEDESTRIAN_MONITOR_WRITER_HPP__
#include <string>

class InflowPedestrianMonitor;

//##############################################################################
/**
 * @~japanese 流入歩行者の記録をファイル出力する
 * @~english  Output inflow pedestrian record to file 
 * @~ @ingroup IO Monitoring PedSim
 */
class InflowPedestrianMonitorWriter
{
public:
    InflowPedestrianMonitorWriter() {};
    ~InflowPedestrianMonitorWriter() {};

    /**
     * @~japanese
     * 発生歩行者検出器 @p monitor に関する出力ファイルを準備する
     *
     * @~english
     * Prepare output file for inflow pedestrian monitor @p monitor
     */
    void prepareFile(InflowPedestrianMonitor* monitor);

    /**
     * @~japanese
     * 流入車両検出器 @p monitor の記録をファイル出力する
     *
     * @~english
     * Output record of inflow pedestrian monitor @p monitor to file
     */
    void writeRecord(InflowPedestrianMonitor* monitor);

private:
    /**
     * @~japanese
     * 発生歩行者検出器 @p monitor の出力ファイル名を戻す
     *
     * @~english
     * Return output file name of inflow pedestrian monitor @p monitor
     */
    const std::string _filename(InflowPedestrianMonitor* monitor) const;
};

#endif //__INFLOW_MONITOR_WRITER_HPP__
#endif //INCLUDE_PEDESTRIANS
