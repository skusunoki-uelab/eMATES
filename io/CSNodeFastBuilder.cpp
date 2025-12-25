#include "CSNodeFastBuilder.hpp"
#include "../AppMates.hpp"
#include "../CSNodeFast.hpp"

//======================================================================
CSNodeFastBuilder::CSNodeFastBuilder(RoadMapBuilder* roadMapBuilder) : IntersectionBuilder(roadMapBuilder)
{
    _inter = nullptr;
    _rdTable = nullptr;
    _connectorIds.clear();
    _roadwayVertexes.clear();
    _borderPoints.clear();

    // デフォルト値であり上書きされうる
    // Default values, may be overwritten.
    _sidewalkWidth
        = AppMates::getGVManager()
        .getNumeric("DEFAULT_SIDEWALK_WIDTH");
    _crosswalkWidth
        = AppMates::getGVManager()
        .getNumeric("DEFAULT_CROSSWALK_WIDTH");

#ifdef INCLUDE_TRAMS
     _builderTramExt = nullptr;
#endif //INCLUDE_TRAMS
}

//======================================================================
CSNodeFastBuilder::~CSNodeFastBuilder()
{
#ifdef INCLUDE_TRAMS
    delete _builderTramExt;
#endif //INCLUDE_TRAMS
}

//======================================================================
Intersection* CSNodeFastBuilder::build(
    const std::string& fmId, const std::string& type, RoadMap* roadMap)
{
    _inter  = new CSNodeFast(fmId, type, roadMap);
    _bundle = _inter;

#ifdef INCLUDE_TRAMS
    _builderTramExt = new IntersectionBuilderTramExt(_inter);
#endif //INCLUDE_TRAMS

    return _inter;
}
