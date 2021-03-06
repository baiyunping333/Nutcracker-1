#ifndef NUTCRACKER_INFINITE_CHAIN_HPP
#define NUTCRACKER_INFINITE_CHAIN_HPP

#include <algorithm>
#include <limits>
#include <boost/optional.hpp>

#include "nutcracker/base_chain.hpp"
#include "nutcracker/boundaries.hpp"
#include "nutcracker/infinite_state.hpp"
#include "nutcracker/infinite_operators.hpp"

namespace Nutcracker {

class InfiniteChain: public BaseChain { // {{{
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(InfiniteChain)
protected:
    OperatorSite operator_site;
    optional<double> maybe_convergence_energy;
public:
    inline InfiniteChain(
        BOOST_RV_REF(InfiniteChain) other
    ) : BaseChain(static_cast<BOOST_RV_REF(BaseChain)>(other))
      , operator_site(boost::move(other.operator_site))
    { }

    inline InfiniteChain(
        CopyFrom<InfiniteChain const> const other
    ) : BaseChain(other)
      , operator_site(copyFrom(other->operator_site))
    { }

    InfiniteChain(
        BOOST_RV_REF(InfiniteOperator) op,
        boost::optional<ChainOptions const&> maybe_options = boost::none
    );

    InfiniteChain(
        BOOST_RV_REF(InfiniteOperator) op,
        BOOST_RV_REF(InfiniteState) state,
        boost::optional<ChainOptions const&> maybe_options = boost::none
    );

    template<typename side> void move();
    unsigned int bandwidthDimension() const { return state_site.leftDimension(); }
    virtual OperatorSite const& getCurrentOperatorSite() const { return operator_site; }
    virtual ProjectorMatrix const& getCurrentProjectorMatrix() const { return ProjectorMatrix::getNull(); }
    virtual unsigned int getCurrentBandwidthDimension() const { return left_expectation_boundary.stateDimension(); }
    virtual unsigned int getMaximumBandwidthDimension() const { return std::numeric_limits<unsigned int>::max(); }
    virtual void performOptimizationSweep();
    virtual void increaseBandwidthDimension(unsigned int const new_bandwidth_dimension);
    StateSite<Middle> const& getStateSite() const { return state_site; }

    virtual boost::optional<double> getConvergenceEnergy() const { return maybe_convergence_energy; }

    virtual void dump() const;

}; // }}}

// External methods {{{

template<typename side> void InfiniteChain::move() {{{
    optimized = false;
    energy_computed = false;

    ExpectationBoundary<side>& expectation_boundary = expectationBoundary<side>();
    ExpectationBoundary<side> new_expectation_boundary(
        contract<side>::SOS_absorb(
             expectation_boundary
            ,state_site
            ,operator_site
        )
    );
    expectationBoundary<side>() = boost::move(new_expectation_boundary);
}}}

// }}}

}

#endif
