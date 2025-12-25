/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file SubSection.hpp
 */
#ifndef __SUB_SECTION_HPP__
#define __SUB_SECTION_HPP__
#include "SubLaneBundle.hpp"

//######################################################################
/**
 * @~japanese 単路部サブセクション
 *
 * 単路部を細分化するためのクラス．SubLaneBundleを継承している．
 *
 * @~english  Subsection in section
 *
 * A class for subdividing an section. Inherits SubLaneBundle.
 *
 * @~ @ingroup RoadNetwork 
 */
class SubSection : public SubLaneBundle
{
public:
    SubSection(const std::string& id, SubsectionType type);
    virtual ~SubSection() {};
};

#endif //__SUB_SECTION_HPP__
