#include "scalar_vec.hpp"

/*

The scalar module.

Signature: 
    - Constants: 0, 1
    - Unary Symbols: CONJ
    - AC Symbols: ADDS, MULS

*/

namespace scalar_vec {

    using namespace ualg;
    using namespace std;

    StringSymbolType symbols = {
        {"0", SymbolType::NORMAL},
        {"1", SymbolType::NORMAL},
        {"CONJ", SymbolType::NORMAL},
        {"ADDS", SymbolType::NORMAL},
        {"MULS", SymbolType::NORMAL}
    };

    const Signature<int> reserved_sig = compile_string_sig(symbols);

    int ZERO = reserved_sig.get_repr("0");
    int ONE = reserved_sig.get_repr("1");
    int CONJ = reserved_sig.get_repr("CONJ");
    int ADDS = reserved_sig.get_repr("ADDS");
    int MULS = reserved_sig.get_repr("MULS");

    std::set<int> ac_symbols = {ADDS, MULS};
    

    //////////////// Flattening AC symbols
    REWRITE_COMPILED_DEF(R_FLATTEN, bank, term) {
        auto res = flatten<int>(term, bank, ac_symbols);
        if (res != term) {
            return res;
        }
        return std::nullopt;
    }


    /////////////////////////////////////////////////////////////////////////
    // Properties
    REWRITE_COMPILED_DEF(R_ADDSID, bank, term) {
        ListArgs<int> args;
        if (match_normal_head(term, ADDS, args)) {
            if (args.size() == 1) {
                return args[0];
            }
        }
        return std::nullopt;
    }

    REWRITE_COMPILED_DEF(R_MULSID, bank, term) {
        ListArgs<int> args;
        if (match_normal_head(term, MULS, args)) {
            if (args.size() == 1) {
                return args[0];
            }
        }
        return std::nullopt;
    }

    /////////////////////////////////////////////////////////////////////////
    // Rewriting Rules

    // ADDS(a 0) -> a
    REWRITE_COMPILED_DEF(R_ADDS0, bank, term) {
        auto zero_term = bank.get_normal_term(ZERO, {});

        ListArgs<int> args_ADDS_a_0;
        if (match_normal_head(term, ADDS, args_ADDS_a_0)) {
            ListArgs<int> new_args;
            for (const auto& arg : args_ADDS_a_0) {
                if (arg == zero_term) {
                    continue;
                }
                new_args.push_back(arg);
            }
            if (new_args.size() < args_ADDS_a_0.size()) {
                return bank.get_normal_term(ADDS, std::move(new_args));
            }
        }

        return std::nullopt;
    }

    // MULS(a 0) -> 0
    REWRITE_COMPILED_DEF(R_MULS0, bank, term) {
        auto zero_term = bank.get_normal_term(ZERO, {});

        ListArgs<int> args_MULS_a_0;
        if (match_normal_head(term, MULS, args_MULS_a_0)) {
            for (const auto& arg : args_MULS_a_0) {
                if (arg == zero_term) {
                    return zero_term;
                }
            }
        }

        return std::nullopt;
    }

    // MULS(a 1) -> a
    REWRITE_COMPILED_DEF(R_MULS1, bank, term) {
        auto one_term = bank.get_normal_term(ONE, {});

        ListArgs<int> args_MULS_a_1;
        if (match_normal_head(term, MULS, args_MULS_a_1)) {
            ListArgs<int> new_args;
            for (const auto& arg : args_MULS_a_1) {
                if (arg == one_term) {
                    continue;
                }
                new_args.push_back(arg);
            }
            if (new_args.size() < args_MULS_a_1.size()) {
                return bank.get_normal_term(MULS, std::move(new_args));
            }
        }

        return std::nullopt;
    }


    // MULS(a ADDS(b c)) -> ADDS(MULS(a b) MULS(a c))
    REWRITE_COMPILED_DEF(R_MULS2, bank, term) {

        ListArgs<int> args_MULS_a_ADDS_b_c;
        if (match_normal_head(term, MULS, args_MULS_a_ADDS_b_c)) {

            // Does not match MULS(ADDS(...))
            if (args_MULS_a_ADDS_b_c.size() == 1) {
                return std::nullopt;
            }

            for (auto i = 0; i != args_MULS_a_ADDS_b_c.size(); ++i) {
                ListArgs<int> args_ADDS_b_c;
                if (match_normal_head(args_MULS_a_ADDS_b_c[i], ADDS, args_ADDS_b_c)) {
                    
                    ListArgs<int> newargs_ADDS_MULS;
                    for (const auto& adds_arg : args_ADDS_b_c) {
                        ListArgs<int> newargs_MULS{args_MULS_a_ADDS_b_c};
                        newargs_MULS[i] = adds_arg;
                        newargs_ADDS_MULS.push_back(bank.get_normal_term(MULS, std::move(newargs_MULS)));
                    }

                    return bank.get_normal_term(ADDS, std::move(newargs_ADDS_MULS));
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

            ListArgs<int> args_ADDS_a_b;
            if (match_normal_head(args_CONJ_ADDS_a_b[0], ADDS, args_ADDS_a_b)) {
                ListArgs<int> newargs_ADDS_CONJ;
                for (const auto& arg : args_ADDS_a_b) {
                    newargs_ADDS_CONJ.push_back(bank.get_normal_term(CONJ, {arg}));
                }
                return bank.get_normal_term(ADDS, std::move(newargs_ADDS_CONJ));
            }
        }

        return std::nullopt;
    }

    // CONJ(MULS(a b)) -> MULS(CONJ(a) CONJ(b))
    REWRITE_COMPILED_DEF(R_CONJ3, bank, term) {
        ListArgs<int> args_CONJ_MULS_a_b;

        if (match_normal_head(term, CONJ, args_CONJ_MULS_a_b)) {
            ListArgs<int> args_MULS_a_b;
            if (match_normal_head(args_CONJ_MULS_a_b[0], MULS, args_MULS_a_b)) {
                ListArgs<int> newargs_MULS_CONJ;
                for (const auto& arg : args_MULS_a_b) {
                    newargs_MULS_CONJ.push_back(bank.get_normal_term(CONJ, {arg}));
                }
                return bank.get_normal_term(MULS, std::move(newargs_MULS_CONJ));
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
        R_FLATTEN,
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

    REWRITE_COMPILED_DEF(R_C_EQ, bank, term) {
        throw std::runtime_error("NOT CALLABLE");
    }

    const Term<int>* normalize(TermBank<int>& bank, const Term<int>* term, RewritingTrace<int>* trace) {

        // rewrite the term
        auto rewriten_term = static_cast<const NormalTerm<int>*>(rewrite_repeated(bank, term, scalar_rules, trace));

        // AC normalization
        auto res = sort_CInstruct(rewriten_term, bank, ac_symbols);

        trace->push_back({R_C_EQ, res.first, res.second});

        return res.first;
    }

    const map<RewritingRule<int>, string> scalar_rule_names = {
        {R_FLATTEN, "R_FLATTEN"},
        {R_ADDSID, "R_ADDSID"},
        {R_MULSID, "R_MULSID"},
        {R_ADDS0, "R_ADDS0"},
        {R_MULS0, "R_MULS0"},
        {R_MULS1, "R_MULS1"},
        {R_MULS2, "R_MULS2"},
        {R_CONJ0, "R_CONJ0"},
        {R_CONJ1, "R_CONJ1"},
        {R_CONJ2, "R_CONJ2"},
        {R_CONJ3, "R_CONJ3"},
        {R_CONJ4, "R_CONJ4"},
        {R_C_EQ, "R_C_EQ"}
    };

    string scalar_printer(const Signature<int>& sig, const RewritingRecord<int>& record) {
        std::string res = "";
        res += scalar_rule_names.at(record.rule) + " ";
        if (record.rule == R_C_EQ) {
            auto instruct = record.get_parameter<CProofInstruct>();
            res += instruct.to_string();
        }
        else {
            res += sig.term_to_string(record.term);
        }
        res += ".\n";
        return res;
    }

        

} // namespace scalar