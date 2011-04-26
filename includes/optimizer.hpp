//@+leo-ver=5-thin
//@+node:gcross.20110214155808.1922: * @file optimizer.hpp
//@@language cplusplus

//@+<< License >>
//@+node:gcross.20110220182654.2014: ** << License >>
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

#ifndef NUTCRACKER_OPTIMIZER_HPP
#define NUTCRACKER_OPTIMIZER_HPP

//@+<< Includes >>
//@+node:gcross.20110214155808.1923: ** << Includes >>
#include "operators.hpp"
#include "projectors.hpp"
#include "states.hpp"
//@-<< Includes >>

namespace Nutcracker {

//@+<< Usings >>
//@+node:gcross.20110214155808.1924: ** << Usings >>
using std::abs;
//@-<< Usings >>

//@+others
//@+node:gcross.20110214155808.1940: ** Exceptions
//@+node:gcross.20110214155808.1943: *3* OptimizerFailure
struct OptimizerFailure : public Exception {
protected:
    OptimizerFailure(string const& message);
};
//@+node:gcross.20110214155808.1944: *4* OptimizerUnableToConverge
struct OptimizerUnableToConverge : public OptimizerFailure {
    unsigned int const number_of_iterations;
    OptimizerUnableToConverge(unsigned int number_of_iterations);
};
//@+node:gcross.20110214155808.1945: *4* OptimizerObtainedEigenvalueDifferentFromExpectationValue
struct OptimizerObtainedEigenvalueDifferentFromExpectationValue : public OptimizerFailure {
    complex<double> const eigenvalue, expected_value;
    OptimizerObtainedEigenvalueDifferentFromExpectationValue(
          complex<double> eigenvalue
        , complex<double> expected_value
    );
};
//@+node:gcross.20110214155808.1946: *4* OptimizerObtainedComplexEigenvalue
struct OptimizerObtainedComplexEigenvalue : public OptimizerFailure {
    complex<double> const eigenvalue;
    OptimizerObtainedComplexEigenvalue(complex<double> eigenvalue);
};
//@+node:gcross.20110214155808.1947: *4* OptimizerObtainedGreaterEigenvalue
struct OptimizerObtainedGreaterEigenvalue : public OptimizerFailure {
    double const old_eigenvalue, new_eigenvalue;
    OptimizerObtainedGreaterEigenvalue(
          double const old_eigenvalue
        , double const new_eigenvalue
    );
};
//@+node:gcross.20110214155808.1948: *4* OptimizerObtainedVanishingEigenvector
struct OptimizerObtainedVanishingEigenvector : public OptimizerFailure {
    double const norm;
    OptimizerObtainedVanishingEigenvector(double norm);
};
//@+node:gcross.20110214155808.1949: *4* OptimizerObtainedEigenvectorInProjectorSpace
struct OptimizerObtainedEigenvectorInProjectorSpace : public OptimizerFailure {
    double const overlap;
    OptimizerObtainedEigenvectorInProjectorSpace(double overlap);
};
//@+node:gcross.20110214155808.1950: *4* OptimizerGivenTooManyProjectors
struct OptimizerGivenTooManyProjectors : public OptimizerFailure {
    unsigned int const number_of_projectors;
    PhysicalDimension const physical_dimension;
    LeftDimension const left_dimension;
    RightDimension const right_dimension;

    OptimizerGivenTooManyProjectors(
          unsigned int number_of_projectors
        , PhysicalDimension physical_dimension
        , LeftDimension left_dimension
        , RightDimension right_dimension
    );
};
//@+node:gcross.20110214155808.1951: *4* OptimizerGivenGuessInProjectorSpace
struct OptimizerGivenGuessInProjectorSpace : public OptimizerFailure {
    OptimizerGivenGuessInProjectorSpace();
};
//@+node:gcross.20110214155808.1952: *4* OptimizerUnknownFailure
struct OptimizerUnknownFailure : public OptimizerFailure {
    int const error_code;
    OptimizerUnknownFailure(int error_code);
};
//@+node:gcross.20110214155808.1981: ** Classes
//@+node:gcross.20110214155808.1982: *3* OptimizerResult
struct OptimizerResult {
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(OptimizerResult)
public:
    unsigned int number_of_iterations;
    double eigenvalue;
    StateSite<Middle> state_site;

    OptimizerResult(BOOST_RV_REF(OptimizerResult) other)
      : number_of_iterations(other.number_of_iterations)
      , eigenvalue(other.eigenvalue)
      , state_site(boost::move(other.state_site))
    {}

    OptimizerResult(
          unsigned int const number_of_iterations
        , double const eigenvalue
        , BOOST_RV_REF(StateSite<Middle>) state_site
    ) : number_of_iterations(number_of_iterations)
      , eigenvalue(eigenvalue)
      , state_site(state_site)
    {}
};
//@+node:gcross.20110214155808.1926: ** Functions
OptimizerResult optimizeStateSite(
      ExpectationBoundary<Left> const& left_boundary
    , StateSite<Middle> const& current_state_site
    , OperatorSite const& operator_site
    , ExpectationBoundary<Right> const& right_boundary
    , ProjectorMatrix const& projector_matrix
    , double const convergence_threshold
    , double const sanity_check_threshold
    , unsigned int const maximum_number_of_iterations
);
//@-others

}

#endif
//@-leo
