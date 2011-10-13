#@+leo-ver=5-thin
#@+node:gcross.20111009193003.5253: * @file grid.py
#@+<< Imports >>
#@+node:gcross.20111009193003.5254: ** << Imports >>
from numpy import array, dot, identity, product, tensordot
from numpy.linalg import cond, svd

from flatland.tensors import StateCenterSite, StateCornerSite, StateSideSite
from flatland.utils import crand, multiplyTensorByMatrixAtIndex
#@-<< Imports >>

#@+others
#@+node:gcross.20111009193003.5256: ** Classes
#@+node:gcross.20111009193003.5257: *3* Grid
class Grid:
    #@+others
    #@+node:gcross.20111009193003.5258: *4* __init__
    def __init__(self,physical_dimension):
        self.physical_dimension = physical_dimension
        self.bandwidth_dimensions = [1,1,1,1]
        self.sides = [StateSideSite.trivial()]*4
        self.corners = [StateCornerSite.trivial()]*4
        self.center = \
            StateCenterSite(
                physical_dimension = physical_dimension,
                rightward_dimension = 1,
                upward_dimension = 1,
                leftward_dimension = 1,
                downward_dimension = 1,
            )
        self.center.data[:,0,0,0,0] = array([1] + [0]*(physical_dimension-1))
    #@+node:gcross.20111013165152.1229: *4* computeNormalization
    def computeNormalization(self):
        return \
            dot(
                dot(
                    self.center.data.reshape(self.physical_dimension,product(self.bandwidth_dimensions)),
                    self.computeNormalizationSubmatrix()
                ).ravel(),
                self.center.data.conj().ravel()
            )
    #@+node:gcross.20111013080525.1260: *4* computeNormalizationConditionNumber
    def computeNormalizationConditionNumber(self):
        return cond(self.computeNormalizationSubmatrix())
    #@+node:gcross.20111009193003.5260: *4* computeNormalizationMatrix
    def computeNormalizationMatrix(self):
        submatrix = self.computeNormalizationSubmatrix()
        return (
             tensordot(
                identity(self.physical_dimension),
                submatrix,
                ((),()),
             )
            .transpose(0,2,1,3)
            .reshape(*((self.physical_dimension*submatrix.shape[0],)*2))
        )
    #@+node:gcross.20111013080525.1259: *4* computeNormalizationSubmatrix
    def computeNormalizationSubmatrix(self):
        side_boundaries = [side.formBoundary().absorbCounterClockwiseCornerBoundary(corner.formBoundary()) for (side,corner) in zip(self.sides,self.corners)]
        final_dimension = product([side.inward_dimension for side in self.sides])
        return (
             tensordot(
                side_boundaries[0].absorbCounterClockwiseSideBoundary(side_boundaries[1]).data,
                side_boundaries[2].absorbCounterClockwiseSideBoundary(side_boundaries[3]).data,
                ((1,0),(0,1))
             )
            .transpose(0,2,1,3)
            .reshape(final_dimension,final_dimension)
        )
    #@+node:gcross.20111013080525.1244: *4* contract
    def contract(self,direction):
        self.sides[direction] = self.sides[direction].absorbCenterSite(self.center,direction)
        self.corners[direction] = self.corners[direction].absorbSideSiteAtCounterClockwise(self.sides[(direction+1)%4])
        self.corners[(direction-1)%4] = self.corners[(direction-1)%4].absorbSideSiteAtClockwise(self.sides[(direction-1)%4])
    #@+node:gcross.20111013080525.3958: *4* increaseBandwidthDimensionBy
    def increaseBandwidthDimensionBy(self,increment,direction):
        return self.increaseBandwidthDimensionTo(self.bandwidth_dimensions[direction]+increment,direction)
    #@+node:gcross.20111013080525.1264: *4* increaseBandwidthDimensionTo
    def increaseBandwidthDimensionTo(self,new_dimension,direction):
        old_dimension = self.bandwidth_dimensions[direction]
        if new_dimension < old_dimension:
            raise ValueError("new dimension ({}) must be at least the old dimension ({})".format(new_dimension,old_dimension))
        if new_dimension == old_dimension:
            return
        matrix, _, _ = svd(crand(new_dimension,old_dimension),full_matrices=False)
        matrix = matrix.transpose()
        self.center = StateCenterSite(multiplyTensorByMatrixAtIndex(self.center.data,matrix,1+direction))
        self.sides[direction] = StateSideSite(multiplyTensorByMatrixAtIndex(self.sides[direction].data,matrix.conj(),3))
        self.bandwidth_dimensions[direction] = new_dimension
    #@-others
#@-others

#@+<< Exports >>
#@+node:gcross.20111009193003.5255: ** << Exports >>
__all__ = [
    "Grid",
]
#@-<< Exports >>
#@-leo
