//@+leo-ver=5-thin
//@+node:gcross.20110823131135.2594: * @file operator_builder.cpp
//@@language cplusplus
//@+<< Includes >>
//@+node:gcross.20110823131135.2596: ** << Includes >>
#include <boost/range/adaptor/indirected.hpp>
#include <boost/make_shared.hpp>

#include "common.hpp"

#include "nutcracker/compiler.hpp"




#include <iostream>
//@-<< Includes >>

//@+<< Usings >>
//@+node:gcross.20110823131135.2597: ** << Usings >>
using boost::adaptors::indirected;
using boost::make_shared;
//@-<< Usings >>

extern "C" {

//@+others
//@+node:gcross.20110823131135.2563: ** Functions
//@+node:gcross.20110905174854.2849: *3* addProductTerm
void Nutcracker_OperatorBuilder_addProductTerm(NutcrackerOperatorBuilder* builder, NutcrackerMatrix const* const* components) {
    builder->addProductTerm(boost::make_iterator_range(components,components+builder->numberOfSites()) | indirected);
}
//@+node:gcross.20110904213222.2787: *3* addTerm
void Nutcracker_OperatorBuilder_addTerm(NutcrackerOperatorBuilder* builder,NutcrackerOperatorTerm* term) { builder->addTerm(*term); }
//@+node:gcross.20110903000806.2750: *3* compile
NutcrackerOperator* Nutcracker_OperatorBuilder_compile(NutcrackerOperatorBuilder* builder) { BEGIN_ERROR_REGION {
    return new NutcrackerOperator(builder->compile());
} END_ERROR_REGION(NULL) }
//@+node:gcross.20110903000806.2751: *3* compileCustomized
NutcrackerOperator* Nutcracker_OperatorBuilder_compileCustomized(NutcrackerOperatorBuilder* builder,bool optimize,bool add_start_and_end_loops) { BEGIN_ERROR_REGION {
    return new NutcrackerOperator(builder->compile(optimize,add_start_and_end_loops));
} END_ERROR_REGION(NULL) }
//@+node:gcross.20110906130654.2933: *3* dimensionOfSite
uint32_t Nutcracker_OperatorBuilder_dimensionOfSite(NutcrackerOperatorBuilder const* builder, uint32_t site_number) { BEGIN_ERROR_REGION {
    return builder->dimensionOfSite(site_number);
} END_ERROR_REGION(0) }
//@+node:gcross.20110903000806.2743: *3* free
void Nutcracker_OperatorBuilder_free(NutcrackerOperatorBuilder* builder) { delete builder; }
//@+node:gcross.20110905151655.2826: *3* new
NutcrackerOperatorBuilder* Nutcracker_OperatorBuilder_new(uint32_t number_of_sites, uint32_t* dimensions) {
    return new NutcrackerOperatorBuilder(boost::make_iterator_range(dimensions,dimensions+number_of_sites));
}
//@+node:gcross.20110903000806.2742: *3* newSimple
NutcrackerOperatorBuilder* Nutcracker_OperatorBuilder_newSimple(uint32_t number_of_sites, uint32_t physical_dimension) {
    return new NutcrackerOperatorBuilder(number_of_sites,physical_dimension);
}
//@+node:gcross.20110903000806.2745: *3* numberOfSites
uint32_t Nutcracker_OperatorBuilder_numberOfSites(NutcrackerOperatorBuilder const* builder) { return builder->numberOfSites(); }
//@-others

}
//@-leo
