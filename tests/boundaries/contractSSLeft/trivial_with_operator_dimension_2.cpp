//@+leo-ver=5-thin
//@+node:gcross.20110307093706.3468: * @file trivial_with_operator_dimension_2.cpp
//@@language cplusplus
//@+<< License >>
//@+node:gcross.20110307093706.3469: ** << License >>
//@+at
// Copyright (c) 2011, Gregory Crosswhite
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// 
//     * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//@@c
//@-<< License >>
//@+<< Includes >>
//@+node:gcross.20110307093706.3470: ** << Includes >>
#include <boost/assign/list_of.hpp>
#include <boost/range/algorithm/equal.hpp>

#include "boundaries.hpp"

#include "test_utils.hpp"

using namespace Nutcracker;

using boost::assign::list_of;
using boost::equal;
//@-<< Includes >>
int main() {
    OverlapSite<Left> const overlap_site
        (RightDimension(1)
        ,LeftDimension(1)
        ,fillWithRange(list_of(5)(-1))
        );
    StateSite<Left> const state_site
        (LeftDimension(1)
        ,RightDimension(1)
        ,fillWithRange(list_of(1)(1))
        );
    OverlapBoundary<Left> const new_boundary(
        contractSSLeft(
             OverlapBoundary<Left>::trivial
            ,overlap_site
            ,state_site
        )
    );
    ASSERT_EQUAL_TO(OverlapDimension(1),new_boundary.overlapDimension());
    ASSERT_EQUAL_TO(StateDimension(1),new_boundary.stateDimension());
    ASSERT_TRUE(equal(list_of(complex<double>(4)),new_boundary));

    return 0;
}
//@-leo
