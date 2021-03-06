/*!
\file boundaries.cpp
\brief Boundary-related functions
*/

#include "nutcracker/boundaries.hpp"
#include "nutcracker/core.hpp"

namespace Nutcracker {

complex<double> computeExpectationValueAtSite( // {{{
      ExpectationBoundary<Left> const& left_boundary
    , StateSite<Middle> const& state_site
    , OperatorSite const& operator_site
    , ExpectationBoundary<Right> const& right_boundary
) {
    return Core::compute_expectation(
         left_boundary | state_site
        ,state_site | right_boundary
        ,left_boundary | operator_site
        ,operator_site | right_boundary
        ,operator_site | state_site
        ,left_boundary
        ,state_site
        ,operator_site.numberOfMatrices(),operator_site,operator_site
        ,right_boundary
    );
} // }}}

MatrixConstPtr computeOptimizationMatrix( // {{{
      ExpectationBoundary<Left> const& left_boundary
    , OperatorSite const& operator_site
    , ExpectationBoundary<Right> const& right_boundary
) {
    size_t const dimension = operator_site.physicalDimension() * left_boundary.stateDimension() * right_boundary.stateDimension();
    MatrixPtr matrix(new Matrix(dimension,dimension));
    Core::compute_optimization_matrix(
         left_boundary.stateDimension()
        ,right_boundary.stateDimension()
        ,left_boundary | operator_site
        ,operator_site | right_boundary
        ,operator_site.physicalDimension()
        ,left_boundary
        ,operator_site.numberOfMatrices(),operator_site,operator_site
        ,right_boundary
        ,matrix->data().begin()
    );
    return matrix;
} // }}}

complex<double> contractExpectationBoundaries( // {{{
      ExpectationBoundary<Left> const& left_boundary
    , ExpectationBoundary<Right> const& right_boundary
) {
    return Core::contract_expectation_boundaries(
         connectDimensions(
             "left boundary state"
            ,left_boundary.stateDimension()
            ,"right boundary state"
            ,right_boundary.stateDimension()
         )
        ,connectDimensions(
             "left boundary operator"
            ,left_boundary.operatorDimension()
            ,"right boundary operator"
            ,right_boundary.operatorDimension()
         )
        ,left_boundary
        ,right_boundary
    );
} // }}}

ExpectationBoundary<Left> contractSOSLeft( // {{{
      ExpectationBoundary<Left> const& old_boundary
    , StateSite<Left> const& state_site
    , OperatorSite const& operator_site
) {
    return Unsafe::contractSOSLeft(old_boundary,state_site,operator_site);
} // }}}

ExpectationBoundary<Right> contractSOSRight( // {{{
      ExpectationBoundary<Right> const& old_boundary
    , StateSite<Right> const& state_site
    , OperatorSite const& operator_site
) {
    return Unsafe::contractSOSRight(old_boundary,state_site,operator_site);
} // }}}

OverlapBoundary<Left> contractVSLeft( // {{{
      OverlapBoundary<Left> const& old_boundary
    , OverlapSite<Left> const& overlap_site
    , StateSite<Left> const& state_site
) {
    return Unsafe::contractVSLeft(old_boundary,overlap_site,state_site);
} // }}}

OverlapBoundary<Right> contractVSRight( // {{{
      OverlapBoundary<Right> const& old_boundary
    , OverlapSite<Right> const& overlap_site
    , StateSite<Right> const& state_site
) {
    OverlapBoundary<Right> new_boundary
        (OverlapDimension(overlap_site.leftDimension())
        ,StateDimension(state_site.leftDimension())
        );
    Core::contract_vs_right(
         overlap_site.leftDimension()
        ,overlap_site | old_boundary
        ,state_site.leftDimension()
        ,state_site | old_boundary
        ,overlap_site | state_site
        ,old_boundary
        ,overlap_site
        ,state_site
        ,new_boundary
    );
    return boost::move(new_boundary);
} // }}}

namespace Unsafe { // {{{

OverlapBoundary<Left> contractVSLeft( // {{{
      OverlapBoundary<Left> const& old_boundary
    , OverlapSiteAny const& overlap_site
    , StateSiteAny const& state_site
) {
    OverlapBoundary<Left> new_boundary
        (OverlapDimension(overlap_site.rightDimension())
        ,StateDimension(state_site.rightDimension())
        );
    Core::contract_vs_left(
         old_boundary | overlap_site
        ,overlap_site.rightDimension()
        ,old_boundary | state_site
        ,state_site.rightDimension()
        ,overlap_site | state_site
        ,old_boundary
        ,overlap_site
        ,state_site
        ,new_boundary
    );
    return boost::move(new_boundary);
} // }}}

ExpectationBoundary<Left> contractSOSLeft( // {{{
      ExpectationBoundary<Left> const& old_boundary
    , StateSiteAny const& state_site
    , OperatorSite const& operator_site
) {
    ExpectationBoundary<Left> new_boundary
        (OperatorDimension(operator_site.rightDimension())
        ,StateDimension(state_site.rightDimension())
        );
    Core::contract_sos_left(
         old_boundary | state_site
        ,state_site.rightDimension()
        ,old_boundary | operator_site
        ,operator_site.rightDimension()
        ,operator_site | state_site
        ,old_boundary
        ,operator_site.numberOfMatrices(),operator_site,operator_site
        ,state_site
        ,new_boundary
    );
    return boost::move(new_boundary);
} // }}}

ExpectationBoundary<Right> contractSOSRight( // {{{
      ExpectationBoundary<Right> const& old_boundary
    , StateSiteAny const& state_site
    , OperatorSite const& operator_site
) {
    ExpectationBoundary<Right> new_boundary
        (OperatorDimension(operator_site.leftDimension())
        ,StateDimension(state_site.leftDimension())
        );
    Core::contract_sos_right(
         state_site.leftDimension()
        ,state_site | old_boundary
        ,operator_site.leftDimension()
        ,operator_site | old_boundary
        ,operator_site | state_site
        ,old_boundary
        ,operator_site.numberOfMatrices(),operator_site,operator_site
        ,state_site
        ,new_boundary
    );
    return boost::move(new_boundary);
} // }}}

} // }}}

}
