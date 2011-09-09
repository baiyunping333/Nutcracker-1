//@+leo-ver=5-thin
//@+node:gcross.20110908221100.3064: * @file state_term.cpp
//@@language cplusplus

//@+<< License >>
//@+node:gcross.20110908221100.3065: ** << License >>
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
//@+node:gcross.20110908221100.3066: ** << Includes >>
#include "terms.hpp"

#include "nutcracker/compiler.hpp"
//@-<< Includes >>

//@+<< Usings >>
//@+node:gcross.20110908221100.3067: ** << Usings >>
//@-<< Usings >>

//@+<< Classes >>
//@+node:gcross.20110908221100.3068: ** << Classes >>
//@+others
//@+node:gcross.20110908221100.3069: *3* NutcrackerStateSumTerm
typedef NutcrackerSumTerm<NutcrackerStateTerm> NutcrackerStateSumTerm;
//@+node:gcross.20110908221100.3070: *3* NutcrackerStateTermWrapper
template<typename Term> struct NutcrackerStateTermWrapper : public NutcrackerTermWrapper<NutcrackerStateTerm,Term> {
    typedef NutcrackerTermWrapper<NutcrackerStateTerm,Term>  Base;
    template<typename A> NutcrackerStateTermWrapper(A const& a) : Base(a) {}
    template<typename A, typename B> NutcrackerStateTermWrapper(A const& a, B const& b) : Base(a,b) {}
    template<typename A, typename B, typename C> NutcrackerStateTermWrapper(A const& a, B const& b, C const& c) : Base(a,b,c) {}
    template<typename A, typename B, typename C, typename D> NutcrackerStateTermWrapper(A const& a, B const& b, C const& c, D const& d) : Base(a,b,c,d) {}
};
//@-others
//@-<< Classes >>

extern "C" {

//@+others
//@+node:gcross.20110908221100.3071: ** Functions
//@+node:gcross.20110908221100.3072: *3* add
NutcrackerStateTerm* Nutcracker_StateTerm_add(NutcrackerStateTerm const* x, NutcrackerStateTerm const* y) {
    return new NutcrackerStateSumTerm(x,y);
}
//@+node:gcross.20110908221100.3076: *3* create_ProductWithOneSiteDifferent
NutcrackerStateTerm* Nutcracker_StateTerm_create_ProductWithOneSiteDifferent(uint32_t site_number, NutcrackerVector const* common_observation, NutcrackerVector const* special_observation) {
    return new NutcrackerStateTermWrapper<Nutcracker::ProductWithOneSiteDifferentTerm>(site_number,*common_observation,*special_observation);
}
//@+node:gcross.20110909000037.3079: *3* create_W
NutcrackerStateTerm* Nutcracker_StateTerm_create_W(NutcrackerVector const* common_observation, NutcrackerVector const* special_observation, bool normalized) {
    return new NutcrackerStateTermWrapper<Nutcracker::WTerm>(*common_observation,*special_observation,normalized);
}
//@+node:gcross.20110908221100.3078: *3* free
void Nutcracker_StateTerm_free(NutcrackerStateTerm* op) {
    delete op;
}
//@+node:gcross.20110908221100.3079: *3* multiply
NutcrackerStateTerm* Nutcracker_StateTerm_multiply(std::complex<double> const* c, NutcrackerStateTerm const* x) {
    return x->copyAndMultiplyBy(*c);
}
//@-others

}
//@-leo