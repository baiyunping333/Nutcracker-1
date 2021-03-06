#include <boost/assign/list_of.hpp>
#include <boost/range/algorithm/equal.hpp>
#include <complex>
#include <illuminate.hpp>

#include "nutcracker/boundaries.hpp"

#include "test_utils.hpp"

using namespace Nutcracker;

using boost::assign::list_of;
using boost::equal;

using std::abs;

TEST_SUITE(Boundaries) {

TEST_SUITE(computeExpectationValueAtSite) {

    TEST_CASE(trivial_with_all_dimensions_1) {
        complex<double> expected_expectation_value = 1;
        complex<double> actual_expectation_value =
            computeExpectationValueAtSite(
                 ExpectationBoundary<Left>::trivial
                ,StateSite<Middle>::trivial
                ,OperatorSite::trivial
                ,ExpectationBoundary<Right>::trivial
            );
        ASSERT_EQ(expected_expectation_value,actual_expectation_value);
    }
    TEST_CASE(trivial_with_all_dimensions_1_and_imaginary_component) {
        OperatorSite const operator_site
            (LeftDimension(1)
            ,RightDimension(1)
            ,fillWithRange(list_of(1)(1))
            ,fillWithRange(list_of(complex<double>(0,1)))
            );

        complex<double> expected_expectation_value(0,1);
        complex<double> actual_expectation_value =
            computeExpectationValueAtSite(
                 ExpectationBoundary<Left>::trivial
                ,StateSite<Middle>::trivial
                ,operator_site
                ,ExpectationBoundary<Right>::trivial
            );
        ASSERT_EQ(expected_expectation_value,actual_expectation_value);
    }
    TEST_SUITE(trivial_with_physical_dimension_2) {

        void runTest(
              complex<double> const a
            , complex<double> const b
            , complex<double> const c
        ) {
            StateSite<Middle> const state_site
                (LeftDimension(1)
                ,RightDimension(1)
                ,fillWithRange(list_of(a)(b))
                );
            OperatorSite const operator_site
                (LeftDimension(1)
                ,RightDimension(1)
                ,fillWithRange(list_of(1)(1))
                ,fillWithRange(list_of(1)(0)(0)(-1))
                );
            complex<double> expectation_value =
                computeExpectationValueAtSite(
                     ExpectationBoundary<Left>::trivial
                    ,state_site
                    ,operator_site
                    ,ExpectationBoundary<Right>::trivial
                );
            ASSERT_EQ(c,expectation_value);
        }

        TEST_CASE(_1_0) { runTest(1,0,1); }
        TEST_CASE(_i_0) { runTest(complex<double>(0,1),0,1); }
        TEST_CASE(_0_1) { runTest(0,1,-1); }
        TEST_CASE(_0_i) { runTest(0,complex<double>(0,1),-1); }
    }

}
TEST_SUITE(contractExpectationBoundaries) {

    TEST_CASE(trivial_with_all_dimensions_1) {
        complex<double> expected_value = 1;
        complex<double> actual_value =
            contractExpectationBoundaries(
                 ExpectationBoundary<Left>::trivial
                ,ExpectationBoundary<Right>::trivial
            );
        ASSERT_EQ(expected_value,actual_value);
    }
    TEST_CASE(non_trivial) {

        ExpectationBoundary<Left> const left_boundary
            (OperatorDimension(2)
            ,fillWithRange(list_of
                (c(1,2))(c(2,3))(c(3,4))
                (c(4,5))(c(5,6))(c(6,7))
                (c(7,8))(c(8,9))(c(9,10))

                (c(1,9))(c(2,8))(c(3,7))
                (c(4,6))(c(5,5))(c(6,4))
                (c(7,3))(c(8,2))(c(9,1))
            ));

        ExpectationBoundary<Right> const right_boundary
            (OperatorDimension(2)
            ,fillWithRange(list_of
                (c(1,1))(c(2,2))(c(3,3))
                (c(6,6))(c(5,5))(c(4,4))
                (c(7,7))(c(8,8))(c(9,9))

                (c(1,-1))(c(2,-2))(c(3,-3))
                (c(6,-6))(c(5,-5))(c(4,-4))
                (c(7,-7))(c(8,-8))(c(9,-9))
            ));

        complex<double> expected_value = c(405,495);
        complex<double> actual_value =
            contractExpectationBoundaries(
                 left_boundary
                ,right_boundary
            );
        ASSERT_EQ(expected_value,actual_value);
    }

}
TEST_SUITE(computeSOSLeft) {

    TEST_CASE(trivial_with_all_dimensions_1) {
        ExpectationBoundary<Left> const new_boundary(
            contractSOSLeft(
                 ExpectationBoundary<Left>::trivial
                ,StateSite<Left>::trivial
                ,OperatorSite::trivial
            )
        );
        EXPECT_EQ_VAL(new_boundary.operatorDimension(),1u);
        EXPECT_EQ_VAL(new_boundary.stateDimension(),1u);
        EXPECT_TRUE(equal(list_of(complex<double>(1)),new_boundary));
    }
    TEST_CASE(trivial_with_operator_dimension_2) {
        OperatorSite const operator_site
            (LeftDimension(1)
            ,RightDimension(2)
            ,fillWithRange(list_of(1)(2))
            ,fillWithRange(list_of(complex<double>(0,1)))
            );
        ExpectationBoundary<Left> const new_boundary(
            contractSOSLeft(
                 ExpectationBoundary<Left>::trivial
                ,StateSite<Left>::trivial
                ,operator_site
            )
        );
        EXPECT_EQ_VAL(new_boundary.operatorDimension(),2u);
        EXPECT_EQ_VAL(new_boundary.stateDimension(),1u);
        EXPECT_TRUE(equal(list_of(complex<double>(0))(complex<double>(0,1)),new_boundary));
    }
    TEST_CASE(non_trivial) {
        ExpectationBoundary<Left> const boundary
            (OperatorDimension(3)
            ,fillWithRange(list_of
                (c(2,0))(c(0,4))
                (c(1,0))(c(0,5))

                (c(1,0))(c(0,5))
                (c(1,0))(c(1,0))

                (c(3,0))(c(0,6))
                (c(0,2))(c(-2,0))
            ));
        StateSite<Left> const state_site
            (LeftDimension(2)
            ,RightDimension(4)
            ,fillWithRange(list_of
                (c(1,0))(c(1,0))(c(-1,0))(c(0,2))
                (c(1,0))(c(-1,0))(c(2,0))(c(-2,0))
            ));
        OperatorSite const operator_site
            (LeftDimension(3)
            ,RightDimension(2)
            ,fillWithRange(list_of(1)(2)(3)(1))
            ,fillWithRange(list_of(complex<double>(1,0))(complex<double>(0,1)))
            );
        ExpectationBoundary<Left> const actual_boundary(
            contractSOSLeft(
                 boundary
                ,state_site
                ,operator_site
            )
        );
        ASSERT_EQ_VAL(actual_boundary.operatorDimension(),2u);
        ASSERT_EQ_VAL(actual_boundary.stateDimension(),4u);
        complex<double> const expected_boundary[] =
            {c(-8.0,1.0),c(4.0,5.0),c(-10.0,-7.0),c(6.0,0.0)
            ,c(-4.0,5.0),c(8.0,1.0),c(-14.0,1.0),c(6.0,0.0)
            ,c(2.0,-7.0),c(-10.0,1.0),c(16.0,-5.0),c(-6.0,0.0)
            ,c(10.0,16.0),c(10.0,-16.0),c(-10.0,32.0),c(0.0,-12.0)

            ,c(3.0,9.0),c(3.0,-9.0),c(-3.0,18.0),c(0.0,-12.0)
            ,c(1.0,-1.0),c(1.0,1.0),c(-1.0,-2.0),c(0.0,4.0)
            ,c(0.0,6.0),c(0.0,-6.0),c(0.0,12.0),c(0.0,-12.0)
            ,c(6.0,-14.0),c(-10.0,6.0),c(18.0,-16.0),c(-8.0,16.0)
            };
        complex<double> const *actual = actual_boundary;
        BOOST_FOREACH(complex<double> expected, expected_boundary) {
            ASSERT_EQ(expected,*actual);
            ++actual;
        }
    }

}
TEST_SUITE(computeSOSRight) {

    TEST_CASE(trivial_with_all_dimensions_1) {
        ExpectationBoundary<Right> const new_boundary(
            contractSOSRight(
                 ExpectationBoundary<Right>::trivial
                ,StateSite<Right>::trivial
                ,OperatorSite::trivial
            )
        );
        EXPECT_EQ_VAL(new_boundary.operatorDimension(),1u);
        EXPECT_EQ_VAL(new_boundary.stateDimension(),1u);
        EXPECT_TRUE(equal(list_of(complex<double>(1)),new_boundary));
    }
    TEST_CASE(trivial_with_operator_dimension_2) {
        OperatorSite const operator_site
            (LeftDimension(2)
            ,RightDimension(1)
            ,fillWithRange(list_of(2)(1))
            ,fillWithRange(list_of(complex<double>(0,1)))
            );
        ExpectationBoundary<Right> const new_boundary(
            contractSOSRight(
                 ExpectationBoundary<Right>::trivial
                ,StateSite<Right>::trivial
                ,operator_site
            )
        );
        EXPECT_EQ_VAL(new_boundary.operatorDimension(),2u);
        EXPECT_EQ_VAL(new_boundary.stateDimension(),1u);
        EXPECT_TRUE(equal(list_of(complex<double>(0))(complex<double>(0,1)),new_boundary));
    }
    TEST_CASE(non_trivial) {
        ExpectationBoundary<Right> const boundary
            (OperatorDimension(3)
            ,fillWithRange(list_of
                (c(2,0))(c(0,4))
                (c(1,0))(c(0,5))

                (c(1,0))(c(0,5))
                (c(1,0))(c(1,0))

                (c(3,0))(c(0,6))
                (c(0,2))(c(-2,0))
            ));
        StateSite<Right> const state_site
            (LeftDimension(4)
            ,RightDimension(2)
            ,fillWithRange(list_of
                (c(1,0))(c(1,0))
                (c(1,0))(c(-1,0))
                (c(-1,0))(c(2,0))
                (c(0,2))(c(-2,0))
            ));
        OperatorSite const operator_site
            (LeftDimension(2)
            ,RightDimension(3)
            ,fillWithRange(list_of(2)(1)(1)(3))
            ,fillWithRange(list_of(complex<double>(1,0))(complex<double>(0,1)))
            );
        ExpectationBoundary<Right> const actual_boundary(
            contractSOSRight(
                 boundary
                ,state_site
                ,operator_site
            )
        );
        ASSERT_EQ_VAL(actual_boundary.operatorDimension(),2u);
        ASSERT_EQ_VAL(actual_boundary.stateDimension(),4u);
        complex<double> const expected_boundary[] =
            {c(-8.0,1.0),c(4.0,5.0),c(-10.0,-7.0),c(18.0,8.0)
            ,c(-4.0,5.0),c(8.0,1.0),c(-14.0,1.0),c(18.0,-8.0)
            ,c(2.0,-7.0),c(-10.0,1.0),c(16.0,-5.0),c(-18.0,16.0)
            ,c(-2.0,-8.0),c(-2.0,8.0),c(2.0,-16.0),c(0.0,20.0)

            ,c(3.0,9.0),c(3.0,-9.0),c(-3.0,18.0),c(0.0,-24.0)
            ,c(1.0,-1.0),c(1.0,1.0),c(-1.0,-2.0),c(0.0,0.0)
            ,c(0.0,6.0),c(0.0,-6.0),c(0.0,12.0),c(0.0,-12.0)
            ,c(-10.0,-6.0),c(6.0,14.0),c(-14.0,-24.0),c(24.0,24.0)
            };
        complex<double> const *actual = actual_boundary;
        BOOST_FOREACH(complex<double> expected, expected_boundary) {
            ASSERT_EQ(expected,*actual);
            ++actual;
        }
    }

}
TEST_SUITE(computeSSLeft) {

    TEST_CASE(trivial_with_all_dimensions_1) {
        OverlapBoundary<Left> const new_boundary(
            contractVSLeft(
                 OverlapBoundary<Left>::trivial
                ,OverlapSite<Left>::trivial
                ,StateSite<Left>::trivial
            )
        );
        EXPECT_EQ_VAL(new_boundary.overlapDimension(),1u);
        EXPECT_EQ_VAL(new_boundary.stateDimension(),1u);
        EXPECT_TRUE(equal(list_of(complex<double>(1)),new_boundary));
    }
    TEST_CASE(trivial_with_physical_dimension_2) {
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
            contractVSLeft(
                 OverlapBoundary<Left>::trivial
                ,overlap_site
                ,state_site
            )
        );
        EXPECT_EQ_VAL(new_boundary.overlapDimension(),1u);
        EXPECT_EQ_VAL(new_boundary.stateDimension(),1u);
        EXPECT_TRUE(equal(list_of(complex<double>(4)),new_boundary));
    }
    TEST_CASE(non_trivial) {
        OverlapBoundary<Left> const boundary
            (OverlapDimension(3)
            ,fillWithRange(list_of
                (c(2,0))(c(0,4))
                (c(1,0))(c(0,5))
                (c(3,0))(c(0,6))
            ));
        OverlapSite<Left> const overlap_site
            (RightDimension(2)
            ,LeftDimension(3)
            ,fillWithRange(list_of
                (c( 1,0))(c( 0,-1))(c(0, 1))
                (c( 1,0))(c( 1, 0))(c(0,-1))
            ));
        StateSite<Left> const state_site
            (LeftDimension(2)
            ,RightDimension(5)
            ,fillWithRange(list_of
                (c(1,0))(c(1,0))(c( 1,0))(c(-1, 0))(c(-1,0))
                (c(2,0))(c(0,2))(c(-2,0))(c( 1,-2))(c( 2,0))
            ));
        OverlapBoundary<Left> const actual_boundary(
            contractVSLeft(
                 boundary
                ,overlap_site
                ,state_site
            )
        );
        ASSERT_EQ_VAL(actual_boundary.overlapDimension(),2u);
        ASSERT_EQ_VAL(actual_boundary.stateDimension(),5u);
        complex<double> const expected_boundary[] =
            {c(0,10),c(-6,0),c(4,-6),c(5,4),c(-4,6)
            ,c(15,15),c(-15,9),c(-9,-21),c(21,0),c(9,21)
            }
        ;
        complex<double> const *actual = actual_boundary;
        BOOST_FOREACH(complex<double> expected, expected_boundary) {
            ASSERT_EQ(expected,*actual);
            ++actual;
        }
    }

}
TEST_SUITE(computeSSRight) {

    TEST_CASE(trivial_with_all_dimensions_1) {
        OverlapBoundary<Right> const new_boundary(
            contractVSRight(
                 OverlapBoundary<Right>::trivial
                ,OverlapSite<Right>::trivial
                ,StateSite<Right>::trivial
            )
        );
        EXPECT_EQ_VAL(new_boundary.overlapDimension(),1u);
        EXPECT_EQ_VAL(new_boundary.stateDimension(),1u);
        EXPECT_TRUE(equal(list_of(complex<double>(1)),new_boundary));
    }
    TEST_CASE(trivial_with_physical_dimension_2) {
        OverlapSite<Right> const overlap_site
            (RightDimension(1)
            ,LeftDimension(1)
            ,fillWithRange(list_of(5)(-1)
            ));
        StateSite<Right> const state_site
            (LeftDimension(1)
            ,RightDimension(1)
            ,fillWithRange(list_of(1)(1)
            ));
        OverlapBoundary<Right> const new_boundary(
            contractVSRight(
                 OverlapBoundary<Right>::trivial
                ,overlap_site
                ,state_site
            )
        );
        EXPECT_EQ_VAL(new_boundary.overlapDimension(),1u);
        EXPECT_EQ_VAL(new_boundary.stateDimension(),1u);
        EXPECT_TRUE(equal(list_of(complex<double>(4)),new_boundary));
    }
    TEST_CASE(non_trivial) {
        OverlapBoundary<Right> const boundary
            (OverlapDimension(2)
            ,fillWithRange(list_of
                (c(2,0))(c(1,0))
                (c(3,0))(c(1,1))
                (c(1,2))(c(0,6))
                (c(0,4))(c(0,5))
                (c(1,1))(c(2,1))
            ));
        OverlapSite<Right> const overlap_site
            (RightDimension(2)
            ,LeftDimension(3)
            ,fillWithRange(list_of
                (c( 1,0))(c( 0,-1))(c(0, 1))
                (c( 1,0))(c( 1, 0))(c(0,-1))
            ));
        StateSite<Right> const state_site
            (LeftDimension(2)
            ,RightDimension(5)
            ,fillWithRange(list_of
                (c(1,0))(c(1,0))(c( 1,0))(c(-1, 0))(c(-1,0))
                (c(2,0))(c(0,2))(c(-2,0))(c( 1,-2))(c( 2,0))
            ));
        OverlapBoundary<Right> const actual_boundary(
            contractVSRight(
                 boundary
                ,overlap_site
                ,state_site
            )
        );
        ASSERT_EQ_VAL(actual_boundary.overlapDimension(),3u);
        ASSERT_EQ_VAL(actual_boundary.stateDimension(),2u);
        complex<double> const expected_boundary[] =
            {c(5,-2),c(-3,-4),c(4,5)
            ,c(26,5),c(22,-15),c(-11,-2)
            }
        ;
        complex<double> const *actual = actual_boundary;
        BOOST_FOREACH(complex<double> expected, expected_boundary) {
            ASSERT_EQ(expected,*actual);
            ++actual;
        }
    }

}
TEST_CASE(consistent) {

    RNG random;

    REPEAT(10) {

        unsigned int const
             left_operator_dimension = random
            ,left_state_dimension = random
            ,physical_dimension = random
            ,right_operator_dimension = random
            ,right_state_dimension = random
            ,number_of_matrices = random
            ;
        ExpectationBoundary<Left> const left_boundary
            (OperatorDimension(left_operator_dimension)
            ,StateDimension(left_state_dimension)
            ,fillWithGenerator(random.randomComplexDouble)
            );
        StateSite<Middle> const state_site
            (PhysicalDimension(physical_dimension)
            ,LeftDimension(left_state_dimension)
            ,RightDimension(right_state_dimension)
            ,fillWithGenerator(random.randomComplexDouble)
            );
        StateSite<Left> const left_state_site(copyFrom(state_site));
        StateSite<Right> const right_state_site(copyFrom(state_site));
        OperatorSite const operator_site
            (number_of_matrices
            ,PhysicalDimension(physical_dimension)
            ,LeftDimension(left_operator_dimension)
            ,RightDimension(right_operator_dimension)
            ,fillWithGenerator(random.generateRandomIndices(
                 LeftDimension(left_operator_dimension)
                ,RightDimension(right_operator_dimension)
             ))
            ,fillWithGenerator(random.randomComplexDouble)
            );
        ExpectationBoundary<Right> const right_boundary
            (OperatorDimension(right_operator_dimension)
            ,StateDimension(right_state_dimension)
            ,fillWithGenerator(random.randomComplexDouble)
            );
        ExpectationBoundary<Left> const new_left_boundary(
            contract<Left>::SOS(
                 left_boundary
                ,left_state_site
                ,operator_site
            )
        );
        ExpectationBoundary<Right> const new_right_boundary(
            contract<Right>::SOS(
                 right_boundary
                ,right_state_site
                ,operator_site
            )
        );

        complex<double> const
             result_from_computeExpectationValue =
                computeExpectationValueAtSite(
                     left_boundary
                    ,state_site
                    ,operator_site
                    ,right_boundary
                )
            ,result_from_contractSOSLeft =
                contractExpectationBoundaries(
                     new_left_boundary
                    ,right_boundary
                )
            ,result_from_contractSOSRight =
                contractExpectationBoundaries(
                     left_boundary
                    ,new_right_boundary
                )
            ;
        ASSERT_NEAR_REL(result_from_computeExpectationValue,result_from_contractSOSLeft,1e-10);
        ASSERT_NEAR_REL(result_from_computeExpectationValue,result_from_contractSOSRight,1e-10);
    }
}

}
