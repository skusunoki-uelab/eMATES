/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file Clocker.hpp
 */
#ifndef __CLOCKER_HPP__
#define __CLOCKER_HPP__
#include <ctime>
#include <string>
#ifdef _OPENMP
#include <omp.h>
#else //_OPENMP not defined
#include <chrono>
#endif //_OPENMP
#include <iostream>

//##############################################################################
/**
 * @~japanese 指定した処理に要した時間を計測するタイマー
 * @~english  Timer that measures the time required for specified processing
 * @~ @ingroup Monitoring Time
 */
class Clocker
{
public:
    explicit Clocker(const std::string& name)
        : _name(name),
          _beginCpuTime(0.0),
          _beginWallclockTime(0.0),
          _totalCpuTime(0.0),
          _totalWallclockTime(0.0),
          _isClocking(false),
          _numCalled(0)
    {
    }
    ~Clocker() {}

    //==========================================================================
    /**
     * @~japanese 計時を開始する
     * @return 正常に計時を開始したかどうか
     *
     * @~english  Start measuring time
     * @return Whether measurement stopped normally
     */
    bool startClock()
    {
        if (_isClocking)
        {
            return false;
        }
        _beginCpuTime = clock();
#ifdef _OPENMP
        _beginWallclockTime = omp_get_wtime();
#else  //_OPENMP not defined
        _beginWallclockTime = std::chrono::system_clock::now();
#endif //_OPENMP

        _isClocking = true;
        _numCalled++;

        return true;
    }

    /**
     * @~japanese 計時を終了する
     * @return 正常に計時を終了したかどうか
     *
     * @~english  Stop measuring time
     * @return Whether measurement stopped normally
     */
    bool stopClock()
    {
        if (!_isClocking)
        {
            return false;
        }
        clock_t endCpuTime = clock();

#ifdef _OPENMP
        double endWallclockTime = omp_get_wtime();
#else  //_OPENMP not defined
        std::chrono::system_clock::time_point endWallclockTime
            = std::chrono::system_clock::now();
#endif //_OPENMP

        _totalCpuTime
            += static_cast<double>(endCpuTime - _beginCpuTime) / CLOCKS_PER_SEC;

#ifdef _OPENMP
        _totalWallclockTime += endWallclockTime - _beginWallclockTime;
#else  //_OPENMP not defined
        _totalWallclockTime
            += std::chrono::duration_cast<std::chrono::microseconds>(
                   endWallclockTime - _beginWallclockTime)
                   .count()
            * 1e-6;
#endif //_OPENMP

        _isClocking = false;

        return true;
    }

    //==========================================================================
    /**
     * @~japanese 計時結果を表示する
     * @~english  Display measured result
     */
    void print() const;

    //==========================================================================
private:
    /**
     * @~japanese タイマーの名称
     * @~english  Timer name
     */
    const std::string _name;

    /**
     * @~japanese CPU timeの計測開始時刻
     * @note clock()関数を使用
     * 
     * @~english  Start and stop of CPU time measurement
     * @note Use the clock() function
     */
    clock_t _beginCpuTime;

    //==========================================================================
    /**
     * @~japanese  wall-clock timeの計測開始時刻
     *
     * @note
     * OpenMP有効時にはomp_get_wtime()を，無効時にはstd::chrono::now()を使用
     *
     * @~english  Start time of wall-clock time measurement
     *
     * @note
     * Use omp_get_wtime() when OpenMP is enabled, std::chrono::now() when it
     * is disabled
     */
#ifdef _OPENMP
    double _beginWallclockTime;
#else  //_OPENMP not defined
    std::chrono::system_clock::time_point _beginWallclockTime;
#endif //_OPENMP

    /**
     * @~japanese CPU timeの合計 
     * @~english  Total CPU time
     */
    double _totalCpuTime;

    /**
     * @~japanese Wall-clock time 
     * @~english  Total wall-clock time
     */
    double _totalWallclockTime;

    /**
     * @~japanese 現在計時中かどうか
     * @~english  Whether the timer is currently running
     */
    bool _isClocking;

    /**
     * @~japanese 呼び出し回数
     * @~english  Number of calls
     */
    unsigned long _numCalled;

    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    const std::string& name() const
    {
        return _name;
    }

    double totalCpuTime() const
    {
        return _totalCpuTime;
    }

    double totalWallclockTime() const
    {
        return _totalWallclockTime;
    }

    unsigned long numCalled() const
    {
        return _numCalled;
    }

    ///@}
};

#endif //__CLOCKER_HPP__
