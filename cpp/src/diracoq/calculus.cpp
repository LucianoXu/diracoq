#include "diracoq.hpp"

namespace diracoq {

    using namespace std;
    using namespace ualg;

    std::optional<Declaration> Kernel::find_in_env(int symbol) {
        for (const auto& [sym, def] : env) {
            if (sym == symbol) {
                return def;
            }
        }
        return std::nullopt;
    }

    std::optional<Declaration> Kernel::find_dec(int symbol) {
        for (int i = ctx.size() - 1; i >= 0; i--) {
            if (ctx[i].first == symbol) {
                return ctx[i].second;
            }
        }
        for (const auto& [sym, def] : env) {
            if (sym == symbol) {
                return def;
            }
        }
        return std::nullopt;
    }

    std::string Kernel::dec_to_string(const std::string& name, const Declaration& dec) const {
        if (dec.is_def()) {
            return name + " := " + term_to_string(*dec.def) + " : " + term_to_string(dec.type);
        }
        else {
            return name + " : " + term_to_string(dec.type);
        }
    }


    TermPtr<int> Kernel::parse(const std::string& code) {
        auto ast = diracoq::parse(code);
        if (ast.has_value()) {
            return sig.ast2term(ast.value());
        }
        else{
            throw std::runtime_error("The code is not valid.");
        }
    }

    TermPtr<int> Kernel::parse(const astparser::AST& ast) {
        return sig.ast2term(ast);
    }

    string Kernel::term_to_string(TermPtr<int> term) const {
        return sig.term_to_string(term);
    }

    string Kernel::env_to_string() const {
        string res = "";
        for (const auto& [sym, def] : env) {
            res += dec_to_string(sig.get_name(sym), def) + "\n";
        }
        return res;
    }

    bool Kernel::is_index(TermPtr<int> term) {
        auto type = calc_type(term);

        if (type->get_head() == INDEX) {
            return true;
        }

        return false;
    }

    bool Kernel::is_type(TermPtr<int> term) {
        auto type = calc_type(term);

        if (type->get_head() == TYPE) {
            return true;
        }

        return false;
    }




    TermPtr<int> Kernel::calc_type(TermPtr<int> term) {
        auto head = term->get_head();
        auto args = term->get_args();

        // Preprocessing (COMPO rules)
        if (head == COMPO) {
            arg_number_check(args, 2);

            auto typeA = calc_type(args[0]);
            auto typeB = calc_type(args[1]);

            if (typeA->get_head() == STYPE) {
                // S @ S : S
                if (typeB->get_head() == STYPE) {
                    return create_term(STYPE);
                }
                // S @ K(A) : K(A)
                if (typeB->get_head() == KTYPE) {
                    return typeB;
                }
                // S @ B(A) : B(A)
                if (typeB->get_head() == BTYPE) {
                    return typeB;
                }
                // S @ O(A, B) : O(A, B)
                if (typeB->get_head() == OTYPE) {
                    return typeB;
                }
            }
            
            if (typeA->get_head() == KTYPE) {
                // K(A) @ S : K(A)
                if (typeB->get_head() == STYPE) {
                    return typeA;
                }
                // K(A) @ K(B) : K(PROD(A B))
                if (typeB->get_head() == KTYPE) {
                    return create_term(KTYPE, 
                        {create_term(PROD, {typeA->get_args()[0], typeB->get_args()[0]})});
                }
                // K(A) @ B(B) : O(A, B)
                if (typeB->get_head() == BTYPE) {
                    return create_term(OTYPE, {typeA->get_args()[0], typeB->get_args()[0]});
                }
                // K(A) @ O(B, C) --- NOT VALID
            }

            if (typeA->get_head() == BTYPE) {
                // B(A) @ S : B(A)
                if (typeB->get_head() == STYPE) {
                    return typeA;
                }
                // B(A) @ K(A) : S
                if (typeB->get_head() == KTYPE) {
                    if (is_judgemental_eq(typeA->get_args()[0], typeB->get_args()[0])) {
                        return create_term(STYPE);
                    }
                    else {
                        throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(typeA->get_args()[0]) + " of the first term is not equal to the argument " + sig.term_to_string(typeB->get_args()[0]) + " of the second term.");
                    }
                }
                // B(A) @ B(B) : B(PROD(A, B))
                if (typeB->get_head() == BTYPE) {
                    return create_term(BTYPE, 
                        {create_term(PROD, {typeA->get_args()[0], typeB->get_args()[0]})});
                }
                // B(A) @ O(A, B) : B(B)
                if (typeB->get_head() == OTYPE) {
                    if (is_judgemental_eq(typeA->get_args()[0], typeB->get_args()[0])) {
                        return create_term(BTYPE, {typeB->get_args()[1]});
                    }
                    else {
                        throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(typeA->get_args()[0]) + " of the first term is not equal to the argument " + sig.term_to_string(typeB->get_args()[0]) + " of the second term.");
                    }
                }   
            }
            if (typeA->get_head() == OTYPE) {
                // O(A, B) @ S : O(A, B)
                if (typeB->get_head() == STYPE) {
                    return typeA;
                }
                // O(A, B) @ K(B) : K(A)
                if (typeB->get_head() == KTYPE) {
                    if (is_judgemental_eq(typeA->get_args()[1], typeB->get_args()[0])) {
                        return create_term(KTYPE, {typeA->get_args()[0]});
                    }
                    else {
                        throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(typeA->get_args()[1]) + " of the first term is not equal to the argument " + sig.term_to_string(typeB->get_args()[0]) + " of the second term.");
                    }
                }
                // O(A, B) @ B(C) --- NOT VALID
                // O(A, B) @ O(B, C) : O(A, C)
                if (typeB->get_head() == OTYPE) {
                    if (is_judgemental_eq(typeA->get_args()[1], typeB->get_args()[0])) {
                        return create_term(OTYPE, {typeA->get_args()[0], typeB->get_args()[1]});
                    }
                    else {
                        throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(typeA->get_args()[1]) + " of the first term is not equal to the argument " + sig.term_to_string(typeB->get_args()[0]) + " of the second term.");
                    }
                }
            }
            // (T1 -> T2) @ T1 : T2
            if (typeA->get_head() == ARROW) {
                if (!is_judgemental_eq(typeA->get_args()[0], typeB)) {
                    throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(typeA->get_args()[0]) + " of the first term is not equal to the second term.");
                }
                return typeA->get_args()[1];
            }
            // (FORALL x, T) @ (t : INDEX) : T{x/t}
            if (typeA->get_head() == FORALL) {
                auto& args_typeA = typeA->get_args();
                if (!is_index(args[1])) {
                    throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the second term is not equal to the index.");
                }
                return subst(sig, args_typeA[1], args_typeA[0]->get_head(), args[1]);
            }

            throw std::runtime_error("Typing error: invalid composition of " + sig.term_to_string(typeA) + " and " + sig.term_to_string(typeB) + ".");
        }

        // Preprocessing (STAR rules)
        if (head == STAR) {
            if (args.size() < 2) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument number is less than 2.");
            }

            auto typeFirst = calc_type(args[0]);
            // STAR(S, S, S, ...) : S
            if (typeFirst->get_head() == STYPE) {
                for (int i = 1; i < args.size(); i++) {
                    auto typeI = calc_type(args[i]);
                    if (typeI->get_head() != STYPE) {
                        throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(args[i]) + " is not a scalar.");
                    }
                }
                return create_term(STYPE);
            }

            // STAR(INDEX INDEX) : INDEX
            if (typeFirst->get_head() == INDEX) {
                arg_number_check(args, 2);
                auto typeSecond = calc_type(args[1]);
                if (typeSecond->get_head() != INDEX) {
                    throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(args[1]) + " is not an index.");
                }
                return create_term(INDEX);
            }

            // STAR(O(a b) O(c d)) : O(PROD(a b) PROD(c d))
            if (typeFirst->get_head() == OTYPE) {
                arg_number_check(args, 2);
                auto typeSecond = calc_type(args[1]);
                if (typeSecond->get_head() != OTYPE) {
                    throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(args[1]) + " is not an operator.");
                }
                return create_term(OTYPE, 
                    {
                        create_term(PROD, {typeFirst->get_args()[0], typeSecond->get_args()[0]}),
                        create_term(PROD, {typeFirst->get_args()[1], typeSecond->get_args()[1]})
                    }
                );
            }

            // STAR(SET(a) SET(b)) : SET(PROD(a b))
            if (typeFirst->get_head() == SET) {
                arg_number_check(args, 2);
                auto typeSecond = calc_type(args[1]);
                if (typeSecond->get_head() != SET) {
                    throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(args[1]) + " is not a set.");
                }
                return create_term(SET, 
                    {
                        create_term(PROD, {typeFirst->get_args()[0], typeSecond->get_args()[0]})
                    }
                );
            }

            throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(args[0]) + " is not a scalar, an index, or a set.");
        }

        // Preprocessing (ADDG rules)
        if (head == ADDG) {
            if (args.size() < 2) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument number is less than 2.");
            }

            auto typeFirst = calc_type(args[0]);

            // ADDG(S, S, S, ...) : S
            if (typeFirst->get_head() == STYPE) {
                for (int i = 1; i < args.size(); i++) {
                    auto typeI = calc_type(args[i]);
                    if (typeI->get_head() != STYPE) {
                        throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(args[i]) + " is not a scalar.");
                    }
                }
                return create_term(STYPE);
            }

            // ADDG(K(a) K(a) ... K(a)) : K(a), ...
            if (typeFirst->get_head() == KTYPE || typeFirst->get_head() == BTYPE || typeFirst->get_head() == OTYPE) {
                for (int i = 1; i < args.size(); i++) {
                    auto typeI = calc_type(args[i]);
                    if (!is_judgemental_eq(typeFirst, typeI)) {
                        throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(args[i]) + " is not equal to the first argument " + sig.term_to_string(args[0]) + ".");
                    }
                }
                return typeFirst;
            }

            throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(args[0]) + " is not a scalar, a ket, a bra, or an operator.");
        }

        // SSUM(i set body) (type will be inferred)
        if (head == SSUM) {
            arg_number_check(args, 3);

            auto type_s = calc_type(args[1]);
            auto& args_s = type_s->get_args();
            if (type_s->get_head() != SET) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the first argument " + sig.term_to_string(args[0]) + " is not of type SET.");
            }

            // form the function temporarily
            auto func = create_term(FUN, 
                {
                    args[0], 
                    create_term(BASIS, {args_s[0]}),
                    args[2]
                }
            );

            auto type_f = calc_type(func);
            auto& args_f = type_f->get_args();
            if (type_f->get_head() != ARROW) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the function " + sig.term_to_string(func) + " is not of type ARROW.");
            }

            if (args_f[1]->get_head() == STYPE || args_f[1]->get_head() == KTYPE || args_f[1]->get_head() == BTYPE || args_f[1]->get_head() == OTYPE) {
                return args_f[1];
            }

            else {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the body " + sig.term_to_string(args[2]) + " is not a scalar, a ket, a bra, or an operator.");
            }
        }

        // (INDEX-PROD)
        if (head == PROD) {
            arg_number_check(args, 2);

            if (!is_index(args[0])) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the first argument " + sig.term_to_string(args[0]) + " is not an index.");
            }

            if (!is_index(args[1])) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the second argument " + sig.term_to_string(args[1]) + " is not an index.");
            }

            return create_term(INDEX);
        }
        
        // (INDEX-QBIT)
        if (head == QBIT) {
            arg_number_check(args, 0);
            return create_term(INDEX);
        }

        // (TYPE-BASIS0) (TYPE-BASIS1)
        if (head == BASIS0 || head == BASIS1) {
            arg_number_check(args, 0);
            return create_term(BASIS, {create_term(QBIT)});
        }

        // (TYPE-ARROW)
        if (head == ARROW) {
            arg_number_check(args, 2);

            if (!is_type(args[0])) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the type of the argument " + sig.term_to_string(args[0]) + " is not a well-typed type.");
            }
            if (!is_type(args[1])) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the type of the body " + sig.term_to_string(args[1]) + " is not a well-typed type.");
            }

            return create_term(TYPE);
        }

        // (TYPE-INDEX)
        if (head == FORALL) {
            arg_number_check(args, 2);

            if (!args[0]->is_atomic()) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the first argument " + sig.term_to_string(args[0]) + " is not a variable.");
            }

            context_push(args[0]->get_head(), create_term(INDEX));

            try {
                if (!is_type(args[1])) {
                    throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the type of the body " + sig.term_to_string(args[1]) + " is not a well-typed type.");
                }

                context_pop();
                return create_term(TYPE);
            }
            catch (const std::runtime_error& e) {
                context_pop();
                throw e;
            }
        }

        // (TYPE-BASIS) 
        if (head == BASIS) {
            arg_number_check(args, 1);

            if (!is_index(args[0])) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(args[0]) + " is not an index.");
            }
            return create_term(TYPE);
        }
        // (TYPE-Ket)
        if (head == KTYPE) {
            arg_number_check(args, 1);
            
            if (!is_index(args[0])) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(args[0]) + " is not an index.");
            }
            return create_term(TYPE);
        }
        // (TYPE-Bra)
        if (head == BTYPE) {
            arg_number_check(args, 1);

            if (!is_index(args[0])) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(args[0]) + " is not an index.");
            }
            return create_term(TYPE);
        }
        // (TYPE-Opt)
        if (head == OTYPE) {
            arg_number_check(args, 2);
            
            if (!(is_index(args[0]) && is_index(args[1]))) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the arguments " + sig.term_to_string(args[0]) + " and " + sig.term_to_string(args[1]) + " are not indices.");
            }
            return create_term(TYPE);
        }
        // (TYPE-Scalar)
        if (head == STYPE) {
            arg_number_check(args, 0);

            return create_term(TYPE);
        }
        // (TYPE-SET)
        if (head == SET) {
            arg_number_check(args, 1);

            if (!is_index(args[0])) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(args[0]) + " is not an index.");
            }

            return create_term(TYPE);
        }

        // (Lam)
        if (head == FUN) {
            arg_number_check(args, 3);

            if (!args[0]->is_atomic()) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the first argument " + sig.term_to_string(args[0]) + " is not a variable.");
            }

            if (!is_type(args[1])) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the type of the argument " + sig.term_to_string(args[0]) + " is not a well-typed type.");
            }

            context_push(args[0]->get_head(), args[1]);

            try {
                
                // calculate the type of the body
                auto type_body = calc_type(args[2]);

                if (!is_type(type_body)) {
                    throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the type of the body " + sig.term_to_string(args[2]) + " is not a well-typed type.");
                }

                context_pop();
                return create_term(ARROW, {args[1], type_body});
            }
            catch (const std::runtime_error& e) {
                // pop the definition
                context_pop();
                throw e;
            }
        }

        // (INDEX)
        if (head == IDX) {

            arg_number_check(args, 2);

            // try to add to the context
            context_push(args[0]->get_head(), create_term(INDEX));

            try {
                // calculate the type of the body
                auto type_body = calc_type(args[1]);

                if (!is_type(type_body)) {
                    throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the type of the body " + sig.term_to_string(args[1]) + " is not a well-typed type.");
                }

                context_pop();
                return create_term(FORALL, {args[0], type_body});
            }
            catch (const std::runtime_error& e) {
                // pop the definition
                context_pop();
                throw e;
            }
        }

        // (App-ARROW), (App-INDEX)
        if (head == APPLY) {
            arg_number_check(args, 2);


            // calculate the type of f, which should be ARROW(T, U)
            auto type_f = calc_type(args[0]);

            auto head_f = type_f->get_head();
            auto& args_f = type_f->get_args();
            if (head_f == ARROW) {
        
                // check whether the type of u matches
                if (!type_check(args[1], args_f[0])) {
                    throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the type of the argument " + sig.term_to_string(args[1]) + " does not match the type of the function argument " + sig.term_to_string(args_f[0]) + ".");
                }

                return args_f[1];
            }

            else if (head_f == FORALL) {
                // check whether the type of u is INDEX
                if (!is_index(args[1])) {
                    throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the type of the argument " + sig.term_to_string(args[1]) + " is not an index.");
                }

                // return the instantiated type
                return subst(sig, args_f[1], args_f[0]->get_head(), args[1]);
            }

            throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the type of the function " + sig.term_to_string(args[0]) + " is not an arrow type or forall type.");
        }

        // (Pair-Base)
        if (head == PAIR) {
            arg_number_check(args, 2);

            auto type_a = calc_type(args[0]);
            auto type_b = calc_type(args[1]);

            auto type_a_head = type_a->get_head();
            auto type_b_head = type_b->get_head();

            auto& args_a = type_a->get_args();
            auto& args_b = type_b->get_args();

            if (type_a_head != BASIS || type_b_head != BASIS) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the types of the arguments " + sig.term_to_string(args[0]) + " and " + sig.term_to_string(args[1]) + " are not of type BASIS.");
            }

            return create_term(BASIS, {create_term(PROD, {args_a[0], args_b[0]})});
        }

        // (Sca-0)
        if (head == ZERO) {
            arg_number_check(args, 0);

            return create_term(STYPE);
        }
        // (Sca-1)
        if (head == ONE) {
            arg_number_check(args, 0);
            
            return create_term(STYPE);
        }
        // (Sca-Delta)
        if (head == DELTA) {
            arg_number_check(args, 2);

            // Calculate the type of the first argument
            auto type_a = calc_type(args[0]);

            // Check whether it is Base
            if (type_a->get_head() != BASIS) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the first argument " + sig.term_to_string(args[0]) + " is not of type BASIS.");
            }

            // Check whether b is of type type_a
            if (!(type_check(args[1], type_a))) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the second argument " + sig.term_to_string(args[1]) + " is not of type " + sig.term_to_string(type_a) + ".");
            }
            
            return create_term(STYPE);
        }
        // (Sca-Add)
        if (head == ADDS) {
            auto SType_term = create_term(STYPE);
            if (args.size() == 0) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because it has no arguments.");
            }
            for (const auto& arg : args) {
                if (!type_check(arg, SType_term)) {
                    throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(arg) + " is not of type STYPE.");
                }
            }
            return SType_term;
        }
        // (Sca-Mul)
        if (head == MULS) {
            auto SType_term = create_term(STYPE);
            if (args.size() == 0) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because it has no arguments.");
            }
            for (const auto& arg : args) {
                if (!type_check(arg, SType_term)) {
                    throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(arg) + " is not of type STYPE.");
                }
            }
            return SType_term;
        }
        // (Sca-Conj)
        if (head == CONJ) {
            auto SType_term = create_term(STYPE);
            arg_number_check(args, 1);
            
            if (!type_check(args[0], SType_term)) {
                throw runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(args[0]) + " is not of type STYPE.");
            }
            return SType_term;
        }


        // (Ket-Adj), (Bra-Adj), (Opt-Adj)
        if (head == ADJ) {
            arg_number_check(args, 1);

            auto type_X = calc_type(args[0]);
            auto type_X_head = type_X->get_head();
            auto& args_X = type_X->get_args();
            if (type_X_head == BTYPE) {
                return create_term(KTYPE, {args_X[0]});
            }
            else if (type_X_head == KTYPE) {
                return create_term(BTYPE, {args_X[0]});
            }
            else if (type_X_head == OTYPE) {
                return create_term(OTYPE, {args_X[1], args_X[0]});
            }

            throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(args[0]) + " is not of type BTYPE, KTYPE or OTYPE.");            
        }

        // (Ket-Scr), (Bra-Scr), (Opt-Scr)
        if (head == SCR) {
            arg_number_check(args, 2);

            auto type_a = calc_type(args[0]);
            if (type_a->get_head() != STYPE) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the first argument " + sig.term_to_string(args[0]) + " is not of type STYPE.");
            }

            auto type_X = calc_type(args[1]);
            auto type_X_head = type_X->get_head();
            if (type_X_head == KTYPE || type_X_head == BTYPE || type_X_head == OTYPE) {
                return type_X;
            }

            throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the second argument " + sig.term_to_string(args[1]) + " is not of type KTYPE, BTYPE or OTYPE.");
        }

        // (Ket-Add), (Bra-Add), (Opt-Add)
        if (head == ADD) {
            if (args.size() == 0) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because it has no arguments.");
            }

            auto type_X = calc_type(args[0]);
            auto type_X_head = type_X->get_head();
            ListArgs<int> args_X;
            if (type_X_head != KTYPE && type_X_head != BTYPE && type_X_head != OTYPE) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the first argument " + sig.term_to_string(args[0]) + " is not of type BTYPE, KTYPE or OTYPE.");
            }

            for (int i = 1; i < args.size(); i++) {
                if (!is_judgemental_eq(calc_type(args[i]), type_X)) {
                    throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(args[i]) + " is not of the same type as the first argument " + sig.term_to_string(args[0]) + ".");
                }
            }

            return type_X;
        }

        // (Ket-Tsr), (Bra-Tsr), (Opt-Tsr)
        if (head == TSR) {
            arg_number_check(args, 2);

            auto type_X1 = calc_type(args[0]);
            auto type_X1_head = type_X1->get_head();
            auto& args_X1 = type_X1->get_args();
            auto type_X2 = calc_type(args[1]);
            auto type_X2_head = type_X2->get_head();
            auto& args_X2 = type_X2->get_args();
            // (Ket-Tsr)
            if (type_X1_head == KTYPE && type_X2_head == KTYPE) {
                return create_term(KTYPE, {create_term(PROD, {args_X1[0], args_X2[0]})});
            }
            // (Bra-Tsr)
            else if (type_X1_head == BTYPE && type_X2_head == BTYPE) {
                return create_term(BTYPE, {create_term(PROD, {args_X1[0], args_X2[0]})});
            }
            // (Opt-Tsr)
            else if (type_X1_head == OTYPE && type_X2_head == OTYPE) {
                return create_term(OTYPE, 
                    {
                        create_term(PROD, {args_X1[0], args_X2[0]}), 
                        create_term(PROD, {args_X1[1], args_X2[1]})
                    }
                );
            }
            
            throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the arguments " + sig.term_to_string(args[0]) + " and " + sig.term_to_string(args[1]) + " are not of type KTYPE, BTYPE or OTYPE.");
        }

        //
        if (head == DOT) {
            arg_number_check(args, 2);
            
            auto type_1 = calc_type(args[0]);
            auto& args_1 = type_1->get_args();
            auto type_2 = calc_type(args[1]);
            auto& args_2 = type_2->get_args();

            // (Sca-Dot)
            if (type_1->get_head() == BTYPE && type_2->get_head() == KTYPE) {
                if (!is_judgemental_eq(args_1[0], args_2[0])) {
                    throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the index of the first argument " + sig.term_to_string(args[0]) + " is not the same as the index of the second argument " + sig.term_to_string(args[1]) + ".");
                }
                return create_term(STYPE);
            }
            
            // (Ket-Mulk)
            if (type_1->get_head() == OTYPE && type_2->get_head() == KTYPE) {
                if (!is_judgemental_eq(args_1[1], args_2[0])) {
                    throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the second index of the first argument " + sig.term_to_string(args[0]) + " is not the same as the index of the second argument " + sig.term_to_string(args[1]) + ".");
                }
                return create_term(KTYPE, {args_1[0]});
            }

            // (Bra-Mulb)
            if (type_1->get_head() == BTYPE && type_2->get_head() == OTYPE) {
                if (!is_judgemental_eq(args_1[0], args_2[0])) {
                    throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the index of the first argument " + sig.term_to_string(args[0]) + " is not the same as the first index of the second argument " + sig.term_to_string(args[1]) + ".");
                }
                return create_term(BTYPE, {args_2[1]});
            }

            // (Opt-Outer)
            if (type_1->get_head() == KTYPE && type_2->get_head() == BTYPE) {
                return create_term(OTYPE, {args_1[0], args_2[0]});
            }

            // (Opt-Mulo)
            if (type_1->get_head() == OTYPE && type_2->get_head() == OTYPE) {
                if (!is_judgemental_eq(args_1[1], args_2[0])) {
                    throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the second index of the first argument " + sig.term_to_string(args[0]) + " is not the same as the index of the second argument " + sig.term_to_string(args[1]) + ".");
                }
                return create_term(OTYPE, {args_1[0], args_2[1]});
            }

            throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the arguments " + sig.term_to_string(args[0]) + " and " + sig.term_to_string(args[1]) + " are not of type KTYPE, BTYPE or OTYPE.");
        }

        // (Ket-0)
        if (head == ZEROK) {
            arg_number_check(args, 1);
            
            if (!is_index(args[0])) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(args[0]) + " is not an index.");
            }

            return create_term(KTYPE, {args[0]});
        }

        // (Ket-Base)
        if (head == KET) {
            arg_number_check(args, 1);

            auto type_t = calc_type(args[0]);
            auto& args_t = type_t->get_args();
            if (type_t->get_head() != BASIS) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(args[0]) + " is not of type Base.");
            }

            return create_term(KTYPE, {args_t[0]});
        }

        // (Bra-0)
        if (head == ZEROB) {
            arg_number_check(args, 1);

            if (!is_index(args[0])) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(args[0]) + " is not an index.");
            }

            return create_term(BTYPE, {args[0]});
        }

        // (Bra-Base)
        if (head == BRA) {
            arg_number_check(args, 1);

            auto type_t = calc_type(args[0]);
            auto& args_t = type_t->get_args();
            if (type_t->get_head() != BASIS) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(args[0]) + " is not of type Base.");
            }

            return create_term(BTYPE, {args_t[0]});
        }

        // (Opt-0)
        if (head == ZEROO) {
            arg_number_check(args, 2);

            if (!(is_index(args[0]) && is_index(args[1]))) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the arguments " + sig.term_to_string(args[0]) + " and " + sig.term_to_string(args[1]) + " are not indices.");
            }

            return create_term(OTYPE, {args[0], args[1]});
        }

        // (Opt-1)
        if (head == ONEO) {
            arg_number_check(args, 1);
            
            if (!is_index(args[0])) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(args[0]) + " is not an index.");
            }

            return create_term(OTYPE, {args[0], args[0]});
        }
        
        // (SET-U)
        if (head == USET) {
            arg_number_check(args, 1);

            if (!is_index(args[0])) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the argument " + sig.term_to_string(args[0]) + " is not an index.");
            }

            return create_term(SET, {args[0]});
        }

        // (SET-PROD)
        if (head == CATPROD) {
            arg_number_check(args, 2);

            auto type_a = calc_type(args[0]);
            auto type_b = calc_type(args[1]);

            auto& args_a = type_a->get_args();
            auto& args_b = type_b->get_args();
            if (type_a->get_head() != SET || type_b->get_head() != SET) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the arguments " + sig.term_to_string(args[0]) + " and " + sig.term_to_string(args[1]) + " are not of type SET.");
            }
            
            return create_term(
                SET, {
                    create_term(PROD, {args_a[0], args_b[0]})
                }
            );
        }

        // (Sum-Scalar), (Sum-Ket), (Sum-Bra), (Sum-Opt)
        if (head == SUM) {
            arg_number_check(args, 2);

            
            auto type_s = calc_type(args[0]);
            auto& args_s = type_s->get_args();
            if (type_s->get_head() != SET) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the first argument " + sig.term_to_string(args[0]) + " is not of type SET.");
            }

            auto type_f = calc_type(args[1]);
            auto& args_f = type_f->get_args();
            if (type_f->get_head() != ARROW) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the second argument " + sig.term_to_string(args[1]) + " is not of type ARROW.");
            }

            auto& args_f_basis = args_f[0]->get_args();
            if (args_f[0]->get_head() != BASIS) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the first argument of the second argument " + sig.term_to_string(args[1]) + " is not of type BASIS.");
            }

            if (!is_judgemental_eq(args_s[0], args_f_basis[0])) {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the index of the first argument " + sig.term_to_string(args[0]) + " is not the same as the index of the first argument of the second argument " + sig.term_to_string(args[1]) + ".");
            }

            if (args_f[1]->get_head() == STYPE || args_f[1]->get_head() == KTYPE || args_f[1]->get_head() == BTYPE || args_f[1]->get_head() == OTYPE) {
                return args_f[1];
            }

            else {
                throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' is not well-typed, because the second argument " + sig.term_to_string(args[1]) + " is not of type STYPE, KTYPE, BTYPE or OTYPE.");
            }
        }

        if (term->is_atomic()) {

            auto dec_find = find_dec(term->get_head());
            if (dec_find != std::nullopt) {
                return dec_find->type;
            }
        }

        // Other symbols will be considered as symbols in Wolfram Language and will be typed as STYPE
        return create_term(STYPE);

        // throw std::runtime_error("Typing error: the term '" + sig.term_to_string(term) + "' cannot be typed.");
    }

    void Kernel::assum(int symbol, TermPtr<int> type) {

        if (is_reserved(symbol)) {
            throw std::runtime_error("The symbol '" + sig.term_to_string(create_term(symbol)) + "' is reserved.");
        }
        if (find_in_env(symbol) != std::nullopt) {
            throw std::runtime_error("The symbol '" + sig.term_to_string(create_term(symbol)) + "' is already in the environment.");
        }

        // W-Assum-INDEX
        if (*type == Term<int>(INDEX)) {
            env.push_back({symbol, {std::nullopt, type}});
        }

        // W-Assum-TYPE
        else if (*type == Term<int>(TYPE)) {
            env.push_back({symbol, {std::nullopt, type}});
        }

        // W-Assum-Term
        else {
            if (!is_type(type)) {
                throw std::runtime_error("The type of the symbol '" + sig.term_to_string(create_term(symbol)) + "' is not a well-typed type.");
            }
            env.push_back({symbol, {std::nullopt, type}});
        }
    }

    void Kernel::def(int symbol, TermPtr<int> term, std::optional<TermPtr<int>> type) {
        if (is_reserved(symbol)) {
            throw std::runtime_error("The symbol '" + sig.term_to_string(create_term(symbol)) + "' is reserved.");
        }

        if (ctx.size() != 0) {
            throw std::runtime_error("The context is not empty.");
        }

        if (find_in_env(symbol) != std::nullopt) {
            throw std::runtime_error("The symbol '" + sig.term_to_string(create_term(symbol)) + "' is already in the environment."); 
        }

        auto deducted_type = calc_type(term);
        if (!is_type(deducted_type)) {
            throw std::runtime_error("The term '" + sig.term_to_string(term) + "' is not well-typed.");
        }

        if (type.has_value()) {
            if (!type_check(term, type.value())) {
                throw std::runtime_error("The term '" + sig.term_to_string(term) + "' is not well-typed with the type '" + sig.term_to_string(type.value()) + "'.");
            }
        }
        else {
            type = deducted_type;
        }
        env.push_back({symbol, {term, type.value()}});
    }

    void Kernel::env_pop() {
        if (env.size() == 0) {
            throw std::runtime_error("The environment is empty.");
        }
        env.pop_back();
    }


    void Kernel::context_push(int symbol, TermPtr<int> type) {
        if (!((type->get_head() == INDEX && type->get_args().size()) == 0 || is_type(type))) {

            throw std::runtime_error("The term '" + sig.term_to_string(type) + "' is not a valid type for bound index.");
        }

        ctx.push_back({symbol, {std::nullopt, type}});
    }

    void Kernel::context_pop() {
        if (ctx.size() == 0) {
            throw std::runtime_error("The context is empty.");
        }
        ctx.pop_back();
    }


    bool Kernel::is_judgemental_eq(TermPtr<int> termA, TermPtr<int> termB) {
        auto reduced_A = pos_rewrite_repeated(*this, termA, rules);
        auto reduced_B = pos_rewrite_repeated(*this, termB, rules);
        return is_eq(sig, reduced_A, reduced_B);
    }

    



} // namespace diracoq