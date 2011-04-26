//@+leo-ver=5-thin
//@+node:gcross.20110214155808.1884: * @file optimizer.cpp
//@@language cplusplus

//@+<< License >>
//@+node:gcross.20110220182654.2034: ** << License >>
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
//@+node:gcross.20110214155808.1885: ** << Includes >>
#include "boundaries.hpp"
#include "core.hpp"
#include "optimizer.hpp"
#include "projectors.hpp"
//@-<< Includes >>

namespace Nutcracker {

//@+<< Usings >>
//@+node:gcross.20110214155808.1886: ** << Usings >>
//@-<< Usings >>

//@+others
//@+node:gcross.20110214155808.1903: ** Exceptions
//@+node:gcross.20110214155808.1905: *3* OptimizerFailure
OptimizerFailure::OptimizerFailure(string const& message)
    : Exception(message)
{ }
//@+node:gcross.20110214155808.1906: *4* OptimizerGivenGuessInProjectorSpace
OptimizerGivenGuessInProjectorSpace::OptimizerGivenGuessInProjectorSpace()
    : OptimizerFailure(
        "Optimizer was given a guess within the forbidden orthogonal space"
      )
{ }
//@+node:gcross.20110214155808.1907: *4* OptimizerGivenTooManyProjectors
OptimizerGivenTooManyProjectors::OptimizerGivenTooManyProjectors(
      unsigned int const number_of_projectors
    , PhysicalDimension const physical_dimension
    , LeftDimension const left_dimension
    , RightDimension const right_dimension
) : OptimizerFailure(
        (format("Optimizer was given too many projectors (%1% >= %2%*%3%*%4% = %5%)") 
            % number_of_projectors
            % *physical_dimension
            % *left_dimension
            % *right_dimension
            % ((*physical_dimension)*(*left_dimension)*(*right_dimension))
        ).str()
    )
  , number_of_projectors(number_of_projectors)
  , physical_dimension(physical_dimension)
  , left_dimension(left_dimension)
  , right_dimension(right_dimension)
{ }
//@+node:gcross.20110214155808.1908: *4* OptimizerObtainedComplexEigenvalue
OptimizerObtainedComplexEigenvalue::OptimizerObtainedComplexEigenvalue(
       complex<double> const eigenvalue
) : OptimizerFailure(
        (format("Optimizer obtained complex eigenvalue (%|.15|)") 
            % eigenvalue
        ).str()
    )
  , eigenvalue(eigenvalue)
{ }
//@+node:gcross.20110214155808.1909: *4* OptimizerObtainedEigenvalueDifferentFromExpectationValue
OptimizerObtainedEigenvalueDifferentFromExpectationValue::OptimizerObtainedEigenvalueDifferentFromExpectationValue(
      complex<double> const eigenvalue
    , complex<double> const expected_value
) : OptimizerFailure(
        (format("Optimizer obtained eigenvalue that was different from the final expected value (%|.15| != %|.15|)")
            % eigenvalue
            % expected_value
        ).str()
    )
  , eigenvalue(eigenvalue)
  , expected_value(expected_value)
{ }
//@+node:gcross.20110214155808.1910: *4* OptimizerObtainedEigenvectorInProjectorSpace
OptimizerObtainedEigenvectorInProjectorSpace::OptimizerObtainedEigenvectorInProjectorSpace(
       double const overlap
) : OptimizerFailure(
        (format("Optimizer obtained eigenvector overlapping with the forbidden orthogonal space (overlap = %|.15|)") 
            % overlap
        ).str()
    )
  , overlap(overlap)
{ }
//@+node:gcross.20110214155808.1911: *4* OptimizerObtainedGreaterEigenvalue
OptimizerObtainedGreaterEigenvalue::OptimizerObtainedGreaterEigenvalue(
      double const old_eigenvalue
    , double const new_eigenvalue
) : OptimizerFailure(
        (format("Optimizer obtained an eigenvalue that was greater than the old eigenvalue (%|.15| > %|.15|)")
            % new_eigenvalue
            % old_eigenvalue
        ).str()
    )
  , old_eigenvalue(old_eigenvalue)
  , new_eigenvalue(new_eigenvalue)
{ }
//@+node:gcross.20110214155808.1912: *4* OptimizerObtainedVanishingEigenvector
OptimizerObtainedVanishingEigenvector::OptimizerObtainedVanishingEigenvector(
       double const norm
) : OptimizerFailure(
        (format("Optimizer obtained vanishing eigenvector (norm = %|.15|)") 
            % norm
        ).str()
    )
  , norm(norm)
{ }
//@+node:gcross.20110214155808.1913: *4* OptimizerUnableToConverge
OptimizerUnableToConverge::OptimizerUnableToConverge(
       unsigned int const number_of_iterations
) : OptimizerFailure(
        (format("Optimizer failed to converge after %1% iterations") 
            % number_of_iterations
        ).str()
    )
  , number_of_iterations(number_of_iterations)
{ }
//@+node:gcross.20110214155808.1914: *4* OptimizerUnknownFailure
OptimizerUnknownFailure::OptimizerUnknownFailure(
       int const error_code
) : OptimizerFailure(
        (format("Optimizer failed with an unknown error code: %1%") 
            % error_code
        ).str()
    )
  , error_code(error_code)
{ }
//@+node:gcross.20110214155808.1889: ** Functions
//@+node:gcross.20110214155808.1890: *3* optimizeStateSite
OptimizerResult optimizeStateSite(
      ExpectationBoundary<Left> const& left_boundary
    , StateSite<Middle> const& current_state_site
    , OperatorSite const& operator_site
    , ExpectationBoundary<Right> const& right_boundary
    , ProjectorMatrix const& projector_matrix
    , double const convergence_threshold
    , double const sanity_check_threshold
    , unsigned int const maximum_number_of_iterations
) {
    uint32_t number_of_iterations = maximum_number_of_iterations;
    complex<double> eigenvalue;
    StateSite<Middle> new_state_site(dimensionsOf(current_state_site));

    double normal;
    int const status =
        projector_matrix.valid()
            ? Core::optimize(
                 left_boundary | current_state_site
                ,current_state_site | right_boundary
                ,left_boundary | operator_site
                ,operator_site | right_boundary
                ,operator_site | current_state_site
                ,left_boundary
                ,operator_site.numberOfMatrices(),operator_site,operator_site
                ,right_boundary
                ,projector_matrix.numberOfProjectors()
                ,projector_matrix.numberOfReflectors()
                ,projector_matrix.orthogonalSubspaceDimension()
                ,projector_matrix.reflectorData()
                ,projector_matrix.coefficientData()
                ,projector_matrix.swapData()
                ,"SR"
                ,convergence_threshold
                ,number_of_iterations
                ,current_state_site
                ,new_state_site
                ,eigenvalue
                ,normal
              )
            : Core::optimize(
                 left_boundary | current_state_site
                ,current_state_site | right_boundary
                ,left_boundary | operator_site
                ,operator_site | right_boundary
                ,operator_site | current_state_site
                ,left_boundary
                ,operator_site.numberOfMatrices(),operator_site,operator_site
                ,right_boundary
                ,0
                ,0
                ,current_state_site.size()
                ,NULL
                ,NULL
                ,NULL
                ,"SR"
                ,convergence_threshold
                ,number_of_iterations
                ,current_state_site
                ,new_state_site
                ,eigenvalue
                ,normal
              )
            ;
    complex<double> const expectation_value =
        computeExpectationValueAtSite(
             left_boundary
            ,new_state_site
            ,operator_site
            ,right_boundary
        );
    double const overlap =
        projector_matrix.valid()
            ? computeOverlapWithProjectors(
                 projector_matrix
                ,new_state_site
              )
            : 0
            ;
    switch(status) {
        case -14:
            throw OptimizerUnableToConverge(number_of_iterations);
        case  10:
            throw OptimizerGivenTooManyProjectors(
                 projector_matrix.numberOfProjectors()
                ,current_state_site.physicalDimension()
                ,current_state_site.leftDimension()
                ,current_state_site.rightDimension()
            );
        case 11:
            throw OptimizerGivenGuessInProjectorSpace();
        case 0:
            if(outsideTolerance(eigenvalue,expectation_value,sanity_check_threshold))
                throw OptimizerObtainedEigenvalueDifferentFromExpectationValue(
                     eigenvalue
                    ,expectation_value
                );
            if(abs(eigenvalue) > sanity_check_threshold
            && abs(eigenvalue.imag())/abs(eigenvalue) > sanity_check_threshold
              ) throw OptimizerObtainedComplexEigenvalue(eigenvalue);
            if(normal < 1-sanity_check_threshold)
                throw OptimizerObtainedVanishingEigenvector(normal);
            if(overlap > sanity_check_threshold)
                throw OptimizerObtainedEigenvectorInProjectorSpace(overlap);
            break;
        default:
            throw OptimizerUnknownFailure(status);
    }
    return OptimizerResult(
         number_of_iterations
        ,eigenvalue.real()
        ,boost::move(new_state_site)
    );
}
//@-others

}
//@-leo
