#ifndef __CS_NODE_NORMAL_BUILDER_HPP__
#define __CS_NODE_NORMAL_BUILDER_HPP__

#include "ODNodeBuilder.hpp"

// 急速充電用CSノード生成のクラス [eMATES]

class CSNodeNormalBuilder : public ODNodeBuilder
{
public:
    CSNodeNormalBuilder(RoadMapBuilder* roadMapBuilder);
    virtual ~CSNodeNormalBuilder(){};

    //==================================================================
    /**
     * @~japanese CSNodeNormalクラスのインスタンスを生成して返す
     * @~english  Generate and return an instance of CSNodeNormal class
     */
    virtual Intersection* build(const std::string& id,
                                const std::string& type,
                                RoadMap* roadMap) override;

};
#endif
