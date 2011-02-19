//@+leo-ver=5-thin
//@+node:gcross.20110130170743.1674: * @thin chain.cpp
//@@language cplusplus

//@+<< Includes >>
//@+node:gcross.20110130170743.1675: ** << Includes >>
#include <boost/assign.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/algorithm/reverse_copy.hpp>
#include <boost/range/irange.hpp>
#include <iterator>
#include <limits>
#include <utility>

#include "boundaries.hpp"
#include "chain.hpp"
#include "core.hpp"
#include "optimizer.hpp"
#include "utilities.hpp"
//@-<< Includes >>

namespace Nutcracker {

//@+<< Usings >>
//@+node:gcross.20110130170743.1676: ** << Usings >>
using boost::adaptors::reversed;

using std::abs;
using std::numeric_limits;
//@-<< Usings >>

//@+others
//@+node:gcross.20110208151104.1790: ** Values
Chain::Options const Chain::defaults =
    { 10000
    , 1e-12
    , 1e-12
    , 1e-12
    , lambda::_1+1
    };
//@+node:gcross.20110202175920.1714: ** class Chain
//@+node:gcross.20110202175920.1715: *3* (constructors)
Chain::Chain(
      Operator const& operator_sites
    , unsigned int const bandwidth_dimension
    , Options const& options
) : number_of_sites(operator_sites.size())
  , operator_sites(operator_sites)
  , current_site_number(0)
  , left_expectation_boundary(make_trivial)
  , right_expectation_boundary(make_trivial)
  , energy(0)
  , physical_dimensions(extractPhysicalDimensions(operator_sites))
  , maximum_bandwidth_dimension(maximumBandwidthDimension(physical_dimensions))
  , bandwidth_dimension(bandwidth_dimension)
  , options(options)
{
    assert(number_of_sites > 0);

    reset(bandwidth_dimension);
}
//@+node:gcross.20110202175920.1720: *3* computeExpectationValueAtSite
complex<double> Chain::computeExpectationValueAtCurrentSite() const {
    return 
        Nutcracker::computeExpectationValueAtSite(
             left_expectation_boundary
            ,state_site
            ,*operator_sites[current_site_number]
            ,right_expectation_boundary
        );
}
//@+node:gcross.20110202175920.1721: *3* computeStateNorm
double Chain::computeStateNorm() const {
    return state_site.norm();
}
//@+node:gcross.20110207120702.1784: *3* increaseBandwidthDimension
void Chain::increaseBandwidthDimension(unsigned int const new_bandwidth_dimension) {
    if(bandwidth_dimension == new_bandwidth_dimension) return;
    assert(bandwidth_dimension < new_bandwidth_dimension);
    assert(new_bandwidth_dimension <= maximum_bandwidth_dimension);
    assert(current_site_number == 0);
    vector<unsigned int> initial_bandwidth_dimensions = computeBandwidthDimensionSequence(new_bandwidth_dimension,physical_dimensions);
    vector<unsigned int>::const_reverse_iterator dimension_iterator = initial_bandwidth_dimensions.rbegin()+1;

    vector<Neighbor<Right> > old_right_neighbors(boost::move(right_neighbors));

    StateSite<Middle> first_state_site(boost::move(state_site));

    resetBoundaries();

    unsigned int operator_number = number_of_sites-1;
    for(vector<Neighbor<Right> >::iterator neighbor_iterator = old_right_neighbors.begin()
       ;neighbor_iterator != old_right_neighbors.end()
       ;++neighbor_iterator,--operator_number
    ) {
        if(operator_number > 1) {
            StateSite<Right>& next_right_state_site = (neighbor_iterator+1)->state_site;
            IncreaseDimensionBetweenResult<Right,Right> result(
                increaseDimensionBetweenRightRight(
                     *(dimension_iterator++)
                    ,next_right_state_site
                    ,neighbor_iterator->state_site
                )
            );
            next_right_state_site = boost::move(result.state_site_1);
            absorb<Right>(boost::move(result.state_site_2),operator_number);
        } else {
            IncreaseDimensionBetweenResult<Middle,Right> result(
                increaseDimensionBetweenMiddleRight(
                     *(dimension_iterator++)
                    ,first_state_site
                    ,neighbor_iterator->state_site
                )
            );
            state_site = boost::move(result.state_site_1);
            absorb<Right>(boost::move(result.state_site_2),operator_number);
        }
    }
    bandwidth_dimension = new_bandwidth_dimension;
}
//@+node:gcross.20110215235924.1878: *3* makeCopyOfState
State Chain::makeCopyOfState() const {
    assert(current_site_number == 0);
    StateSite<Middle> first_state_site(copyFrom<StateSite<Middle> const>(state_site));
    vector<StateSite<Right> > rest_state_sites; rest_state_sites.reserve(number_of_sites-1);
    BOOST_FOREACH(Neighbor<Right> const& neighbor, right_neighbors | reversed) {
        rest_state_sites.emplace_back(copyFrom<StateSite<Right> const>(neighbor.state_site));
    }
    return State(
         boost::move(first_state_site)
        ,boost::move(rest_state_sites)
    );
}
//@+node:gcross.20110218083552.2522: *3* moveTo
void Chain::moveTo(unsigned int new_site_number) {
    assert(new_site_number < number_of_sites);
    while(current_site_number > new_site_number) {
        move<Left>();
    }
    while(current_site_number < new_site_number) {
        move<Right>();
    }
}
//@+node:gcross.20110208230325.1790: *3* optimizeChain
void Chain::optimizeChain() {
    double previous_energy = energy;
    sweepUntilConverged();
    while(outsideTolerance(previous_energy,energy,options.chain_convergence_threshold)
       && bandwidth_dimension < maximum_bandwidth_dimension
    ) {
        previous_energy = energy;
        increaseBandwidthDimension(options.computeNewBandwidthDimension(bandwidth_dimension));
        sweepUntilConverged();
    }
    signalChainOptimized();
}
//@+node:gcross.20110206130502.1754: *3* optimizeSite
void Chain::optimizeSite() {
    try {
        OptimizerResult result(
            optimizeStateSite(
                 left_expectation_boundary
                ,state_site
                ,*operator_sites[current_site_number]
                ,right_expectation_boundary
                ,projector_matrix
                ,options.site_convergence_threshold
                ,options.maximum_number_of_iterations
            )
        );
        if(result.eigenvalue > energy && outsideTolerance(result.eigenvalue,energy,options.site_convergence_threshold)) {
            throw OptimizerObtainedGreaterEigenvalue(energy,result.eigenvalue);
        }
        energy = result.eigenvalue;
        state_site = boost::move(result.state_site);
        signalOptimizeSiteSuccess(result.number_of_iterations);
    } catch(OptimizerFailure& failure) {
        signalOptimizeSiteFailure(failure);
    }
}
//@+node:gcross.20110206130502.1759: *3* performOptimizationSweep
void Chain::performOptimizationSweep() {
    unsigned int const starting_site = current_site_number;
    optimizeSite();
    while(current_site_number+1 < number_of_sites) {
        move<Right>();
        optimizeSite();
    }
    while(current_site_number > 0) {
        move<Left>();
        optimizeSite();
    }
    while(current_site_number < starting_site) {
        move<Right>();
        optimizeSite();
    }
    signalSweepPerformed();
}
//@+node:gcross.20110208233325.1798: *3* reset
void Chain::reset(unsigned int bandwidth_dimension) {
    current_site_number = 0;

    resetBoundaries();

    right_neighbors.clear();
    right_neighbors.reserve(number_of_sites-1);

    vector<unsigned int> initial_bandwidth_dimensions = computeBandwidthDimensionSequence(bandwidth_dimension,physical_dimensions);
    vector<unsigned int>::const_reverse_iterator
          right_dimension = initial_bandwidth_dimensions.rbegin()
        , left_dimension = right_dimension+1;
    BOOST_FOREACH(
         unsigned int const operator_number
        ,irange(1u,number_of_sites) | reversed
    ) {
        absorb<Right>(
             randomStateSiteRight(
                 operator_sites[operator_number]->physicalDimension()
                ,LeftDimension(*(left_dimension++))
                ,RightDimension(*(right_dimension++))
             )
            ,operator_number
        );
    }

    state_site =
        randomStateSiteMiddle(
             operator_sites[0]->physicalDimension()
            ,LeftDimension(initial_bandwidth_dimensions[0])
            ,RightDimension(initial_bandwidth_dimensions[1])
        );

    complex<double> const expectation_value = computeExpectationValueAtCurrentSite();
    if(abs(expectation_value.imag()) > 1e-10) throw InitialChainEnergyNotRealError(expectation_value);
    energy = expectation_value.real();

    signalChainReset();
}
//@+node:gcross.20110218114759.1932: *3* resetBoundaries
void Chain::resetBoundaries() {
    left_expectation_boundary = ExpectationBoundary<Left>(make_trivial);

    left_overlap_boundaries.clear();
    REPEAT(projectors.size()) {
        left_overlap_boundaries.emplace_back(make_trivial);
    }

    right_expectation_boundary = ExpectationBoundary<Right>(make_trivial);

    right_overlap_boundaries.clear();
    REPEAT(projectors.size()) {
        right_overlap_boundaries.emplace_back(make_trivial);
    }
}
//@+node:gcross.20110208151104.1791: *3* sweepUntilConverged
void Chain::sweepUntilConverged() {
    double previous_energy = energy;
    performOptimizationSweep();
    while(outsideTolerance(previous_energy,energy,options.sweep_convergence_threshold)) {
        previous_energy = energy;
        performOptimizationSweep();
    }
    signalSweepsConverged();
}
//@-others

}
//@-leo
