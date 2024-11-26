#include "scalar.hpp"

/*

The scalar module.

Signature: 
    - Constants: 0, 1
    - Unary Symbols: CONJ
    - AC Symbols: ADDS, MULS

*/

namespace scalar {

    using namespace ualg;

    StringSymbolType symbols = {
        {"0", SymbolType::NORMAL},
        {"1", SymbolType::NORMAL},
        {"CONJ", SymbolType::NORMAL},
        {"ADDS", SymbolType::AC},
        {"MULS", SymbolType::AC}
    };

    const Signature<int> reserved_sig = compile_string_sig(symbols);

    int ZERO = reserved_sig.get_repr("0");
    int ONE = reserved_sig.get_repr("1");
    int CONJ = reserved_sig.get_repr("CONJ");
    int ADDS = reserved_sig.get_repr("ADDS");
    int MULS = reserved_sig.get_repr("MULS");

    /////////////////////////////////////////////////////////////////////////
    // Properties
    REWRITE_COMPILED_DEF(R_ADDSID, bank, term) {
        TermCountMapping<int> args;
        if (match_ac_head(term, ADDS, args)) {
            if (args.size() == 1 && args.begin()->second == 1) {
                return args.begin()->first;
            }
        }
        return std::nullopt;
    }

    REWRITE_COMPILED_DEF(R_MULSID, bank, term) {
        TermCountMapping<int> args;
        if (match_ac_head(term, MULS, args)) {
            if (args.size() == 1 && args.begin()->second == 1) {
                return args.begin()->first;
            }
        }
        return std::nullopt;
    }

    /////////////////////////////////////////////////////////////////////////
    // Rewriting Rules

    // ADDS(a 0) -> a
    REWRITE_COMPILED_DEF(R_ADDS0, bank, term) {
        auto zero_term = bank.get_normal_term(ZERO, {});

        TermCountMapping<int> args_ADDS_a_0;
        if (match_ac_head(term, ADDS, args_ADDS_a_0)) {
            if (args_ADDS_a_0.find(zero_term) != args_ADDS_a_0.end()) {

                // remove the zero term
                auto new_args = TermCountMapping<int>(args_ADDS_a_0);
                new_args.erase(zero_term);

                return bank.get_ac_term(ADDS, std::move(new_args));
            }
        }

        return std::nullopt;
    }

    // MULS(a 0) -> 0
    REWRITE_COMPILED_DEF(R_MULS0, bank, term) {
        auto zero_term = bank.get_normal_term(ZERO, {});

        TermCountMapping<int> args_MULS_a_0;
        if (match_ac_head(term, MULS, args_MULS_a_0)) {
            if (args_MULS_a_0.find(zero_term) != args_MULS_a_0.end()) {
                return zero_term;
            }
        }

        return std::nullopt;
    }

    // MULS(a 1) -> a
    REWRITE_COMPILED_DEF(R_MULS1, bank, term) {
        auto one_term = bank.get_normal_term(ONE, {});

        TermCountMapping<int> args_MULS_a_1;
        if (match_ac_head(term, MULS, args_MULS_a_1)) {
            if (args_MULS_a_1.find(one_term) != args_MULS_a_1.end()) {

                // remove the one term
                auto new_args = TermCountMapping<int>(args_MULS_a_1);
                new_args.erase(one_term);

                return bank.get_ac_term(MULS, std::move(new_args));
            }
        }

        return std::nullopt;
    }


    // MULS(a ADDS(b c)) -> ADDS(MULS(a b) MULS(a c))
    REWRITE_COMPILED_DEF(R_MULS2, bank, term) {

        TermCountMapping<int> args_MULS_a_ADDS_b_c;
        if (match_ac_head(term, MULS, args_MULS_a_ADDS_b_c)) {

            for (const auto& [arg, count] : args_MULS_a_ADDS_b_c) {
                TermCountMapping<int> args_ADDS_b_c;
                if (match_ac_head(arg, ADDS, args_ADDS_b_c)) {
                    
                    // get the arguments
                    auto resarg_MULS = TermCountMapping<int>(args_MULS_a_ADDS_b_c);
                    subtract_TermCountMapping(resarg_MULS, arg, 1);

                    auto resarg_ADDS = TermCountMapping<int>();

                    for (const auto& [arg, count]: args_ADDS_b_c) {
                        auto resarg_inner_MULS = TermCountMapping<int>(resarg_MULS);
                        add_TermCountMapping(resarg_inner_MULS, arg, 1);
                        resarg_ADDS[bank.get_ac_term(MULS, std::move(resarg_inner_MULS))] = count;
                    }

                    return bank.get_ac_term(ADDS, std::move(resarg_ADDS));
                }
            }
        }

        return std::nullopt;
    }

    // CONJ(0) -> 0
    REWRITE_COMPILED_DEF(R_CONJ0, bank, term) {
        auto zero_term = bank.get_normal_term(ZERO, {});
        auto CONJ_0_term = bank.get_normal_term(CONJ, {zero_term});

        if (term == CONJ_0_term) {
            return zero_term;
        }

        return std::nullopt;
    }

    // CONJ(1) -> 1
    REWRITE_COMPILED_DEF(R_CONJ1, bank, term) {
        auto one_term = bank.get_normal_term(ONE, {});
        auto CONJ_1_term = bank.get_normal_term(CONJ, {one_term});

        if (term == CONJ_1_term) {
            return one_term;
        }

        return std::nullopt;
    }

    // CONJ(ADDS(a b)) -> ADDS(CONJ(a) CONJ(b))
    REWRITE_COMPILED_DEF(R_CONJ2, bank, term) {

        ListArgs<int> args_CONJ_ADDS_a_b;
        if (match_normal_head(term, CONJ, args_CONJ_ADDS_a_b)) {
            TermCountMapping<int> args_ADDS_a_b;
            if (match_ac_head(args_CONJ_ADDS_a_b[0], ADDS, args_ADDS_a_b)) {
                TermCountMapping<int> new_args;
                for (const auto& [arg, count] : args_ADDS_a_b) {
                    add_TermCountMapping(new_args, bank.get_normal_term(CONJ, {arg}), count);
                }
                return bank.get_ac_term(ADDS, std::move(new_args));
            }
        }

        return std::nullopt;
    }

    // CONJ(MULS(a b)) -> MULS(CONJ(a) CONJ(b))
    REWRITE_COMPILED_DEF(R_CONJ3, bank, term) {
        ListArgs<int> args_CONJ_MULS_a_b;

        if (match_normal_head(term, CONJ, args_CONJ_MULS_a_b)) {
            TermCountMapping<int> args_MULS_a_b;
            if (match_ac_head(args_CONJ_MULS_a_b[0], MULS, args_MULS_a_b)) {
                TermCountMapping<int> new_args;
                for (const auto& [arg, count] : args_MULS_a_b) {
                    add_TermCountMapping(new_args, bank.get_normal_term(CONJ, {arg}), count);
                }
                return bank.get_ac_term(MULS, std::move(new_args));
            }
        }

        return std::nullopt;
    }

    // CONJ(CONJ(a)) -> a
    REWRITE_COMPILED_DEF(R_CONJ4, bank, term) {
        ListArgs<int> args_CONJ_CONJ_a;
        if (match_normal_head(term, CONJ, args_CONJ_CONJ_a)) {
            ListArgs<int> args_CONJ_a;
            if (match_normal_head(args_CONJ_CONJ_a[0], CONJ, args_CONJ_a)) {
                return args_CONJ_a[0];
            }
        }

        return std::nullopt;
    }

    //////////////////////////////////////////
    // define the rule list
    const std::vector<RewritingRule<int>> scalar_rules = {
        R_ADDSID,
        R_MULSID,
        R_ADDS0,
        R_MULS0,
        R_MULS1,
        R_MULS2,
        R_CONJ0,
        R_CONJ1,
        R_CONJ2,
        R_CONJ3,
        R_CONJ4
    };

} // namespace scalar