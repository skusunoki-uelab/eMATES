/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file SignalBuilder.hpp
 */
#ifndef __SIGNAL_BUILDER_HPP__
#define __SIGNAL_BUILDER_HPP__
#include "../RoadMap.hpp"
#include "../Signal.hpp"
#include "../SignalAspect.hpp"
#include <string>
#include <vector>

class Intersection;

//##############################################################################
/**
 * @~japanese
 * 信号制御データをファイルから入力し信号を生成する
 *
 * @~english
 * Generate traffic light by inputting control data from files
 *
 * @~
 * @ingroup Initialization IO Signal
 */
class SignalBuilder
{
public:
    SignalBuilder()
    {
        _unsignalizedIntersections.clear();
    };
    ~SignalBuilder() {};

    /**
     * @~japanese 信号を生成する
     * @~english  Generate traffic lights
     */
    bool buildSignals(RoadMap* roadMap);

private:
    /**
     * @~japanese ファイル読み無信号交差点のリストを作成する
     * @~english  Read file and create list of unsignalized intersections
     */
    void _readUnsignalizedIntersectionFile(RoadMap* _roadMap);

    /**
     * @~japanese 交差点 @p inter に設置される信号機 @p signal の属性を設定する
     * @~english  Set property of @p signal placed on intersection @p inter
     */
    Signal* _buildSignal(const Intersection* inter);

    /**
     * @~japanese 信号機 @p signal のサイクルを設定する
     * @~english  Set cycle of @p signal
     */
    void _setCycles(Signal* signal) const;

    /**
     * @~japanese 信号機 @p signal の現示を設定する
     * @~english  Set aspects of @p signal
     */
    void _setAspectSet(Signal* signal) const;

    /**
     * @~japanese
     * 識別番号 @p id の交差点あるいは信号に対するサイクル定義ファイルの名前を
     * 戻す
     * 
     * @~english
     * Return the name of the cycle definition file for the intersection or
     * signal with ID number @p id
     */
    std::string _cycleFileName(const std::string& id) const;

    /**
     * @~japanese デフォルトのサイクル定義ファイルの名前を戻す
     * @~english  Return the name of the default cycle definition file
     */
    std::string _defaultCycleFileName() const;

    /**
     * @~japanese
     * 識別番号 @p id の交差点あるいは信号に対する現示パターン定義ファイルの
     * 名前を戻す
     * 
     * @~english
     * Returns the name of aspect pattern definition file for intersection or
     * signal with ID number @p id
     */
    std::string _aspectFileName(const std::string& id) const;

    /**
     * @~japanese デフォルトの現示パターン定義ファイルの名前を戻す
     * @param numSides 交差点の次数 (隣接交差点数)
     * 
     * @~english  Return the name of the default aspect pattern definition file
     * @param numSides Intersection degree (number of adjacent intersections)
     */
    std::string _defaultAspectFileName(int numSides) const;

private:
    /**
     * @~japanese 無信号交差点のリスト
     * 
     * _readUnsignalizedIntersectionFile で作成し，その後 RoadMap に登録する
     * 
     * @~english  List of unsignalized intersections
     * 
     * Created by _readUnsignalizedIntersectionFile, and intersections in the 
     * list will be registered with RoadMap later.
     */
    std::vector<Intersection*> _unsignalizedIntersections;
};

#endif //__SIGNAL_BUILDER_H__
