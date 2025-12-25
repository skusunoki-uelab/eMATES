/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
  * @file RoutingLink.cpp
  */
#include "RoutingLink.hpp"
#include "AppMates.hpp"
#include "CSNodeFast.hpp" // [eMATES]
#include "Intersection.hpp"
#include "LinkFlowRecord.hpp"
#include "RouterManager.hpp"
#include "RoutingNode.hpp"
#include "Section.hpp"
#include <algorithm>
#include <cassert>
#include <cfloat>
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <AmuConverter.hpp>

using namespace std;
using namespace amu::converter;

//==============================================================================
RoutingLink::RoutingLink(
    const Intersection* inter, const RoutingNode* begin, const RoutingNode* end)
    : _inter(inter), _beginNode(begin), _endNode(end)
{
    _rank  = 0;
    _subId = formatId("0", NUM_FIGURE_FOR_ROUTING_LINK);
    _id    = formatId(to_string(_rank), NUM_FIGURE_FOR_ROUTING_LAYER) + _subId;

    _upperLink = nullptr;
    _lowerNodes.clear();
    _lowerLinks.clear();
    _includedLowestNodes.clear();
    _abbreviatedNodes.clear();

    _length                  = 0.0;
    _includesStraightDriving = false;
    _includesLeftTurn        = false;
    _includesRightTurn       = false;
    _width                   = 0;

    for (unsigned int i = 0; i < VEHICLE_ROUTING_PARAMETER_SIZE; i++)
    {
        _costs[i] = 0.0;
    }

    // [eMATES] 2025/5/30 by abe CS経由コスト
    // CSNodeNormalはCS探索以外でも探索されることを考慮してコストを足さない。
    if (inter && (typeid(*inter) == typeid(CSNodeFast)))
    {
        _csCost = CS_ENTRY_PENALTY;
    }
}

//==============================================================================
void RoutingLink::setProperty()
{
    if (_rank == 0)
    {
        if (_inter)
        {
            // レイヤ間リンクには_interが設定されていない．
            // For inter-layer links, _inter is not set.
            _setPropertyDirectly();
        }
    }
    else
    {
        if (!(_lowerLinks.empty()))
        {
            // レイヤ間リンクには_lowerLinksが設定されていない．
            // For inter-layer links, _lowerLinks are not set.
            _setPropertyOfHigherRank();
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // _abbreviatedNodesの記録
        _setAbbreviatedNodes();
    }
}

//==============================================================================
void RoutingLink::_setPropertyDirectly()
{
    const Section* section = _beginNode->section();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * リンクの長さ [m]
     *   直前のRoutingNode（RoadMapにおけるSection）に入ってから
     *   このRoutingLink（RoadMapにおけるIntersection）を出るまでの距離
     *
     * Link length [m]
     *   Distance from entering the previous RoutingNode (i.e. Section
     *   in RoadMap) to exiting this RoutingLink (i.e. Intersection in
     *   RoadMap)
     */

    // 単路の長さ
    // Section length
    _length = section->length();

    // 交差点における inDir から outDir までの距離
    // Distance from inDir to outDir at intersection
    int inDir  = _inter->direction(section);
    int outDir = _inter->direction(_endNode->section());

    _length += _inter->length(inDir, outDir);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 直進・左折・右折
    // Driving straight, turning left, turning right
    if (_inter->relativeDirection(inDir, outDir) == RD::STRAIGHT)
    {
        _includesStraightDriving = true;
    }
    else if (_inter->relativeDirection(inDir, outDir) == RD::LEFT)
    {
        _includesLeftTurn = true;
    }
    else if (_inter->relativeDirection(inDir, outDir) == RD::RIGHT)
    {
        _includesRightTurn = true;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 道幅
    // Road width
    bool isUp = section->isUp(section->anotherIntersection(_inter), _inter);
    if (isUp)
    {
        _width = section->upWidth();
    }
    else
    {
        _width = section->downWidth();
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 通行権
    // Right-of-way
    /*
    for (auto itr : _inter->allowedVehicleTypes(inDir, outDir))
    {
        _restriction.addAllowedVehicleType(itr);
    }
    for (auto itr : _inter->deniedVehicleTypes(inDir, outDir))
    {
        _restriction.addDeniedVehicleType(itr);
    }
    */
    _restriction.addPermission(_inter->restriction(inDir, outDir));

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // ランク0のリンクに_probabilityは設定されない
    // No _probability are given to rank-0 links
}

//==============================================================================
void RoutingLink::_setPropertyOfHigherRank()
{
    // 下位のリンクの属性を集約する
    // Aggregate properties of lower-rank links
    for (auto itr : _lowerLinks)
    {
        _length += itr->length();
        _includesStraightDriving |= itr->includesStraightDriving();
        _includesLeftTurn |= itr->includesLeftTurn();
        _includesRightTurn |= itr->includesRightTurn();
        _width = (_width == 0 ? itr->width() : min(_width, itr->width()));
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 下位ノード・下位リンクの _restriction ，_probability ，および
     * 下位リンクの _includedLowestNodes を集約する．
     *
     * Aggregate _restriction and _probability of lower-rank links
     * /nodes, and _includedLowestNodes of lower-rank links.
     */
    for (auto itr : _lowerNodes)
    {
        _restriction.addPermission(itr->restriction());
        _probability.addProbability(itr->probability());
    }

    for (auto itr : _lowerLinks)
    {
        _restriction.addPermission(itr->restriction());
        _probability.addProbability(itr->probability());

        //--------------------------------------------------------------
        //_includedLowestNodes

        // 最初以外は下位リンクの始点を含む
        // Include the begin node of the lower-rank link except the first
        if (itr != *(_lowerLinks.begin()))
        {
            _includedLowestNodes.emplace_back(itr->beginNode()->lowestNode());
        }
        // 下位リンクが含んでいたノードを追加する
        // Include nodes included in the lower-rank link
        for (auto itr_n : itr->includedLowestNodes())
        {
            _includedLowestNodes.emplace_back(itr_n);
        }
    }
}

//==============================================================================
void RoutingLink::_setAbbreviatedNodes()
{
    if (!(_abbreviatedNodes.empty()))
    {
        return;
    }

    if (_lowerNodes.empty())
    {
        if (!_lowerLinks.empty())
        {
            const vector<const RoutingNode*>& nodes
                = const_cast<RoutingLink*>(_lowerLinks[0])->abbreviatedNodes();
            _abbreviatedNodes.insert(
                _abbreviatedNodes.end(), nodes.begin(), nodes.end());
        }
    }
    else
    {
        unsigned int i;
        for (i = 0; i <= _lowerNodes.size(); i++)
        {
            const vector<const RoutingNode*>& nodes
                = const_cast<RoutingLink*>(_lowerLinks[i])->abbreviatedNodes();
            _abbreviatedNodes.insert(
                _abbreviatedNodes.end(), nodes.begin(), nodes.end());

            // _lowerLinksは_lowerNodesより1つ多いことに注意．
            // Note that _lowerLinks is one more than _lowerNodes.
            if (i < _lowerNodes.size())
            {
                _abbreviatedNodes.emplace_back(_lowerNodes[i]);
            }
        }
    }
}

//==============================================================================
void RoutingLink::registerToBeUpdated() const
{
    // 最上位リンクのみを RouterManager に登録する
    // Register only the top link to RouterManager
    if (_upperLink)
    {
        _upperLink->registerToBeUpdated();
    }
    else
    {
        AppMates::getRouterManager().addUpdateRequiredLink(this);
    }
}

//==============================================================================
void RoutingLink::setInitialCosts()
{
    /*
     * コストは実際の値(右左折直進は0/1)を入力し，各Routerで重み付けする．
     * 配列で扱うため，すべてdouble型とする．現状では時間コストのみ
     * 動的コストとして定期的に更新する．時間コストは距離/最高速度を
     * 初期値とする．
     *
     * For the costs, enter the actual value (0/1 for straight/left-turn
     * /right-turn) and weigh them with each router. Since treated in an
     * array, they are all double type values. Currently, only the time
     * cost is updated at fixed intervals as a dynamic cost. The initial
     * value of the time cost is the distance divided by the maximum
     * speed.
     */
    double costs[VEHICLE_ROUTING_PARAMETER_SIZE];

    // 距離
    // Length
    costs[toUnderlying(RoutingParamIndex::DISTANCE)]
        = static_cast<double>(_length);

    // 時間
    // Time
    // 120[km/h] = 2000[m/min] = 100/3[m/sec]
    costs[toUnderlying(RoutingParamIndex::TIME)]
        = static_cast<double>(_length) * 3.0 / 100.0;

    // 直進
    // Going straight
    costs[toUnderlying(RoutingParamIndex::STRAIGHT)]
        = (_includesStraightDriving ? 1.0 : 0.0);

    // 左折
    // Left-turn
    costs[toUnderlying(RoutingParamIndex::LEFT_TURN)]
        = (_includesLeftTurn ? 1.0 : 0.0);

    // 右折
    // Right-turn
    costs[toUnderlying(RoutingParamIndex::RIGHT_TURN)]
        = (_includesRightTurn ? 1.0 : 0.0);

    // [eMATES] CS関連コストはゼロ
    costs[toUnderlying(RoutingParamIndex::CS_TIME)] = 0;
    costs[toUnderlying(RoutingParamIndex::CS_YEN)] = 0;

    //--------------------------------------------------------------
    for (unsigned int i = 0; i < VEHICLE_ROUTING_PARAMETER_SIZE; i++)
    {
        _costs[i] = costs[i];
    }
}

//==============================================================================
void RoutingLink::renewDynamicCosts()
{
    /*
     * UpwardリンクおよびDownwardリンクは仮想リンクであるので更新しない
     * Do not update upward and downward links as they are virtual links
     */
    if (_linkType != RoutingLinkType::FLAT)
    {
        return;
    }

    double time = 0.0;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if (_rank == 0)
    {
        const Section* section = _beginNode->section();

        if (!_inter)
        {
            cerr << "ERROR: RoutingLink[" << _id << "] has no _inter." << _id
                 << endl;
            exit(EXIT_FAILURE);
        }

        // 交差点内の転回方向
        // Turning direction at the intersection
        int inDir  = _inter->direction(section);
        int outDir = _inter->direction(_endNode->section());
        int diff   = (outDir - inDir + _inter->numNexts()) % _inter->numNexts();

        // 通過時間
        // Link travel time
        //time += _inter->averageTransitTimeForGlobalRouting(inDir, outDir);
        if (_inter->numNexts() == 1)
        {
            time = _inter->linkFlowRecord(0)->estimatedTravelTime(0);
        }
        else
        {
            time = _inter->linkFlowRecord(inDir)->estimatedTravelTime(diff);
        }
        time /= 1000.0; //[ms]->[s]
    }
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    else
    {
        // 先に下位リンクのコストを更新する
        // Update the cost of lower links first
        assert(_lowerLinks.size());

        for (auto itr : _lowerLinks)
        {
            const_cast<RoutingLink*>(itr)->renewDynamicCosts();
            time += itr->cost(toUnderlying(RoutingParamIndex::TIME));
        }
    }

    _costs[toUnderlying(RoutingParamIndex::TIME)] = time;
}

//==============================================================================
void RoutingLink::print(ostream& out) const
{
    out << "RoutingLink ID: " << _id
        << ", Intersectioin: " << (_inter ? _inter->id() : "none") << endl;

    out << "  LinkType: ";
    switch (_linkType)
    {
    case RoutingLinkType::FLAT:
        out << "flat";
        break;
    case RoutingLinkType::UPWARD:
        out << "upward";
        break;
    case RoutingLinkType::DOWNWARD:
        out << "downward";
        break;
    default:
        out << "undefined";
        break;
    }
    out << endl;

    if (!_lowerLinks.empty())
    {
        out << "  LowerLinks: ";
        for (auto itr : _lowerLinks)
        {
            out << itr->id() << ", ";
        }
        out << endl;
    }
    if (!_lowerNodes.empty())
    {
        out << "  LowerNodes: ";
        for (auto itr : _lowerNodes)
        {
            out << itr->id() << ", ";
        }
        out << endl;
    }
    if (!_includedLowestNodes.empty())
    {
        out << "  IncludedLowestNodes: ";
        for (auto itr : _includedLowestNodes)
        {
            out << itr->id() << ", ";
        }
        out << endl;
    }
    out << "  Costs: ";
    for (auto itr : _costs)
    {
        cout << itr << ", ";
    }
    cout << endl;
    _restriction.print(cout);
    _probability.print(cout);
}
