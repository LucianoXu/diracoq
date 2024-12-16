#pragma once

#include "ualg.hpp"
#include <string>
#include <set>

namespace diracoq {

    extern const int deBruijn_index_num;

    extern std::vector<std::string> diracoq_symbols;

    extern const ualg::Signature<int> diracoq_sig;

    inline bool is_reserved(int symbol) {
        return diracoq_sig.find_name(symbol) != std::nullopt;
    }

    extern const int INDEX, TYPE;
    extern const int Prod;
    extern const int BASIS, SType, KType, BType, OType, ARROW, FORALL, Set;

    extern const int PAIR, FUN, IDX, APPLY;
    
    extern const int ZERO, ONE, ADDS, MULS, CONJ, DELTA, DOT, ZEROK, ZEROB, ZEROO, ONEO, KET, BRA, ADJ, SCR, ADD, TSR, MULK, MULB, OUTER, MULO;

    extern const int USET, CATPROD, SUM;

    extern const std::set<int> a_symbols;
    extern const std::set<int> c_symbols;


    extern unsigned long long unique_var_id;

    inline std::string unique_var() {
        return "@" + std::to_string(unique_var_id++);
    }
    
} // namespace diracoq