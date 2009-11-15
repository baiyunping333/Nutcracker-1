-- @+leo-ver=4-thin
-- @+node:gcross.20091111171052.1608:@thin test.hs
-- @@language Haskell

-- @<< Language extensions >>
-- @+node:gcross.20091113142219.2512:<< Language extensions >>
{-# LANGUAGE ScopedTypeVariables #-}
-- @-node:gcross.20091113142219.2512:<< Language extensions >>
-- @nl

-- @<< Import needed modules >>
-- @+node:gcross.20091111171052.1610:<< Import needed modules >>
import Control.Arrow
import Control.Exception
import Control.Monad

import Data.Array.Storable
import Data.Array.Unboxed
import Data.Complex
import Data.Int

import Debug.Trace

import Foreign.Marshal.Utils
import Foreign.Ptr
import Foreign.Storable

import Test.HUnit
import Test.Framework
import Test.Framework.Providers.HUnit
import Test.Framework.Providers.QuickCheck2
import Test.QuickCheck

import Text.Printf

import System.IO.Unsafe

import VMPS.EnergyMinimizationChain
import VMPS.Miscellaneous
import VMPS.Tensors
import VMPS.Wrappers
-- @-node:gcross.20091111171052.1610:<< Import needed modules >>
-- @nl

-- @+others
-- @+node:gcross.20091113142219.2499:Generators
-- @+node:gcross.20091113142219.2500:UnderTenInt
newtype UnderTenInt = UTI Int deriving (Show,Eq)
instance Arbitrary UnderTenInt where
    arbitrary = choose (1,10) >>= return.UTI
-- @-node:gcross.20091113142219.2500:UnderTenInt
-- @+node:gcross.20091113142219.2514:NumberOfSitesInt
newtype NumberOfSitesInt = NOSI Int deriving (Show,Eq)
instance Arbitrary NumberOfSitesInt where
    arbitrary = choose (20,1000) >>= return.NOSI
-- @-node:gcross.20091113142219.2514:NumberOfSitesInt
-- @+node:gcross.20091113142219.2502:BandwidthInt
newtype BandwidthInt = BI Int deriving (Show,Eq)
instance Arbitrary BandwidthInt where
    arbitrary = choose (1,1000) >>= return.BI
-- @-node:gcross.20091113142219.2502:BandwidthInt
-- @+node:gcross.20091113142219.2504:PhysicalDimensionInt
newtype PhysicalDimensionInt = PDI Int deriving (Show,Eq)
instance Arbitrary PhysicalDimensionInt where
    arbitrary = choose (2,4) >>= return.PDI
-- @-node:gcross.20091113142219.2504:PhysicalDimensionInt
-- @-node:gcross.20091113142219.2499:Generators
-- @+node:gcross.20091113142219.2508:Helpers
-- @+node:gcross.20091113142219.2509:echo
echo x = trace (show x) x
-- @-node:gcross.20091113142219.2509:echo
-- @+node:gcross.20091114174920.1734:assertAlmostEqual
assertAlmostEqual :: (Show a, AlmostEq a) => String -> a -> a -> Assertion
assertAlmostEqual message x y
    | x ~= y     = return ()
    | otherwise  = assertFailure $ message ++ " (" ++ show x ++ " /~ " ++ show y ++ ")"
-- @-node:gcross.20091114174920.1734:assertAlmostEqual
-- @+node:gcross.20091114174920.1737:createEnergyInvarianceTest
createEnergyInvarianceTest number_of_sites bandwidth_dimension = 
        let operator_site_tensors = replicate number_of_sites $ makeOperatorSiteTensorFromPaulis 1 1 [((1,1),I)]
        in do
            chain <- generateRandomizedChain operator_site_tensors 2 2
            let chains_going_right =
                    take number_of_sites
                    .
                    iterate moveActiveSiteRightByOneBead
                    $
                    chain
                chains_going_left =
                    take number_of_sites
                    .
                    iterate moveActiveSiteLeftByOneBead
                    .
                    last
                    $
                    chains_going_right
                correct_energy = chainEnergy chain
            forM_ (zip [1..] . map chainEnergy $ chains_going_right) $ \(site_number::Int,energy) ->
                assertAlmostEqual
                    (printf "Did the energy change after moving right to site %i?" site_number)
                    correct_energy energy
            forM_ (zip [number_of_sites,number_of_sites-1..] . map chainEnergy $ chains_going_left) $ \(site_number,energy) ->
                assertAlmostEqual
                    (printf "Did the energy change after moving left to site %i?" site_number)
                    correct_energy energy
-- @-node:gcross.20091114174920.1737:createEnergyInvarianceTest
-- @-node:gcross.20091113142219.2508:Helpers
-- @-others

main = defaultMain
    -- @    << Tests >>
    -- @+node:gcross.20091111171052.1640:<< Tests >>
    -- @+others
    -- @+node:gcross.20091113142219.1701:VMPS.EnergyMinimizationChain
    [testGroup "VMPS.EnergyMinimizationChain"
        -- @    @+others
        -- @+node:gcross.20091113142219.1853:computeBandwidthDimensionSequence
        [testGroup "computeBandwidthDimensionSequence"
            -- @    @+others
            -- @+node:gcross.20091113142219.2505:gets there eventually
            [testProperty "gets there eventually" $
                \(PDI physical_dimension) (BI bandwith_dimension) (NOSI number_of_sites) ->
                    maximum (computeBandwidthDimensionSequence number_of_sites physical_dimension bandwith_dimension) == bandwith_dimension
            -- @-node:gcross.20091113142219.2505:gets there eventually
            -- @+node:gcross.20091113142219.2507:has the right number of entries
            ,testProperty "has the right number of entries" $
                \(PDI physical_dimension) (BI bandwith_dimension) (NOSI number_of_sites) ->
                    length (computeBandwidthDimensionSequence number_of_sites physical_dimension bandwith_dimension) == number_of_sites + 1
            -- @-node:gcross.20091113142219.2507:has the right number of entries
            -- @-others
            ]
        -- @-node:gcross.20091113142219.1853:computeBandwidthDimensionSequence
        -- @+node:gcross.20091113142219.2532:computeBandwidthDimensionsAtAllSites
        ,testGroup "computeBandwidthDimensionsAtAllSites"
            -- @    @+others
            -- @+node:gcross.20091113142219.2534:has the right number of entries
            [testProperty "has the right number of entries" $
                \(PDI physical_dimension) (BI bandwith_dimension) (NOSI number_of_sites) ->
                    length (computeBandwidthDimensionsAtAllSites number_of_sites physical_dimension bandwith_dimension) == number_of_sites
            -- @-node:gcross.20091113142219.2534:has the right number of entries
            -- @+node:gcross.20091113142219.2516:doesn't grow too fast from the left
            ,testProperty "doesn't grow too fast from the left" $
                \(PDI physical_dimension) (BI bandwith_dimension) (NOSI number_of_sites) ->
                    all (\(x,y) -> y <= physical_dimension * x) $
                        computeBandwidthDimensionsAtAllSites number_of_sites physical_dimension bandwith_dimension
            -- @-node:gcross.20091113142219.2516:doesn't grow too fast from the left
            -- @+node:gcross.20091113142219.2518:doesn't grow too fast from the right
            ,testProperty "doesn't grow too fast from the right" $
                \(PDI physical_dimension) (BI bandwith_dimension) (NOSI number_of_sites) ->
                    all (\(x,y) -> x <= physical_dimension * y) $
                        computeBandwidthDimensionsAtAllSites number_of_sites physical_dimension bandwith_dimension
            -- @-node:gcross.20091113142219.2518:doesn't grow too fast from the right
            -- @+node:gcross.20091113142219.2511:complains if too large
            ,testProperty "complains if too large" $
                \(PDI physical_dimension) (UTI number_of_sites) -> unsafePerformIO $
                    Control.Exception.catch
                        ((evaluate
                            .
                            computeBandwidthDimensionSequence number_of_sites physical_dimension
                            .
                            product
                            .
                            replicate (number_of_sites `div` 2 + 1)
                            $
                            physical_dimension)
                        >>=
                        putStrLn . show
                        >>
                        return False)
                        (\(_ :: ErrorCall) -> return True)
            -- @-node:gcross.20091113142219.2511:complains if too large
            -- @-others
            ]
        -- @-node:gcross.20091113142219.2532:computeBandwidthDimensionsAtAllSites
        -- @+node:gcross.20091114174920.1728:Chain movement
        ,testGroup "Chain energy invariant under movement"
            [testCase (show number_of_sites ++ " sites") $ 
                createEnergyInvarianceTest number_of_sites bandwidth_dimension
            | (number_of_sites,bandwidth_dimension) <-
                [( 2, 2)
                ,( 3, 2)
                ,( 4, 4)
                ,( 5, 4)
                ,(10,16)
            ]]
        -- @-node:gcross.20091114174920.1728:Chain movement
        -- @-others
        ]
    -- @-node:gcross.20091113142219.1701:VMPS.EnergyMinimizationChain
    -- @+node:gcross.20091113142219.1700:VMPS.Wrapper
    ,testGroup "VMPS.Wrapper"
        -- @    @+others
        -- @+node:gcross.20091111171052.1649:computeExpectation
        [testGroup "computeExpectation"
            -- @    @+others
            -- @+node:gcross.20091111171052.1650:trivial, all dimensions 1
            [testCase "trivial, all dimensions 1" $
                let left_boundary = trivial_left_boundary
                    right_boundary = trivial_right_boundary
                    state_site_tensor = UnnormalizedStateSiteTensor $ StateSiteTensor 1 1 1 trivial_complex_array
                    operator_indices = unsafePerformIO $ newArray ((1,1),(1,2)) 1
                    operator_matrices = unsafePerformIO $ newListArray ((1,1,1),(1,1,1)) [1.0 + 0.0]
                    operator_site_tensor = OperatorSiteTensor 1 1 1 1 operator_indices operator_matrices
                    expectation = computeExpectation left_boundary state_site_tensor operator_site_tensor right_boundary
                in assertEqual "is the correct result returned?" (1 :+ 0) expectation
            -- @-node:gcross.20091111171052.1650:trivial, all dimensions 1
            -- @+node:gcross.20091113142219.1683:trivial, all dimensions 1, imaginary
            ,testCase "trivial, all dimensions 1, imaginary" $
                let left_boundary = trivial_left_boundary
                    right_boundary = trivial_right_boundary
                    state_site_tensor = UnnormalizedStateSiteTensor $ StateSiteTensor 1 1 1 trivial_complex_array
                    operator_indices = unsafePerformIO $ newArray ((1,1),(1,2)) 1
                    operator_matrices = unsafePerformIO $ newListArray ((1,1,1),(1,1,1)) [0 :+ 1]
                    operator_site_tensor = OperatorSiteTensor 1 1 1 1 operator_indices operator_matrices
                    expectation = computeExpectation left_boundary state_site_tensor operator_site_tensor right_boundary
                in assertEqual "is the correct result returned?" (0 :+ 1) expectation
            -- @-node:gcross.20091113142219.1683:trivial, all dimensions 1, imaginary
            -- @+node:gcross.20091111171052.1655:trivial, d =2
            ,testCase "trivial, d = 2" $
                assertEqual "are the correct results returned for all calls?"
                [1,1,-1,-1]
                $
                map (\state ->
                    let left_boundary = trivial_left_boundary
                        right_boundary = trivial_right_boundary
                        state_site_tensor = UnnormalizedStateSiteTensor . StateSiteTensor 1 1 2 . complexArrayFromList $ state
                        operator_site_tensor = makeOperatorSiteTensorFromPaulis 1 1 [((1,1),Z)]
                    in computeExpectation left_boundary state_site_tensor operator_site_tensor right_boundary
                ) [[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]]
            -- @-node:gcross.20091111171052.1655:trivial, d =2
            -- @-others
            ]
        -- @-node:gcross.20091111171052.1649:computeExpectation
        -- @+node:gcross.20091111171052.1659:computeOptimalSiteStateTensor
        ,testGroup "computeOptimalSiteStateTensor"
            -- @    @+others
            -- @+node:gcross.20091111171052.1661:trivial, d = 4
            [testCase "trivial, d = 4" $
                let left_boundary = trivial_left_boundary
                    right_boundary = trivial_right_boundary
                    state_site_tensor = UnnormalizedStateSiteTensor . StateSiteTensor 1 1 4 . complexArrayFromList $ replicate (4*2) 1
                    operator_indices = unsafePerformIO $ newArray ((1,1),(1,2)) 1
                    operator_matrices = unsafePerformIO $ newListArray ((1,1,1),(1,4,4))
                        [ 1, 0, 0, 0
                        , 0, 1, 0, 0
                        , 0, 0, 1, 0
                        , 0, 0, 0,-1
                        ]
                    operator_site_tensor = OperatorSiteTensor 1 1 4 1 operator_indices operator_matrices
                    (_, eigenvalue, (UnnormalizedStateSiteTensor (StateSiteTensor bl br d (ComplexArray actual_array)))) =
                        computeOptimalSiteStateTensor
                            left_boundary
                            state_site_tensor
                            operator_site_tensor
                            right_boundary
                            SR
                            0
                            10000
                    components = elems actual_array
                in do
                    assertEqual "is the new left bandwidth dimension the same?" (leftBandwidthOfState state_site_tensor) bl
                    assertEqual "is the new right bandwidth dimension the same?" (rightBandwidthOfState state_site_tensor) br
                    assertEqual "is the new physical dimension the same?" (physicalDimensionOfState state_site_tensor) d
                    assertBool "are all but the last component of the state zero?" (take 6 components ~= replicate 6 0)
                    assertBool "are either of the last components non-zero?" (any (/~ 0) . drop 6 $ components)
                    assertBool "is the eigenvalue correct?" (eigenvalue ~= (-1))
            -- @-node:gcross.20091111171052.1661:trivial, d = 4
            -- @-others
            ]
        -- @-node:gcross.20091111171052.1659:computeOptimalSiteStateTensor
        -- @+node:gcross.20091112145455.1629:contractSOSLeft
        ,testGroup "contractSOSLeft"
            -- @    @+others
            -- @+node:gcross.20091112145455.1630:trivial, all dimensions 1
            [testCase "trivial, all dimensions 1" $
                let left_boundary = trivial_left_boundary
                    state_site_tensor = LeftAbsorptionNormalizedStateSiteTensor $ StateSiteTensor 1 1 1 trivial_complex_array
                    operator_indices = unsafePerformIO $ newArray ((1,1),(1,2)) 1
                    operator_matrices = unsafePerformIO $ newListArray ((1,1,1),(1,1,1)) [1]
                    operator_site_tensor = OperatorSiteTensor 1 1 1 1 operator_indices operator_matrices
                    LeftBoundaryTensor (BoundaryTensor br cr (ComplexArray actual_array)) = contractSOSLeft left_boundary state_site_tensor operator_site_tensor
                    components = elems actual_array
                in do
                    assertEqual "is the new state bandwidth dimension correct?" (rightBandwidthOfState state_site_tensor) br
                    assertEqual "is the new operator bandwidth dimension correct?" (operatorRightBandwidth operator_site_tensor) cr
                    assertBool "are the components correct?" (components ~= [1.0,0.0])
            -- @-node:gcross.20091112145455.1630:trivial, all dimensions 1
            -- @+node:gcross.20091112145455.1633:trivial, c = 2
            ,testCase "trivial, c = 2" $
                let left_boundary = LeftBoundaryTensor . BoundaryTensor 1 1 . ComplexArray . listArray (1,2) $ [1.0,0.0]
                    state_site_tensor = LeftAbsorptionNormalizedStateSiteTensor $ StateSiteTensor 1 1 1 trivial_complex_array
                    operator_indices = unsafePerformIO $ newListArray ((1,1),(1,2)) [1,2]
                    operator_matrices = unsafePerformIO $ newListArray ((1,1,1),(1,1,1)) [0 :+ 1]
                    operator_site_tensor = OperatorSiteTensor 1 2 1 1 operator_indices operator_matrices
                    LeftBoundaryTensor (BoundaryTensor br cr (ComplexArray actual_array)) = contractSOSLeft left_boundary state_site_tensor operator_site_tensor
                    components = elems actual_array
                in do
                    assertEqual "is the new state bandwidth dimension correct?" (rightBandwidthOfState state_site_tensor) br
                    assertEqual "is the new operator bandwidth dimension correct?" (operatorRightBandwidth operator_site_tensor) cr
                    assertBool "are the components correct?" (components ~= [0.0,0.0,0.0,1.0])
            -- @-node:gcross.20091112145455.1633:trivial, c = 2
            -- @-others
            ]
        -- @-node:gcross.20091112145455.1629:contractSOSLeft
        -- @+node:gcross.20091112145455.1641:contractSOSRight
        ,testGroup "contractSOSRight"
            -- @    @+others
            -- @+node:gcross.20091112145455.1642:trivial, all dimensions 1
            [testCase "trivial, all dimensions 1" $
                let right_boundary = trivial_right_boundary
                    state_site_tensor = RightAbsorptionNormalizedStateSiteTensor $ StateSiteTensor 1 1 1 trivial_complex_array
                    operator_indices = unsafePerformIO $ newArray ((1,1),(1,2)) 1
                    operator_matrices = unsafePerformIO $ newListArray ((1,1,1),(1,1,1)) [1]
                    operator_site_tensor = OperatorSiteTensor 1 1 1 1 operator_indices operator_matrices
                    RightBoundaryTensor (BoundaryTensor br cr (ComplexArray actual_array)) = contractSOSRight right_boundary state_site_tensor operator_site_tensor
                    components = elems actual_array
                in do
                    assertEqual "is the new state bandwidth dimension correct?" (leftBandwidthOfState state_site_tensor) br
                    assertEqual "is the new operator bandwidth dimension correct?" (operatorLeftBandwidth operator_site_tensor) cr
                    assertBool "are the components correct?" (components ~= [1.0,0.0])
            -- @-node:gcross.20091112145455.1642:trivial, all dimensions 1
            -- @+node:gcross.20091112145455.1643:trivial, c = 2
            ,testCase "trivial, c = 2" $
                let right_boundary = RightBoundaryTensor . BoundaryTensor 1 1 . ComplexArray . listArray (1,2) $ [1.0,0.0]
                    state_site_tensor = RightAbsorptionNormalizedStateSiteTensor $ StateSiteTensor 1 1 1 trivial_complex_array
                    operator_indices = unsafePerformIO $ newListArray ((1,1),(1,2)) [2,1]
                    operator_matrices = unsafePerformIO $ newListArray ((1,1,1),(1,1,1)) [0 :+ 1]
                    operator_site_tensor = OperatorSiteTensor 2 1 1 1 operator_indices operator_matrices
                    RightBoundaryTensor (BoundaryTensor bl cl (ComplexArray actual_array)) = contractSOSRight right_boundary state_site_tensor operator_site_tensor
                    components = elems actual_array
                in do
                    assertEqual "is the new state bandwidth dimension correct?" (leftBandwidthOfState state_site_tensor) bl
                    assertEqual "is the new operator bandwidth dimension correct?" (operatorLeftBandwidth operator_site_tensor) cl
                    assertBool "are the components correct?" (components ~= [0.0,0.0,0.0,1.0])
            -- @-node:gcross.20091112145455.1643:trivial, c = 2
            -- @-others
            ]
        -- @-node:gcross.20091112145455.1641:contractSOSRight
        -- @+node:gcross.20091112145455.1660:generateRandomizedStateTensor
        ,testGroup "generateRandomizedUnnormalizedSiteStateTensor"
            -- @    @+others
            -- @+node:gcross.20091112145455.1667:unnormalized
            [testGroup "unnormalized"
                -- @    @+others
                -- @+node:gcross.20091112145455.1661:selected dimensions
                [testCase "bl = 1, br = 2, d = 3" $ do
                    state_site_tensor <- generateRandomizedStateSiteTensor 3 1 2 :: IO (UnnormalizedStateSiteTensor)
                    assertEqual "is the left bandwidth dimension correct?" 1 (leftBandwidthOfState state_site_tensor)
                    assertEqual "is the right bandwidth dimension correct?" 2 (rightBandwidthOfState state_site_tensor)
                    assertEqual "is the physical bandwidth dimension correct?" 3 (physicalDimensionOfState state_site_tensor)
                -- @-node:gcross.20091112145455.1661:selected dimensions
                -- @-others
                ]
            -- @-node:gcross.20091112145455.1667:unnormalized
            -- @+node:gcross.20091112145455.1665:normalized
            ,testGroup "normalized"
                -- @    @+others
                -- @+node:gcross.20091112145455.1666:selected dimensions
                [testCase "bl = 1, br = 4, d = 8" $ do
                    state_site_tensor <- generateRandomizedStateSiteTensor 8 1 4 :: IO (RightAbsorptionNormalizedStateSiteTensor)
                    assertEqual "is the left bandwidth dimension correct?" 1 (leftBandwidthOfState state_site_tensor)
                    assertEqual "is the right bandwidth dimension correct?" 4 (rightBandwidthOfState state_site_tensor)
                    assertEqual "is the physical bandwidth dimension correct?" 8 (physicalDimensionOfState state_site_tensor)
                    let normalization =
                            sum
                            .
                            map ((** 2) . magnitude)
                            .
                            toListOfComplexNumbers
                            .
                            stateData
                            .
                            unwrapRightAbsorptionNormalizedStateSiteTensor
                            $
                            state_site_tensor
                    assertBool "is the state site tensor properly normalized?" (1 ~= normalization)
                -- @-node:gcross.20091112145455.1666:selected dimensions
                -- @-others
                ]
            -- @-node:gcross.20091112145455.1665:normalized
            -- @-others
            ]
        -- @-node:gcross.20091112145455.1660:generateRandomizedStateTensor
        -- @-others
        ]
    -- @-node:gcross.20091113142219.1700:VMPS.Wrapper
    -- @-others
    -- @-node:gcross.20091111171052.1640:<< Tests >>
    -- @nl
    ]
-- @-node:gcross.20091111171052.1608:@thin test.hs
-- @-leo