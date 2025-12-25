/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file SignalTimeSeriesWriter.hpp
 */
#ifndef __SIGNAL_TIME_SERIES_WRITER_HPP__
#define __SIGNAL_TIME_SERIES_WRITER_HPP__
#include "../Config.hpp"
#include "../RoadMap.hpp"
#include "../Signal.hpp"
#include <fstream>
#include <string>
#include <unordered_map>

#ifdef USE_ZLIB
#include <zlib.h>
#else //USE_ZLIB
#include <cstdio>
#endif //USE_ZLIB

//######################################################################
/**
 * @~japanese
 * 信号機の時系列データをファイルに書き込む
 *
 * @~english
 * Write traffic light historical data to file
 *
 * @~
 * @ingroup IO Signal
 */
class SignalTimeSeriesWriter
{
public:
    SignalTimeSeriesWriter() {};
    ~SignalTimeSeriesWriter() {};

    /**
     * @~japanese 信号機の時系列データをファイルに書き込む
     * @~english  Write traffic light time series data to file
     */
    bool writeSignalsData(const RoadMap* roadMap);

private:
    /**
     * @~japanese 出力ファイル名を決定する
     * @~english  Decide output file name
     */
    std::vector<std::string> _decideFileName(ulint time);

#ifdef USE_ZLIB
    using FILEOUT = gzFile;
#else  //USE_ZLIB
    using FILEOUT = FILE*;
#endif //USE_ZLIB

    /**
     * @~japanese writeSignalsData()の実体
     * @~english  Substance of writeSignalsData()
     */
    bool _writeSignalData(
        FILEOUT fout, const RoadMap* roadMap, Signal* signal);
};

#endif //__SIGNAL_TIME_SERIES_WRITER_HPP__
