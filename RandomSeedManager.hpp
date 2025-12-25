/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file RandomSeedManager.hpp
 */
#ifndef __RANDOM_SEED_MANAGER_HPP__
#define __RANDOM_SEED_MANAGER_HPP__
#include "ManagerBase.hpp"
#include <climits>
#include <iostream>
#include <typeinfo>

//##############################################################################
/**
 * @~japanese 乱数の種を管理する
 * @~english  Manager random number seed
 *
 * @~
 * @ingroup Random Manager
 * @see     RandomNumberGenerator
 */
class RandomSeedManager : public ManagerBase
{
    friend class ManagerPool;

private:
    RandomSeedManager()
    {
        _numAssigned       = 0;
        _numAssignedForSim = 0;
        _className         = typeid(this).name();
    }
    ~RandomSeedManager() {};

    /**
     * @~japanese 親クラスの関数のオーバーライド
     * @~english  Override functions of parent class
     */
    void _finalizeInside() override {}

public:
    /**
     * @~japanese 乱数生成器のシードを戻す
     * @~english  Returns a seed for a random number generator
     */
    unsigned int seed()
    {
        unsigned int s = (_globalSeed + _numAssigned) % UINT_MAX;
        _numAssigned++;
        return s;
    }

    /**
     * @~japanese advmates-sim用乱数生成器のシードを戻す
     * @~english  Returns a seed for a random number generator for advmates-sim
     */
    unsigned int seedForSim()
    {
        unsigned int s = (_globalSeed + _numAssignedForSim) % UINT_MAX;
        _numAssignedForSim++;
        return s;
    }

    //==========================================================================
private:
    /**
     * @~japanese グローバルシード
     * @~english  Global seed
     */
    unsigned int _globalSeed;

    /**
     * @~japanese 生成した乱数生成器の数
     * @~english  Number of assigned random number generator
     */
    unsigned int _numAssigned;

    /**
     * @~japanese
     * 生成したadvmates-sim用乱数生成器の数
     *
     * @~english
     * Number of assigned random number generator for advmates-sim
     */
    unsigned int _numAssignedForSim;

public:
    //==========================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
    void setGlobalSeed(unsigned int seed)
    {
        _globalSeed = seed;
    }
    unsigned int globalSeed() const
    {
        return _globalSeed;
    }

    ///@}
};

#endif //__RANDOM_SEED_MANAGER_HPP__
