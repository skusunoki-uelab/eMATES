#ifndef __CS_NODE_FAST_BUILDER_HPP__
#define __CS_NODE_FAST_BUILDER_HPP__

#include "IntersectionBuilder.hpp"

// 急速充電用CSノード生成のクラス [eMATES]

class CSNodeFastBuilder : public IntersectionBuilder
{
public:
    CSNodeFastBuilder(RoadMapBuilder* roadMapBuilder);
    virtual ~CSNodeFastBuilder();

    //==================================================================
    /**
     * @~japanese CSNodeFastクラスのインスタンスを生成して返す
     * @~english  Generate and return an instance of CSNodeFast class
     */
    virtual Intersection* build(const std::string& id,
                                const std::string& type,
                                RoadMap* roadMap) override;

};
#endif
