/*!
\file flat.hpp
\brief Classes and functions relating to flat representations of states
*/

#ifndef NUTCRACKER_FLATTENING_HPP
#define NUTCRACKER_FLATTENING_HPP

#include <boost/bind.hpp>
#include <boost/concept_check.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/concepts.hpp>
#include <boost/smart_ptr/scoped_array.hpp>

#include "nutcracker/core.hpp"
#include "nutcracker/tensors.hpp"

namespace Nutcracker {

using boost::adaptors::transformed;
using boost::scoped_array;
using boost::SinglePassRangeConcept;

//! \defgroup Flat Flat representations
//! @{

//! An intermediate result produced when constructing a flattened representation of a state.
/*!

This tensor represents one of the intermediate results of flattening a matrix product state.  It has two ranks:  the physical dimension which corresponds to the state space of the qubits that we have flattened so far, and a right dimension that connects this state vector fragment to the remaining qubits in the matrix product state.  When the matrix product state has been completely flattened the physical dimension will be equal to the dimension of the original Hilbert space and the right dimension will be equal to be one.

\note
See the documentation in BaseTensor for a description of the policy of how data ownership in tensors works.  (Short version: tensors own their data, which can be moved but not copied unless you explicitly ask for a copy to be made.)

\see BaseTensor
*/
class Fragment : public BaseTensor {
    private:

    BOOST_MOVABLE_BUT_NOT_COPYABLE(Fragment)
    //! \name Assignment
    //! @{

    protected:

    //! Moves the data (and dimensions) from \c other to \c this and invalidates \c other.
    void operator=(BOOST_RV_REF(Fragment) other) {
        BaseTensor::operator=(boost::move(static_cast<BaseTensor&>(other)));
        physical_dimension = copyAndReset(other.physical_dimension);
        right_dimension = copyAndReset(other.right_dimension);
    }

    //! @}
    //! \name Constructors
    //! @{

    public:

    //! Construct an invalid tensor (presumably into which you will eventually move data from elsewhere).
    Fragment()
      : physical_dimension(0u)
      , right_dimension(0u)
    {}

    //! Move the data (and dimensions) from \c other into \c this and invalidate \c other.
    Fragment(BOOST_RV_REF(Fragment) other)
      : BaseTensor(boost::move(static_cast<BaseTensor&>(other)))
      , physical_dimension(copyAndReset(other.physical_dimension))
      , right_dimension(copyAndReset(other.right_dimension))
    { }

    //! Initialize the dimensions with the given values and allocate memory for an array of the appropriate size.
    Fragment(
          PhysicalDimension const physical_dimension
        , RightDimension const right_dimension
        , unsigned int const size
    ) : BaseTensor(size)
      , physical_dimension(*physical_dimension)
      , right_dimension(*right_dimension)
    { }

    //! Constructs a tensor using data supplied from a generator.
    /*! \see BaseTensor(unsigned int const size, FillWithGenerator<G> const generator) */
    template<typename G> Fragment(
          PhysicalDimension const physical_dimension
        , RightDimension const right_dimension
        , unsigned int const size
        , FillWithGenerator<G> const generator
    ) : BaseTensor(size,generator)
      , physical_dimension(*physical_dimension)
      , right_dimension(*right_dimension)
    { }

    //! Constructs a tensor using data supplied from a range.
    /*!
    \note The physical dimension is inferred automatically from the size of the range and the left and right dimensions.
    \see BaseTensor(FillWithRange<Range> const init)
    */
    template<typename Range> Fragment(
          PhysicalDimension const physical_dimension
        , RightDimension const right_dimension
        , FillWithRange<Range> const init
    ) : BaseTensor(init)
      , physical_dimension(*physical_dimension)
      , right_dimension(*right_dimension)
    { }

    //! Sets all dimensions to 1, and then allocates an array of size one and fills it with the value 1.
    Fragment(
          MakeTrivial const make_trivial
    ) : BaseTensor(make_trivial)
      , physical_dimension(1)
      , right_dimension(1)
    { }

    //! @}
    //! \name Dimension information
    //! @{

    public:

    //! Returns the physical dimension of this tensor.
    unsigned int physicalDimension() const { return physical_dimension; }
    //! Returns the physical dimension of this tensor wrapped in an instance of PhysicalDimension.
    PhysicalDimension physicalDimension(AsDimension const _) const { return PhysicalDimension(physical_dimension); }

    //! Returns the right dimension of this tensor.
    unsigned int rightDimension() const { return right_dimension; }
    //! Returns the right dimension of this tensor wrapped in an instance of RightDimension.
    RightDimension rightDimension(AsDimension const _) const { return RightDimension(right_dimension); }


    //! @}
    private:

    //! The physical dimension of the site (which corresponds to the state space of the qubits that have been flattened).
    unsigned int physical_dimension;

    //! The right dimension of this site.
    unsigned int right_dimension;
    public:

    //! Connects this tensor's right dimension of to the left dimension of \c state_site.
    /*! \see TensorConnectors */
    unsigned int operator|(StateSiteAny const& state_site) const {
        return connectDimensions(
             "fragment right"
            ,rightDimension()
            ,"state site left"
            ,state_site.leftDimension()
        );
    }
};
//! An intermediate result of constructing a flat state vector from a matrix product state.
/*!
\image html state_vector_fragment_tensor.png
\image latex state_vector_fragment_tensor.eps

This tensor represents one of the intermediate results of converting a matrix product state into the flat vector representation of a state.  It has two ranks:  the physical dimension which corresponds to the state space of the qubits that we have flattened so far, and a right dimension that connects this state vector fragment to the remaining qubits in the matrix product state.  When the matrix product state has been completely flattened the physical dimension will be equal to the dimension of the original Hilbert space and the right dimension will be equal to be one, and at that point this object can be casted to a Vector without raising an error.

\note
See the documentation in BaseTensor for a description of the policy of how data ownership in tensors works.  (Short version: tensors own their data, which can be moved but not copied unless you explicitly ask for a copy to be made.)

\see BaseTensor
*/
class StateVectorFragment : public Fragment {
    private:

    BOOST_MOVABLE_BUT_NOT_COPYABLE(StateVectorFragment)
    //! \name Assignment
    //! @{

    public:

    //! Moves the data (and dimensions) from \c other to \c this and invalidates \c other.
    StateVectorFragment& operator=(BOOST_RV_REF(StateVectorFragment) other) {
        if(this == &other) return *this;
        Fragment::operator=(boost::move(static_cast<Fragment&>(other)));
        return *this;
    }

    //! @}
    //! \name Constructors
    //! @{

    public:

    //! Construct an invalid tensor (presumably into which you will eventually move data from elsewhere).
    StateVectorFragment() {}

    //! Move the data (and dimensions) from \c other into \c this and invalidate \c other.
    StateVectorFragment(BOOST_RV_REF(StateVectorFragment) other)
      : Fragment(boost::move(static_cast<Fragment&>(other)))
    { }

    //! Initialize the dimensions with the given values and allocate memory for an array of the appropriate size.
    StateVectorFragment(
          PhysicalDimension const physical_dimension
        , RightDimension const right_dimension
    ) : Fragment(physical_dimension,right_dimension,(*physical_dimension)*(*right_dimension))
    { }

    //! Constructs a tensor using data supplied from a generator.
    /*! \see BaseTensor( unsigned int const size, FillWithGenerator<G> const generator) */
    template<typename G> StateVectorFragment(
          PhysicalDimension const physical_dimension
        , RightDimension const right_dimension
        , FillWithGenerator<G> const generator
    ) : Fragment(physical_dimension,right_dimension,(*physical_dimension)*(*right_dimension),generator)
    { }

    //! Constructs a tensor using data supplied from a range.
    /*!
    \note The physical dimension is inferred automatically from the size of the range and the left and right dimensions.
    \see BaseTensor(FillWithRange<Range> const init)
    */
    template<typename Range> StateVectorFragment(
          PhysicalDimension const physical_dimension
        , FillWithRange<Range> const init
    ) : Fragment(physical_dimension,init.size()/(*physical_dimension),init)
    { }

    //! Sets all dimensions to 1, and then allocates an array of size one and fills it with the value 1.
    StateVectorFragment(
          MakeTrivial const make_trivial
    ) : Fragment(make_trivial)
    { }

    //! @}
    //! \name Casts

    //! @{

    public:

    //! Casts this tensor to a flat Vector as long as the right dimension is 1.
    operator Vector() const {
        assert(rightDimension() == 1);
        Vector v(size());
        copy(*this,v.data().begin());
        return v;
    }

    //! @}
    public:

    //! The trivial state vector fragment tensor with all dimensions one and containing the single value 1.
    static StateVectorFragment const trivial;
};
StateVectorFragment extendStateVectorFragment(
      StateVectorFragment const& old_fragment
    , StateSiteAny const& state_site
);

//! Computes a flat state vector from a matrx product state represented by a list of state site tensors.
/*!

\note Since this function necesarily has exponential running time, if you only need a few components of this vector then you should consider computeStateVectorComponent().

\tparam StateSiteRange the type of the list, which must satisfy the Boost single pass range concept with the value type \c StateSiteAny \c const.
\param state_sites the list of state site tensors
\returns the state vector
*/
template<typename StateSiteRange> Vector computeStateVector(StateSiteRange const& state_sites) {
    BOOST_CONCEPT_ASSERT((SinglePassRangeConcept<StateSiteRange const>));
    StateVectorFragment current_fragment(make_trivial);
    BOOST_FOREACH(StateSiteAny const& state_site, state_sites) {
        StateVectorFragment next_fragment =
            extendStateVectorFragment(
                 current_fragment
                ,state_site
            );
        current_fragment = boost::move(next_fragment);
    }
    return current_fragment;
}
//! Computes the value of a single component of a quantum state given the list of observed qudit values
/*!
\note If you need the entire state vector then it is more efficient to call computeStateVector().

\tparam StateSiteRange the type of the list of state site tensors, which must satisfy the Boost single pass range concept with the value type \c StateSiteAny \c const.
\param state_sites the list of state site tensors
\param observed_values the observation value of each qudit in the matrix product state, which together specify the component of the quantum state whose value is to be computed
\returns the amplitude of the requested component
*/
template<typename StateSiteRange> complex<double> computeStateVectorComponent(StateSiteRange const& state_sites, vector<unsigned int> const& observed_values) {
    BOOST_CONCEPT_ASSERT((SinglePassRangeConcept<StateSiteRange const>));
    scoped_array<complex<double> > left_boundary(new complex<double>[1]);  left_boundary[0] = c(1,0);
    unsigned int left_dimension = 1;
    unsigned int i = 0;
    BOOST_FOREACH(StateSiteAny const& state_site, state_sites) {
        assert(state_site.leftDimension()==left_dimension);
        complex<double> const* const transition_matrix = state_site.transitionMatrixForObservation(observed_values[i]);
        assert(transition_matrix >= state_site.begin());
        assert(transition_matrix < state_site.end());
        unsigned int const right_dimension = state_site.rightDimension();
        scoped_array<complex<double> > new_left_boundary(new complex<double>[right_dimension]);
        zgemv(
            "N"
            ,right_dimension,left_dimension
            ,c(1,0)
            ,transition_matrix,right_dimension
            ,left_boundary.get(),1
            ,c(0,0)
            ,new_left_boundary.get(),1
        );
        left_dimension = right_dimension;
        left_boundary.swap(new_left_boundary);
        ++i;
    }
    assert(i == observed_values.size() && "observed_values vector is larger than the list of state sites");
    assert(left_dimension == 1);
    return left_boundary[0];
}

//! Computes the value of a single component of a quantum state given the index of the component in the flat representation.
/*!
\note If you need the entire state vector then it is more efficient to call computeStateVector().

\tparam StateSiteRange the type of the list of state site tensors, which must satisfy the Boost single pass range concept with the value type \c StateSiteAny \c const.
\param state_sites the list of state site tensors
\param component the index of the desired component (in the flat vector representation of the state)
\returns the amplitude of the requested component
*/
template<typename StateSiteRange> complex<double> computeStateVectorComponent(StateSiteRange const& state_sites, unsigned long long const component) {
    return computeStateVectorComponent(state_sites,flatIndexToTensorIndex(state_sites | transformed(bind(&StateSiteAny::physicalDimension,_1)),component));
}
//! Computes the number of components in the flat vector representation of the state.
/*!
\tparam StateSiteRange the type of the list, which must satisfy the Boost single pass range concept with the value type \c StateSiteAny \c const.
\param state_sites the list of state site tensors
\returns the number of components in the flat vector representation of the state
*/
template<typename StateSiteRange> unsigned long long computeStateVectorLength(StateSiteRange const& state_sites) {
    BOOST_CONCEPT_ASSERT((SinglePassRangeConcept<StateSiteRange const>));
    unsigned long long length = 1;
    BOOST_FOREACH(StateSiteAny const& state_site, state_sites) {
        length *= state_site.physicalDimension();
    }
    return length;
}
//! Extends a state vector fragement to include another state site tensor.
/*!
\image html extendStateVectorFragment.png
\image latex extendStateVectorFragment.eps

\param old_fragment the current state vector fragment (F')
\param state_site a state site tensor (S)
\returns a new state vector fragment that includes the given state site tensor (F')
*/
StateVectorFragment extendStateVectorFragment(
      StateVectorFragment const& old_fragment
    , StateSiteAny const& state_site
);
//! Converts an integral index of the flat representation of a tensor into the corresponding multi-index of the multi-dimensional representation of a tensor.
/*!
\note The conversion assumes that the tensor is stored in row-major order --- that is, an increment in the first entry of the multi index causes the greatest increase in the flat index and an increment in the last entry of the multi-index causes the least increase in the flast index.

\tparam DimensionRange the type of the list of the dimensions
\param dimensions the dimension of the tensor
\param flat_index the index into the flattened tensor
\return the multi-dimensional index referring to the same element in the original multi-dimensional tensor as \c flat_index had referred to in the flattened one-dimensional representation of that tensor.
*/
template<typename DimensionRange> vector<unsigned int> flatIndexToTensorIndex(DimensionRange const& dimensions, unsigned long long flat_index) {
    vector<unsigned int> tensor_index;
    tensor_index.reserve(dimensions.size());
    BOOST_FOREACH(unsigned int const dimension, dimensions | reversed) {
        tensor_index.push_back(flat_index % dimension);
        flat_index /= dimension;
    }
    assert(flat_index == 0);
    reverse(tensor_index);
    return boost::move(tensor_index);
}
//! Converts a multi-index of the multi-dimensional representation of a tensor into the corresponding integral index of the flat representation of a tensor.
/*!
\note The conversion assumes that the tensor is stored in row-major order --- that is, an increment in the first entry of the multi index causes the greatest increase in the flat index and an increment in the last entry of the multi-index causes the least increase in the flast index.

\tparam DimensionRange the type of the list of the dimensions
\param dimensions the dimension of the tensor
\param tensor_index the index into the flattened tensor
\return the flat index referring to the same element in the flattened tensor as \c tensor_index had referred to in the original multi--dimensional representation of that tensor.
*/
template<typename DimensionRange> unsigned long long tensorIndexToFlatIndex(DimensionRange const& dimensions, vector<unsigned int> const& tensor_index) {
    assert(dimensions.size() == tensor_index.size());
    unsigned long long flat_index = 0;
    BOOST_FOREACH(unsigned int const i, irange(0u,(unsigned int)dimensions.size())) {
        assert(tensor_index[i] < dimensions[i]);
        flat_index *= dimensions[i];
        flat_index += tensor_index[i];
    }
    return flat_index;
}

//! @}

}

#endif
