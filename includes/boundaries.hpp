//@+leo-ver=5-thin
//@+node:gcross.20110214183844.1813: * @file boundaries.hpp
//@@language cplusplus

//@+<< License >>
//@+node:gcross.20110220182654.2006: ** << License >>
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

#ifndef NUTCRACKER_BOUNDARIES_HPP
#define NUTCRACKER_BOUNDARIES_HPP

//@+<< Includes >>
//@+node:gcross.20110214155808.1853: ** << Includes >>
#include "tensors.hpp"
//@-<< Includes >>

namespace Nutcracker {

//@+<< Usings >>
//@+node:gcross.20110214155808.1854: ** << Usings >>
//@-<< Usings >>

//@+others
//@+node:gcross.20110214155808.1856: ** Functions
complex<double> computeExpectationValueAtSite(
      ExpectationBoundary<Left> const& left_boundary
    , StateSite<Middle> const& state_site
    , OperatorSite const& operator_site
    , ExpectationBoundary<Right> const& right_boundary
);

complex<double> contractExpectationBoundaries(
      ExpectationBoundary<Left> const& left_boundary
    , ExpectationBoundary<Right> const& right_boundary
);

ExpectationBoundary<Left> contractSOSLeft(
      ExpectationBoundary<Left> const& old_boundary
    , StateSite<Left> const& state_site
    , OperatorSite const& operator_site
);

ExpectationBoundary<Right> contractSOSRight(
      ExpectationBoundary<Right> const& old_boundary
    , StateSite<Right> const& state_site
    , OperatorSite const& operator_site
);

OverlapBoundary<Left> contractSSLeft(
      OverlapBoundary<Left> const& old_boundary
    , OverlapSite<Left> const& overlap_site
    , StateSite<Left> const& state_site
);

OverlapBoundary<Right> contractSSRight(
      OverlapBoundary<Right> const& old_boundary
    , OverlapSite<Right> const& overlap_site
    , StateSite<Right> const& state_site
);
//@+node:gcross.20110215235924.2012: *3* Unsafe
namespace Unsafe {

ExpectationBoundary<Left> contractSOSLeft(
      ExpectationBoundary<Left> const& old_boundary
    , StateSiteAny const& state_site
    , OperatorSite const& operator_site
);

OverlapBoundary<Left> contractSSLeft(
      OverlapBoundary<Left> const& old_boundary
    , OverlapSiteAny const& overlap_site
    , StateSiteAny const& state_site
);

}
//@+node:gcross.20110214155808.1858: ** struct contract
template<typename side> struct contract {};

template<> struct contract<Left> {
    static ExpectationBoundary<Left> SOS(
          ExpectationBoundary<Left> const& old_boundary
        , StateSite<Left> const& state_site
        , OperatorSite const& operator_site
    ) { return contractSOSLeft(old_boundary,state_site,operator_site); }

    static OverlapBoundary<Left> SS(
          OverlapBoundary<Left> const& old_boundary
        , OverlapSite<Left> const& overlap_site
        , StateSite<Left> const& state_site
    ) { return contractSSLeft(old_boundary,overlap_site,state_site); }
};

template<> struct contract<Right> {
    static ExpectationBoundary<Right> SOS(
          ExpectationBoundary<Right> const& old_boundary
        , StateSite<Right> const& state_site
        , OperatorSite const& operator_site
    ) { return contractSOSRight(old_boundary,state_site,operator_site); }

    static OverlapBoundary<Right> SS(
          OverlapBoundary<Right> const& old_boundary
        , OverlapSite<Right> const& overlap_site
        , StateSite<Right> const& state_site
    ) { return contractSSRight(old_boundary,overlap_site,state_site); }
};
//@-others

}

#endif
//@-leo
