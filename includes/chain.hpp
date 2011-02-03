//@+leo-ver=5-thin
//@+node:gcross.20110130170743.1665: * @thin chain.hpp
//@@language cplusplus

#ifndef NUTCRACKER_CHAIN_HPP
#define NUTCRACKER_CHAIN_HPP

//@+<< Includes >>
//@+node:gcross.20110130170743.1666: ** << Includes >>
#include <boost/container/vector.hpp>
#include <boost/move/move.hpp>

#include "core.hpp"
#include "tensors.hpp"
//@-<< Includes >>

namespace Nutcracker {

//@+<< Usings >>
//@+node:gcross.20110130170743.1667: ** << Usings >>
using namespace boost;
using namespace std;

namespace moveable = boost::container;
//@-<< Usings >>

//@+others
//@+node:gcross.20110130193548.1684: ** Exceptions
//@+node:gcross.20110202200838.1712: *3* InitialChainEnergyNotRealError
struct InitialChainEnergyNotRealError : public Exception {
    complex<double> const energy;
    InitialChainEnergyNotRealError(complex<double> const energy)
      : Exception((
            format("The initial chain energy is not real (energy = %1%).")
                % energy
        ).str())
      , energy(energy)
    {}
};
//@+node:gcross.20110130193548.1685: *3* RequestedBandwidthDimensionTooLargeError
struct RequestedBandwidthDimensionTooLargeError : public Exception {
    unsigned int const
          requested_bandwidth_dimension
        , greatest_possible_bandwidth_dimension
        ;
    RequestedBandwidthDimensionTooLargeError(
        unsigned int const requested_bandwidth_dimension
      , unsigned int const greatest_possible_bandwidth_dimension
    ) : Exception((
            format("Requested bandwidth dimension %1% is too large;  the highest possible with the given physical dimensions is %2%.")
                % requested_bandwidth_dimension
                % greatest_possible_bandwidth_dimension
        ).str())
      , requested_bandwidth_dimension(requested_bandwidth_dimension)
      , greatest_possible_bandwidth_dimension(greatest_possible_bandwidth_dimension)    
    {}
};
//@+node:gcross.20110130170743.1683: ** Functions
//@+node:gcross.20110130170743.1684: *3* computeBandwidthDimensionSequence
vector<unsigned int> computeBandwidthDimensionSequence(
    unsigned int const requested_bandwidth_dimension
   ,vector<unsigned int> const& physical_dimensions
);
//@+node:gcross.20110202175920.1701: ** Classes
//@+node:gcross.20110202175920.1702: *3* OverlapSiteTrio
struct OverlapSiteTrio {
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(OverlapSiteTrio)

    OverlapSite<Left> left;
    OverlapSite<Middle> middle;
    OverlapSite<Right> right;
public:
    OverlapSiteTrio(BOOST_RV_REF(OverlapSiteTrio) other)
      : left(boost::move(left))
      , middle(boost::move(middle))
      , right(boost::move(right))
    { }

    OverlapSiteTrio(
          BOOST_RV_REF(OverlapSite<Left>) left
        , BOOST_RV_REF(OverlapSite<Middle>) middle
        , BOOST_RV_REF(OverlapSite<Right>) right
    ) : left(left)
      , middle(middle)
      , right(right)
    { }

    void operator=(BOOST_RV_REF(OverlapSiteTrio) other) {
        left = boost::move(other.left);
        middle = boost::move(other.middle);
        right = boost::move(other.right);
    }

    void swap(OverlapSiteTrio& other) {
        left.swap(other.left);
        middle.swap(other.middle);
        right.swap(other.right);
    }

    template<Side side> OverlapSite<side> const& get() {}
};
template<> inline OverlapSite<Left> const& OverlapSiteTrio::get<Left>() { return left; }
template<> inline OverlapSite<Middle> const& OverlapSiteTrio::get<Middle>() { return middle; }
template<> inline OverlapSite<Right> const& OverlapSiteTrio::get<Right>() { return right; }
//@+node:gcross.20110202175920.1703: *3* Neighbor
template<Side side> struct Neighbor {
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(Neighbor)
public:
    ExpectationBoundary<side> expectation_boundary;
    StateSite<side> state_site;
    OperatorSite operator_site;
    moveable::vector<OverlapBoundary<side> > overlap_boundaries;
    moveable::vector<OverlapSiteTrio> overlap_site_trios;

    Neighbor(BOOST_RV_REF(Neighbor) other)
      : expectation_boundary(boost::move(expectation_boundary))
      , state_site(boost::move(state_site))
      , operator_site(boost::move(operator_site))
      , overlap_boundaries(boost::move(overlap_boundaries))
      , overlap_site_trios(boost::move(overlap_site_trios))
    { }

    Neighbor(
          BOOST_RV_REF(ExpectationBoundary<side>) expectation_boundary
        , BOOST_RV_REF(StateSite<side>) state_site
        , BOOST_RV_REF(OperatorSite) operator_site
        , BOOST_RV_REF(moveable::vector<OverlapBoundary<side> >) overlap_boundaries
        , BOOST_RV_REF(moveable::vector<OverlapSiteTrio>) overlap_site_trios
    ) : expectation_boundary(expectation_boundary)
      , state_site(state_site)
      , operator_site(operator_site)
      , overlap_boundaries(overlap_boundaries)
      , overlap_site_trios(overlap_site_trios)
    { }

    void operator=(BOOST_RV_REF(Neighbor) other) {
        expectation_boundary = boost::move(other.expectation_boundary);
        state_site = boost::move(other.state_site);
        operator_site = boost::move(other.operator_site);
        overlap_boundaries = boost::move(other.overlap_boundaries);
        overlap_site_trios = boost::move(other.overlap_site_trios);
    }
};
//@+node:gcross.20110202175920.1704: *3* Chain
class Chain {
public:
    unsigned int const number_of_sites;
protected:
    unsigned int current_site_number;
    ExpectationBoundary<Left> left_expectation_boundary;
    moveable::vector<OverlapBoundary<Left> > left_overlap_boundaries;
    ExpectationBoundary<Right> right_expectation_boundary;
    moveable::vector<OverlapBoundary<Right> > right_overlap_boundaries;
    StateSite<Middle> state_site;
    moveable::vector<OverlapSiteTrio> overlap_site_trios;
    OperatorSite operator_site;
    moveable::vector<Neighbor<Left> > left_neighbors;
    moveable::vector<Neighbor<Right> > right_neighbors;
    ProjectorMatrix projector_matrix;
    double energy;
    vector<unsigned int> initial_bandwidth_dimensions;

    template<Side side> ExpectationBoundary<side>& expectationBoundary() { throw BadProgrammerException("no middle expectation boundary"); }
    template<Side side> moveable::vector<OverlapBoundary<side> >& overlapBoundaries() { throw BadProgrammerException("no middle overlap boundary"); }
    template<Side side> moveable::vector<Neighbor<side> >& neighbors() { throw BadProgrammerException("no middle neighbors"); }

public:
    Chain(
      moveable::vector<OperatorSite> operators
    , unsigned int const initial_bandwidth = 1
    );

    double getEnergy() const { return energy; }
    complex<double> computeExpectationValue() const;
    double computeStateNorm() const;

    template<Side side> void move();
};

template<> inline ExpectationBoundary<Left>& Chain::expectationBoundary<Left>() { return left_expectation_boundary; }
template<> inline ExpectationBoundary<Right>& Chain::expectationBoundary<Right>() { return right_expectation_boundary; }

template<> inline moveable::vector<OverlapBoundary<Left> >& Chain::overlapBoundaries<Left>() { return left_overlap_boundaries; }
template<> inline moveable::vector<OverlapBoundary<Right> >& Chain::overlapBoundaries<Right>() { return right_overlap_boundaries; }

template<> inline moveable::vector<Neighbor<Left> >& Chain::neighbors<Left>() { return left_neighbors; }
template<> inline moveable::vector<Neighbor<Right> >& Chain::neighbors<Right>() { return right_neighbors; }

//@+others
//@+node:gcross.20110202175920.1705: *4* move
template<Side side> void Chain::move() {
    if(side == Left) {
        assert(--current_site_number >= 0);
    } else if(side == Right) {
        assert(++current_site_number >= 0);
    } else {
        throw BadProgrammerException("the chain can only be moved left or right, not 'middle'");
    }

    typedef Other<side> other;

    moveable::vector<Neighbor<side> >& side_neighbors = neighbors<side>();

    Neighbor<side>& neighbor = side_neighbors.back();

    MoveSiteCursorResult<side> cursor(moveSiteCursor<side>::from(
         *neighbor.state_site
        ,*state_site
    ));

    ExpectationBoundary<other::side>& expectation_boundary = expectationBoundary<other::side>();
    ExpectationBoundary<other::side> new_expectation_boundary(
        contract<other::side>::SOS(
             expectation_boundary
            ,cursor.other_side_state_site
            ,operator_site
        )
    );

    moveable::vector<OverlapBoundary<other::side> >& overlap_boundaries = overlapBoundaries<other::side>();
    moveable::vector<OverlapBoundary<other::side> > new_overlap_boundaries;
    new_overlap_boundaries.reserve(overlap_boundaries.size());
    typename moveable::vector<OverlapBoundary<other::side> >::const_iterator overlap_boundary = overlap_boundaries.begin();
    BOOST_FOREACH(
         OverlapSiteTrio const& overlap_site_trio
        ,overlap_site_trios
    ) {
        new_overlap_boundaries.push_back(
            contract<other::side>::SS(
                 *overlap_boundary
                ,overlap_site_trio.get<other::side>()
                ,cursor.other_side_state_site
            )
        );
        ++overlap_boundary;
    }

    neighbors<other::side>().emplace_back(
         boost::move(expectation_boundary)
        ,boost::move(cursor.other_side_state_site)
        ,boost::move(operator_site)
        ,boost::move(overlap_boundaries)
        ,boost::move(overlap_site_trios)
    );

    expectationBoundary<side>() = boost::move(neighbor.expectation_boundary);
    state_site = boost::move(cursor.middle_state_site);
    operator_site = boost::move(neighbor.operator_site);
    overlapBoundaries<side>() = boost::move(neighbor.overlap_boundaries);
    overlap_site_trios = boost::move(neighbor.overlap_site_trios);

    side_neighbors.pop_back();
}
//@-others
//@-others

}

#endif
//@-leo
