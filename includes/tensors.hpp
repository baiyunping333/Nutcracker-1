//@+leo-ver=5-thin
//@+node:gcross.20110124161335.2009: * @thin tensors.hpp
//@@language cplusplus

#ifndef NUTCRACKER_TENSORS_HPP
#define NUTCRACKER_TENSORS_HPP

//@+<< Includes >>
//@+node:gcross.20110124161335.2010: ** << Includes >>
#include <boost/assign/list_of.hpp>
#include <boost/concept_check.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/move/move.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/concepts.hpp>
#include <boost/range/irange.hpp>
#include <boost/smart_ptr/scoped_array.hpp>
#include <boost/utility.hpp>
#include <complex>
#include <exception>
#include <ostream>
#include <stdint.h>

#include "utilities.hpp"
//@-<< Includes >>

namespace Nutcracker {

//@+<< Usings >>
//@+node:gcross.20110124161335.2011: ** << Usings >>
using namespace boost;
using namespace boost::assign;
using namespace std;
//@-<< Usings >>

//@+others
//@+node:gcross.20110124175241.1520: ** enum Side
enum Side { Left, Middle, Right };

template<Side other_side> struct Other { static Side const side; };
template<Side other_side> Side const Other<other_side>::side = (Side)(2u-other_side);
//@+node:gcross.20110127123226.2852: ** Parameters wrappers
#define DEFINE_TEMPLATIZED_PARAMETER(Parameter,parameter) \
    template<typename T> struct Parameter { \
        T& data; \
        Parameter(T& data) : data(data) {} \
        T& operator*() const { return data; } \
        T* operator->() const { return &data; } \
        operator Parameter<T const>() const { return Parameter(data); } \
    }; \
    template<typename T> Parameter<T> parameter(T& data) { return Parameter<T>(data); } \
    template<typename T> Parameter<T const> parameter(T const& data) { return Parameter<T const>(data); } \
    template<typename T> Parameter<T const> parameter##Const(T& data) { return Parameter<T const>(data); }

DEFINE_TEMPLATIZED_PARAMETER(CopyFrom,copyFrom)
DEFINE_TEMPLATIZED_PARAMETER(DimensionsOf,dimensionsOf)
DEFINE_TEMPLATIZED_PARAMETER(FillWithGenerator,fillWithGenerator)
DEFINE_TEMPLATIZED_PARAMETER(FillWithRange,fillWithRange)
//@+node:gcross.20110127123226.2856: ** Dimension wrappers
#define DEFINE_DIMENSION(CapsName) \
    class CapsName##Dimension { \
    private: \
        unsigned int const dimension; \
    public: \
        explicit CapsName##Dimension(unsigned int const dimension) : dimension(dimension) { } \
        unsigned int operator *() const { return dimension; } \
        unsigned int operator ()() const { return dimension; } \
        bool operator==(CapsName##Dimension const other) const { return dimension == other.dimension; } \
    }; \
    inline ostream& operator<<(ostream& out, CapsName##Dimension const d) { return (out << d()); }

DEFINE_DIMENSION(Left);
DEFINE_DIMENSION(Operator);
DEFINE_DIMENSION(Overlap);
DEFINE_DIMENSION(Physical);
DEFINE_DIMENSION(Right);
DEFINE_DIMENSION(State);
//@+node:gcross.20110129220506.1661: ** Dummy arguments
#define DEFINE_DUMMY_PARAMETER(Parameter,parameter) \
    static struct Parameter {} const parameter = Parameter();

DEFINE_DUMMY_PARAMETER(MakeTrivial,make_trivial);
DEFINE_DUMMY_PARAMETER(AsUnsignedInteger,as_unsigned_integer);
//@+node:gcross.20110124161335.2012: ** Classes
//@+node:gcross.20110126150230.1601: *3* BaseTensor
class BaseTensor {
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(BaseTensor)
    unsigned int data_size;
    scoped_array<complex<double> > data;
public:

    typedef complex<double> value_type;
    typedef value_type* iterator;
    typedef value_type const* const_iterator;
    typedef value_type& reference;
    typedef value_type const& const_reference;

    BaseTensor(BOOST_RV_REF(BaseTensor) other)
      : data_size(other.data_size)
    { data.swap(other.data); }

    BaseTensor(unsigned int const size)
      : data_size(size)
      , data(new complex<double>[data_size])
    { }

    template<typename Tensor> BaseTensor(CopyFrom<Tensor> const other)
      : data_size(other->data_size)
      , data(new complex<double>[data_size])
    {
        copy(*other,data.get());
    }

    template<typename G> BaseTensor
        ( unsigned int const size
        , FillWithGenerator<G> const generator
        )
      : data_size(size)
      , data(new complex<double>[data_size])
    {
        BOOST_CONCEPT_ASSERT(( Generator<G,complex<double> > ));
        generate_n(data.get(),size,*generator);
    }

    template<typename Range> BaseTensor(FillWithRange<Range> const init)
      : data_size(init->size())
      , data(new complex<double>[data_size])
    {
        BOOST_CONCEPT_ASSERT(( RandomAccessRangeConcept<Range> ));
        copy(*init,data.get());
    }

    BaseTensor(MakeTrivial const make_trivial)
      : data_size(1)
      , data(new complex<double>[1])
    {
        data[0] = c(1,0);
    }

    unsigned int size() const { return data_size; }

    operator complex<double>*() { return data.get(); }
    operator complex<double> const*() const { return data.get(); }

    complex<double>* begin() { return data.get(); }
    complex<double> const* begin() const { return data.get(); }

    complex<double>* end() { return data.get()+data_size; }
    complex<double> const* end() const { return data.get()+data_size; }

    complex<double>& operator[](unsigned int const index) { return *(data.get()+index); }
    complex<double> operator[](unsigned int const index) const { return *(data.get()+index); }
};
//@+node:gcross.20110124175241.1530: *3* Boundary
//@+node:gcross.20110124161335.2013: *4* ExpectationBoundary
template<Side side> class ExpectationBoundary : public BaseTensor {
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(ExpectationBoundary)
    OperatorDimension operator_dimension;
    StateDimension state_dimension;
public:
    ExpectationBoundary(BOOST_RV_REF(ExpectationBoundary) other)
      : BaseTensor(boost::move(static_cast<BaseTensor&>(other)))
      , operator_dimension(other.operatorDimension())
      , state_dimension(other.stateDimension())
    { }

    ExpectationBoundary(
          OperatorDimension const operator_dimension
        , StateDimension const state_dimension
    ) : BaseTensor(operator_dimension()*state_dimension()*state_dimension())
      , operator_dimension(operator_dimension)
      , state_dimension(state_dimension)
    { }

    template<Side other_side> ExpectationBoundary(
          CopyFrom<ExpectationBoundary<other_side> const> const other
    ) : BaseTensor(other)
      , operator_dimension(other->operator_dimension)
      , state_dimension(other->state_dimension)
    { }

    template<typename G> ExpectationBoundary(
          OperatorDimension const operator_dimension
        , StateDimension const state_dimension
        , FillWithGenerator<G> const generator
    ) : BaseTensor(operator_dimension()*state_dimension()*state_dimension(),generator)
      , operator_dimension(operator_dimension)
      , state_dimension(state_dimension)
    { }

    template<typename Range> ExpectationBoundary(
          OperatorDimension const operator_dimension
        , FillWithRange<Range> const init
    ) : BaseTensor(init)
      , operator_dimension(operator_dimension)
      , state_dimension((unsigned int)sqrt(size()/operator_dimension()))
    { }

    ExpectationBoundary(
          MakeTrivial const make_trivial
    ) : BaseTensor(make_trivial)
      , operator_dimension(1)
      , state_dimension(1)
    { }

    OperatorDimension operatorDimension() const { return operator_dimension; }
    unsigned int operatorDimension(AsUnsignedInteger _) const { return *operator_dimension; }

    StateDimension stateDimension() const { return state_dimension; }
    unsigned int stateDimension(AsUnsignedInteger _) const { return *state_dimension; }

    static ExpectationBoundary const trivial;
};

template<Side side> ExpectationBoundary<side> const ExpectationBoundary<side>::trivial(make_trivial);
//@+node:gcross.20110124175241.1526: *4* OverlapBoundary
template<Side side> class OverlapBoundary : public BaseTensor {
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(OverlapBoundary)
    OverlapDimension overlap_dimension;
    StateDimension state_dimension;
public:
    OverlapBoundary(BOOST_RV_REF(OverlapBoundary) other)
      : BaseTensor(boost::move(static_cast<BaseTensor&>(other)))
      , overlap_dimension(other.overlapDimension())
      , state_dimension(other.stateDimension())
    { }

    OverlapBoundary(
          OverlapDimension const overlap_dimension
        , StateDimension const state_dimension
    ) : BaseTensor(overlap_dimension()*state_dimension())
      , overlap_dimension(overlap_dimension)
      , state_dimension(state_dimension)
    { }

    template<Side other_side> OverlapBoundary(
          CopyFrom<OverlapBoundary<other_side> const> const other
    ) : BaseTensor(other)
      , overlap_dimension(other->overlap_dimension)
      , state_dimension(other->state_dimension)
    { }

    template<typename G> OverlapBoundary(
          OverlapDimension const overlap_dimension
        , StateDimension const state_dimension
        , FillWithGenerator<G> const generator
    ) : BaseTensor(overlap_dimension()*state_dimension(),generator)
      , overlap_dimension(overlap_dimension)
      , state_dimension(state_dimension)
    { }

    template<typename Range> OverlapBoundary(
          OverlapDimension const overlap_dimension
        , FillWithRange<Range> const init
    ) : BaseTensor(init)
      , overlap_dimension(overlap_dimension)
      , state_dimension(size()/overlap_dimension())
    { }

    OverlapBoundary(
          MakeTrivial const make_trivial
    ) : BaseTensor(make_trivial)
      , overlap_dimension(1)
      , state_dimension(1)
    { }

    OverlapDimension overlapDimension() const { return overlap_dimension; }
    unsigned int overlapDimension(AsUnsignedInteger _) const { return *overlap_dimension; }

    StateDimension stateDimension() const { return state_dimension; }
    unsigned int stateDimension(AsUnsignedInteger _) const { return *state_dimension; }

    static OverlapBoundary const trivial;
};

template<Side side> OverlapBoundary<side> const OverlapBoundary<side>::trivial(make_trivial);
//@+node:gcross.20110124175241.1538: *3* ProjectorMatrix
class ProjectorMatrix {
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(ProjectorMatrix)
    unsigned int
          number_of_projectors
        , projector_length
        , number_of_reflectors
        , orthogonal_subspace_dimension
        ;
    scoped_array<complex<double> > reflector_data, coefficient_data;
    scoped_array<uint32_t> swap_data;
public:

    ProjectorMatrix(BOOST_RV_REF(ProjectorMatrix) other)
      : number_of_projectors(other.number_of_projectors)
      , projector_length(other.projector_length)
      , number_of_reflectors(other.number_of_reflectors)
      , orthogonal_subspace_dimension(other.orthogonal_subspace_dimension)
    {
        reflector_data.swap(other.reflector_data);
        coefficient_data.swap(other.coefficient_data);
        swap_data.swap(other.swap_data);
    }

    ProjectorMatrix(
          unsigned int const number_of_projectors
        , unsigned int const projector_length
        , unsigned int const number_of_reflectors
        , unsigned int const orthogonal_subspace_dimension
        , complex<double>* reflector_data
        , complex<double>* coefficient_data
        , uint32_t* swap_data
    ) : number_of_projectors(number_of_projectors)
      , projector_length(projector_length)
      , number_of_reflectors(number_of_reflectors)
      , orthogonal_subspace_dimension(orthogonal_subspace_dimension)
      , reflector_data(reflector_data)
      , coefficient_data(coefficient_data)
      , swap_data(swap_data)
    { }

    unsigned int numberOfProjectors() const { return number_of_projectors; }
    unsigned int projectorLength() const { return projector_length; }
    unsigned int numberOfReflectors() const { return number_of_reflectors; }
    unsigned int orthogonalSubspaceDimension() const { return orthogonal_subspace_dimension; }

    complex<double>* reflectorData() { return reflector_data.get(); }
    complex<double> const* reflectorData() const { return reflector_data.get(); }

    complex<double>* coefficientData() { return coefficient_data.get(); }
    complex<double> const* coefficientData() const { return coefficient_data.get(); }

    uint32_t* swapData() { return swap_data.get(); }
    uint32_t const* swapData() const { return swap_data.get(); }
};
//@+node:gcross.20110124175241.1531: *3* Site
//@+node:gcross.20110202142407.1690: *4* SiteBaseTensor
class SiteBaseTensor : public BaseTensor {
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(SiteBaseTensor)

    PhysicalDimension const physical_dimension;
    LeftDimension const left_dimension;
    RightDimension const right_dimension;

protected:
    SiteBaseTensor(BOOST_RV_REF(SiteBaseTensor) other)
      : BaseTensor(boost::move(static_cast<BaseTensor&>(other)))
      , physical_dimension(other.physical_dimension)
      , left_dimension(other.left_dimension)
      , right_dimension(other.right_dimension)
    { }

    SiteBaseTensor(
          PhysicalDimension const physical_dimension
        , LeftDimension const left_dimension
        , RightDimension const right_dimension
        , unsigned int const size
    ) : BaseTensor(size)
      , physical_dimension(physical_dimension)
      , left_dimension(left_dimension)
      , right_dimension(right_dimension)
    { }

    template<typename Tensor> SiteBaseTensor(CopyFrom<Tensor const> const other)
      : BaseTensor(other)
      , physical_dimension(other->physicalDimension())
      , left_dimension(other->leftDimension())
      , right_dimension(other->rightDimension())
    { }

    template<typename Tensor> SiteBaseTensor(DimensionsOf<Tensor const> const other)
      : BaseTensor(other->size())
      , physical_dimension(other->physicalDimension())
      , left_dimension(other->leftDimension())
      , right_dimension(other->rightDimension())
    { }

    template<typename G> SiteBaseTensor(
          PhysicalDimension const physical_dimension
        , LeftDimension const left_dimension
        , RightDimension const right_dimension
        , unsigned int const size
        , FillWithGenerator<G> const generator
    ) : BaseTensor(size,generator)
      , physical_dimension(physical_dimension)
      , left_dimension(left_dimension)
      , right_dimension(right_dimension)
    { }

    template<typename Range> SiteBaseTensor(
          PhysicalDimension const physical_dimension
        , LeftDimension const left_dimension
        , RightDimension const right_dimension
        , FillWithRange<Range> const init
    ) : BaseTensor(init)
      , physical_dimension(physical_dimension)
      , left_dimension(left_dimension)
      , right_dimension(right_dimension)
    { }

    SiteBaseTensor(MakeTrivial const make_trivial)
      : BaseTensor(make_trivial)
      , physical_dimension(1)
      , left_dimension(1)
      , right_dimension(1)
    { }

public:
    PhysicalDimension physicalDimension() const { return physical_dimension; }
    unsigned int physicalDimension(AsUnsignedInteger _) const { return *physical_dimension; }

    LeftDimension leftDimension() const { return left_dimension; }
    unsigned int leftDimension(AsUnsignedInteger _) const { return *left_dimension; }

    RightDimension rightDimension() const { return right_dimension; }
    unsigned int rightDimension(AsUnsignedInteger _) const { return *right_dimension; }
};
//@+node:gcross.20110124175241.1533: *4* OperatorSite
class OperatorSite : public SiteBaseTensor {
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(OperatorSite)

    unsigned int const number_of_matrices;
    scoped_array<uint32_t> index_data;
public:
    OperatorSite(BOOST_RV_REF(OperatorSite) other)
      : SiteBaseTensor(boost::move(static_cast<SiteBaseTensor&>(other)))
      , number_of_matrices(other.numberOfMatrices())
    {
        index_data.swap(other.index_data);
    }

    OperatorSite(
          unsigned int const number_of_matrices
        , PhysicalDimension const physical_dimension
        , LeftDimension const left_dimension
        , RightDimension const right_dimension
    ) : SiteBaseTensor(
             physical_dimension
            ,left_dimension
            ,right_dimension
            ,number_of_matrices*physical_dimension()*physical_dimension()
        )
      , number_of_matrices(number_of_matrices)
      , index_data(new uint32_t[number_of_matrices*2])
    { }

    template<typename G1, typename G2> OperatorSite(
          unsigned int const number_of_matrices
        , PhysicalDimension const physical_dimension
        , LeftDimension const left_dimension
        , RightDimension const right_dimension
        , FillWithGenerator<G1> const index_generator
        , FillWithGenerator<G2> const matrix_generator
    ) : SiteBaseTensor(
             physical_dimension
            ,left_dimension
            ,right_dimension
            ,number_of_matrices*physical_dimension()*physical_dimension()
            ,matrix_generator
        )
      , number_of_matrices(number_of_matrices)
      , index_data(new uint32_t[number_of_matrices*2])
    {
        BOOST_CONCEPT_ASSERT(( Generator<G1,uint32_t> ));
        uint32_t* index = index_data.get();
        REPEAT(number_of_matrices) {
            uint32_t left_index = (*index_generator)()
                   , right_index = (*index_generator)()
                   ;
            assert(left_index >= 1 && left_index <= left_dimension());
            assert(right_index >= 1 && right_index <= right_dimension());
            *index++ = left_index;
            *index++ = right_index;
        }
        //generate_n(index_data.get(),size,*index_generator);
    }

    template<typename Range1, typename Range2> OperatorSite(
          LeftDimension const left_dimension
        , RightDimension const right_dimension
        , FillWithRange<Range1> const index_init
        , FillWithRange<Range2> const matrix_init
    ) : SiteBaseTensor(
             PhysicalDimension((unsigned int)sqrt(matrix_init->size()/(index_init->size()/2)))
            ,left_dimension
            ,right_dimension
            ,matrix_init
        )
      , number_of_matrices(index_init->size()/2)
      , index_data(new uint32_t[index_init->size()])
    {
        BOOST_CONCEPT_ASSERT(( RandomAccessRangeConcept<Range1> ));
        copy(*index_init,index_data.get());
    }

    OperatorSite(MakeTrivial const make_trivial)
      : SiteBaseTensor(make_trivial)
      , number_of_matrices(1)
      , index_data(new uint32_t[2])
    {
        fill_n(index_data.get(),2,1);
    }

    unsigned int numberOfMatrices() const { return number_of_matrices; }

    operator uint32_t*() { return index_data.get(); }
    operator uint32_t const*() const { return index_data.get(); }

    static OperatorSite const trivial;
};
//@+node:gcross.20110124175241.1535: *4* StateSite
template<Side side> class StateSite : public SiteBaseTensor {
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(StateSite)
public:
    StateSite(BOOST_RV_REF(StateSite) other)
      : SiteBaseTensor(boost::move(static_cast<SiteBaseTensor&>(other)))
    { }

    StateSite(
          PhysicalDimension const physical_dimension
        , LeftDimension const left_dimension
        , RightDimension const right_dimension
    ) : SiteBaseTensor(
             physical_dimension
            ,left_dimension
            ,right_dimension
            ,physical_dimension()*left_dimension()*right_dimension()
        )
    { }

    template<Side other_side> StateSite(
          CopyFrom<StateSite<other_side> const> const other_site
    ) : SiteBaseTensor(other_site)
    { }

    template<Side other_side> StateSite(
          DimensionsOf<StateSite<other_side> const> const other_site
    ) : SiteBaseTensor(other_site)
    { }

    template<typename G> StateSite(
          PhysicalDimension const physical_dimension
        , LeftDimension const left_dimension
        , RightDimension const right_dimension
        , FillWithGenerator<G> const generator
    ) : SiteBaseTensor(
             physical_dimension
            ,left_dimension
            ,right_dimension
            ,physical_dimension()*left_dimension()*right_dimension()
            ,generator
        )
    { }

    template<typename Range> StateSite(
          LeftDimension const left_dimension
        , RightDimension const right_dimension
        , FillWithRange<Range> const init
    ) : SiteBaseTensor(
             PhysicalDimension(init->size()/(left_dimension()*right_dimension()))
            ,left_dimension
            ,right_dimension
            ,init
        )
    { }

    StateSite(MakeTrivial const make_trivial) : SiteBaseTensor(make_trivial) {}

    static StateSite const trivial;
};

template<Side side> StateSite<side> const StateSite<side>::trivial(make_trivial);
//@+node:gcross.20110124175241.1537: *4* OverlapSite
template<Side side> class OverlapSite : public SiteBaseTensor {
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(OverlapSite)
public:
    OverlapSite(BOOST_RV_REF(OverlapSite) other)
      : SiteBaseTensor(boost::move(static_cast<SiteBaseTensor&>(other)))
    { }

    OverlapSite(
          RightDimension const right_dimension
        , PhysicalDimension const physical_dimension
        , LeftDimension const left_dimension
    ) : SiteBaseTensor(
             physical_dimension
            ,left_dimension
            ,right_dimension
            ,physical_dimension()*left_dimension()*right_dimension()
        )
    { }

    template<Side other_side> OverlapSite(
          CopyFrom<OverlapSite<other_side> const> const other_site
    ) : SiteBaseTensor(other_site)
    { }

    template<Side other_side> OverlapSite(
          DimensionsOf<StateSite<other_side> const> const other_site
    ) : SiteBaseTensor(other_site)
    { }

    template<typename G> OverlapSite(
          RightDimension const right_dimension
        , PhysicalDimension const physical_dimension
        , LeftDimension const left_dimension
        , FillWithGenerator<G> const generator
    ) : SiteBaseTensor(
             physical_dimension
            ,left_dimension
            ,right_dimension
            ,physical_dimension()*left_dimension()*right_dimension()
            ,generator
        )
    { }

    template<typename Range> OverlapSite(
          RightDimension const right_dimension
        , LeftDimension const left_dimension
        , FillWithRange<Range> const init
    ) : SiteBaseTensor(
             PhysicalDimension(init->size()/(left_dimension()*right_dimension()))
            ,left_dimension
            ,right_dimension
            ,init
        )
    { }

    OverlapSite(MakeTrivial const make_trivial) : SiteBaseTensor(make_trivial) {}

    static OverlapSite const trivial;
};

template<Side side> OverlapSite<side> const OverlapSite<side>::trivial(make_trivial);
//@+node:gcross.20110125120748.2431: ** Connectors
//@+node:gcross.20110125120748.2434: *3* exception DimensionMismatch
struct DimensionMismatch : public Exception {
    DimensionMismatch(
          const char* n1
        , unsigned int const d1
        , const char* n2
        , unsigned int const d2
    ) : Exception((format("%1% dimension (%2%) does not match %3% dimension (%4%)") % n1 % d1 % n2 % d2).str())
    { }
};
//@+node:gcross.20110125120748.2435: *3* function connectDimensions
inline unsigned int connectDimensions(
      const char* n1
    , unsigned int const d1
    , const char* n2
    , unsigned int const d2
) {
    if(d1 != d2) throw DimensionMismatch(n1,d1,n2,d2);
    return d1;
}
//@+node:gcross.20110125120748.2439: *3* operator||
//@+node:gcross.20110125120748.2449: *4* ExpectationBoundary<Left> || OperatorSite
inline unsigned int operator||(
      ExpectationBoundary<Left> const& expectation_boundary
    , OperatorSite const& operator_site
) {
    return connectDimensions(
         "left expectation boundary state"
        ,expectation_boundary.operatorDimension(as_unsigned_integer)
        ,"operator site left"
        ,operator_site.leftDimension(as_unsigned_integer)
    );
}
//@+node:gcross.20110125120748.2441: *4* ExpectationBoundary<Left> || StateSite<Middle>
inline unsigned int operator||(
      ExpectationBoundary<Left> const& expectation_boundary
    , StateSite<Middle> const& state_site
) {
    return connectDimensions(
         "left expectation boundary state"
        ,expectation_boundary.stateDimension(as_unsigned_integer)
        ,"middle state site left"
        ,state_site.leftDimension(as_unsigned_integer)
    );
}
//@+node:gcross.20110127123226.2871: *4* ExpectationBoundary<Left> || StateSite<Left>
inline unsigned int operator||(
      ExpectationBoundary<Left> const& expectation_boundary
    , StateSite<Left> const& state_site
) {
    return connectDimensions(
         "left expectation boundary state"
        ,expectation_boundary.stateDimension(as_unsigned_integer)
        ,"middle state site left"
        ,state_site.leftDimension(as_unsigned_integer)
    );
}
//@+node:gcross.20110125120748.2451: *4* OperatorSite || ExpectationBoundary<Right>
inline unsigned int operator||(
      OperatorSite const& operator_site
    , ExpectationBoundary<Right> const& expectation_boundary
) {
    return connectDimensions(
         "operator site right"
        ,operator_site.rightDimension(as_unsigned_integer)
        ,"right expectation boundary state"
        ,expectation_boundary.operatorDimension(as_unsigned_integer)
    );
}
//@+node:gcross.20110125120748.2457: *4* OperatorSite || StateSite<Middle>
template<Side side> inline unsigned int operator||(
      OperatorSite const& operator_site
    , StateSite<side> const& state_site
) {
    return connectDimensions(
         "operator site physical"
        ,operator_site.physicalDimension(as_unsigned_integer)
        ,"middle state site physical"
        ,state_site.physicalDimension(as_unsigned_integer)
    );
}
//@+node:gcross.20110127123226.2843: *4* OverlapBoundary<Left> || OverlapSite<Left>
inline unsigned int operator||(
      OverlapBoundary<Left> const& overlap_boundary
    , OverlapSite<Left> const& overlap_site
) {
    return connectDimensions(
         "left overlap boundary overlap"
        ,overlap_boundary.overlapDimension(as_unsigned_integer)
        ,"left overlap site left"
        ,overlap_site.leftDimension(as_unsigned_integer)
    );
}
//@+node:gcross.20110127123226.2841: *4* OverlapBoundary<Left> || StateSite<Left>
inline unsigned int operator||(
      OverlapBoundary<Left> const& overlap_boundary
    , StateSite<Left> const& state_site
) {
    return connectDimensions(
         "left overlap boundary state"
        ,overlap_boundary.stateDimension(as_unsigned_integer)
        ,"left state site left"
        ,state_site.leftDimension(as_unsigned_integer)
    );
}
//@+node:gcross.20110125120748.2445: *4* OverlapBoundary<Left> || StateSite<Middle>
inline unsigned int operator||(
      OverlapBoundary<Left> const& overlap_boundary
    , StateSite<Middle> const& state_site
) {
    return connectDimensions(
         "left overlap boundary state"
        ,overlap_boundary.stateDimension(as_unsigned_integer)
        ,"middle state site left"
        ,state_site.leftDimension(as_unsigned_integer)
    );
}
//@+node:gcross.20110125120748.2453: *4* OverlapSite<*> || StateSite<*>
template<Side side> inline unsigned int operator||(
      OverlapSite<side> const& overlap_site
    , StateSite<side> const& state_site
) {
    return connectDimensions(
         "overlap site physical"
        ,overlap_site.physicalDimension(as_unsigned_integer)
        ,"state site physical"
        ,state_site.physicalDimension(as_unsigned_integer)
    );
}
//@+node:gcross.20110127123226.2847: *4* OverlapSite<Right> || OverlapBoundary<Right>
inline unsigned int operator||(
      OverlapSite<Right> const& overlap_site
    , OverlapBoundary<Right> const& overlap_boundary
) {
    return connectDimensions(
         "right overlap site right"
        ,overlap_site.rightDimension(as_unsigned_integer)
        ,"right overlap boundary overlap"
        ,overlap_boundary.overlapDimension(as_unsigned_integer)
    );
}
//@+node:gcross.20110126102637.2200: *4* ProjectorMatrix || StateSite<Middle>
inline unsigned int operator||(
      ProjectorMatrix const& projector_matrix
    , StateSite<Middle> const& state_site
) {
    return connectDimensions(
         "state site size"
        ,state_site.size()
        ,"projector length"
        ,projector_matrix.projectorLength()
    );
}
//@+node:gcross.20110125120748.2437: *4* StateSite<Left> || StateSite<Middle>
inline unsigned int operator||(
      StateSite<Left> const& state_site_1
    , StateSite<Middle> const& state_site_2
) {
    return connectDimensions(
         "left state site right"
        ,state_site_1.rightDimension(as_unsigned_integer)
        ,"middle state site left"
        ,state_site_2.leftDimension(as_unsigned_integer)
    );
}
//@+node:gcross.20110125120748.2443: *4* StateSite<Middle> || ExpectationBoundary<Left>
inline unsigned int operator||(
      StateSite<Middle> const& state_site
    , ExpectationBoundary<Right> const& expectation_boundary
) {
    return connectDimensions(
         "middle state site right"
        ,state_site.rightDimension(as_unsigned_integer)
        ,"right expectation boundary state"
        ,expectation_boundary.stateDimension(as_unsigned_integer)
    );
}
//@+node:gcross.20110125120748.2447: *4* StateSite<Middle> || OverlapBoundary<Right>
inline unsigned int operator||(
      StateSite<Middle> const& state_site
    , OverlapBoundary<Right> const& overlap_boundary
) {
    return connectDimensions(
         "middle state site right"
        ,state_site.rightDimension(as_unsigned_integer)
        ,"right overlap boundary state"
        ,overlap_boundary.stateDimension(as_unsigned_integer)
    );
}
//@+node:gcross.20110125120748.2436: *4* StateSite<Middle> || StateSite<Right>
inline unsigned int operator||(
      StateSite<Middle> const& state_site_1
    , StateSite<Right> const& state_site_2
) {
    return connectDimensions(
         "middle state site right"
        ,state_site_1.rightDimension(as_unsigned_integer)
        ,"right state site left"
        ,state_site_2.leftDimension(as_unsigned_integer)
    );
}
//@+node:gcross.20110127123226.2869: *4* StateSite<Right> || ExpectationBoundary<Right>
inline unsigned int operator||(
      StateSite<Right> const& state_site
    , ExpectationBoundary<Right> const& expectation_boundary
) {
    return connectDimensions(
         "right state site right"
        ,state_site.rightDimension(as_unsigned_integer)
        ,"right overlap boundary state"
        ,expectation_boundary.stateDimension(as_unsigned_integer)
    );
}
//@+node:gcross.20110127123226.2867: *4* StateSite<Right> || OverlapBoundary<Right>
inline unsigned int operator||(
      StateSite<Right> const& state_site
    , OverlapBoundary<Right> const& overlap_boundary
) {
    return connectDimensions(
         "right state site right"
        ,state_site.rightDimension(as_unsigned_integer)
        ,"right overlap boundary state"
        ,overlap_boundary.stateDimension(as_unsigned_integer)
    );
}
//@+node:gcross.20110127123226.2855: *4* StateSite<Right> || StateSite<Right>
inline unsigned int operator||(
      StateSite<Right> const& state_site_1
    , StateSite<Right> const& state_site_2
) {
    return connectDimensions(
         "right state site right"
        ,state_site_1.rightDimension(as_unsigned_integer)
        ,"right state site left"
        ,state_site_2.leftDimension(as_unsigned_integer)
    );
}
//@-others

}

#endif
//@-leo
