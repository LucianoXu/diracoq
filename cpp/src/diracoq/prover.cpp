#include "diracoq.hpp"

namespace diracoq {
    using namespace std;
    using namespace ualg;

    bool Prover::process(const astparser::AST& ast) {
        // GROUP ( ... )
        try {
            if (ast.head == "GROUP") {
                for (const auto& cmd : ast.children) {
                    if (!process(cmd)){
                        return false;
                    }
                }
                return true;
            }
            else if (ast.head == "DEF") {
                // DEF(x t)
                if (ast.children.size() == 2) {
                    if (!check_id(ast.children[0])) return false;

                    auto name = ast.children[0].head;
                    auto term = kernel.parse(ast.children[1]);
                    kernel.def(kernel.register_symbol(name), term);

                    // generate Coq code
                    if (gen_coq) {
                        append_coq_code("(* " + ast.to_string() + " *)\n");
                        append_coq_code("Definition " + name + " := " + term_to_coq(kernel, term) + ".\n\n");
                    }

                    return true;
                }
                // DEF(x t T)
                if (ast.children.size() == 3) {
                    if (!check_id(ast.children[0])) return false;

                    auto name = ast.children[0].head;
                    auto term = kernel.parse(ast.children[1]);
                    auto type = kernel.parse(ast.children[2]);
                    kernel.def(kernel.register_symbol(name), term, type);

                    // generate Coq code
                    if (gen_coq) {
                        append_coq_code("(* " + ast.to_string() + " *)\n");
                        append_coq_code("Definition " + name + " : " + term_to_coq(kernel, type) + " := " + term_to_coq(kernel, term) + ".\n\n");
                    }

                    return true;
                }
                else {
                    output << "Error: the definition is not valid." << endl;
                    output << "In the command: " << ast.to_string() << endl;
                    return false;
                }
            }
            else if (ast.head == "VAR")  {
                // Assum(x T)
                if (ast.children.size() == 2) {
                    if (!check_id(ast.children[0])) return false;

                    
                    auto name = ast.children[0].head;
                    auto type = kernel.parse(ast.children[1]);
                    kernel.assum(kernel.register_symbol(name), type);

                    // generate Coq code
                    if (gen_coq) {
                        append_coq_code("(* " + ast.to_string() + " *)\n");
                        append_coq_code("Variable " + name + " : " + term_to_coq(kernel, type) + ".\n\n");
                    }

                    return true;
                }
                else {
                    output << "Error: the assumption is not valid." << endl;
                    output << "In the command: " << ast.to_string() << endl;
                    return false;
                }
            }
            else if (ast.head == "CHECK") {
                // CHECK(x)
                if (ast.children.size() == 1) {
                    try {
                        auto term = kernel.parse(ast.children[0]);
                        auto type = kernel.calc_type(term);
                        output << kernel.term_to_string(term) << " : " << kernel.term_to_string(type) << endl;

                        // generate Coq code
                        if (gen_coq) {
                            append_coq_code("(* " + ast.to_string() + " *)\n");
                            append_coq_code("CHECK " + term_to_coq(kernel, term) + ".\n\n");
                        }
                        
                        return true;
                    }
                    catch (const exception& e) {
                        output << "Error: " << e.what() << endl;
                        return false;
                    }
                }
            }
            // SHOW(x)
            else if (ast.head == "SHOW") {
                if (ast.children.size() == 1) {
                    if (!check_id(ast.children[0])) return false;

                    auto name = ast.children[0].head;
                    try {
                        // get the definition in the env
                        auto find_def = kernel.find_in_env(kernel.register_symbol(name));
                        if (find_def == nullopt) {
                            output << "Error: the symbol '" << name << "' is not defined." << endl;
                            return false;
                        }
                        output << kernel.dec_to_string(name, find_def.value()) << endl;

                        // generate Coq code
                        if (gen_coq) {
                            append_coq_code("(* " + ast.to_string() + " *)\n");
                            append_coq_code("Print " + name + ".\n\n");
                        }

                        return true;
                    }
                    catch (const exception& e) {
                        output << "Error: " << e.what() << endl;
                        return false;
                    }
                }
            }
            // SHOWALL
            else if (ast.head == "SHOWALL") {
                if (ast.children.size() == 0) {
                    output << "Environment:" << endl;
                    output << kernel.env_to_string() << endl;

                    // generate Coq code
                    if (gen_coq) {
                        append_coq_code("(* " + ast.to_string() + " *)\n");
                        append_coq_code("Print All.\n\n");
                    }
                    
                    return true;
                }
                else {
                    output << "Error: the command is not valid." << endl;
                    output << "In the command: " << ast.to_string() << endl;
                    return false;
                }
            }
            else if (ast.head == "NORMALIZE") {
                if (ast.children.size() > 2) {
                    output << "Error: NORMALIZE command should have one or two arguments." << endl;
                    return false;
                }
                if (ast.children.size() == 2) {
                    if (ast.children[1].head != "TRACE") {
                        output << "Error: the second argument of NORMALIZE command should be 'TRACE'." << endl;
                        return false;
                    }
                }

                // Typecheck the term
                auto term = kernel.parse(ast.children[0]);
                auto type = kernel.calc_type(term);

                // calculate the normalized term
                vector<PosReplaceRecord> trace;

                try {
                    auto temp = pos_rewrite_repeated(kernel, term, all_rules, &trace);

                    // expand on variables
                    auto expanded_term = variable_expand(kernel, temp);
                    temp = pos_rewrite_repeated(kernel, expanded_term, rules, &trace);

                    auto normalized_term = deBruijn_normalize(kernel, temp);
                    auto [sorted_term, instruct] = sort_CInstruct(normalized_term, kernel.get_bank(), c_symbols);

                    auto final_term = normalized_term;

                    // if output trace
                    if (ast.children.size() == 2) {
                        output << "Trace:" << endl;
                        for (int i = 0; i < trace.size(); ++i) {
                            output << "Step " + to_string(i) << endl;
                            output << kernel.term_to_string(trace[i].init_term) << endl;
                            output << pos_replace_record_to_string(kernel, trace[i]) << endl;
                        }
                    }
                    
                    // Output the normalized term
                    output << kernel.term_to_string(final_term) + " : " + kernel.term_to_string(type)  << endl;


                    // generate Coq code
                    if (gen_coq) {
                        append_coq_code("(* " + ast.to_string() + " *)\n");
                        append_coq_code(normalize_to_coq(kernel, term, final_term, trace, instruct) + "\n\n");
                    }

                    return true;

                }
                catch (const exception& e) {
                    // output the trace first
                    output << "Trace:" << endl;
                    for (int i = 0; i < trace.size(); ++i) {
                        output << "Step " + to_string(i) << endl;
                        output << kernel.term_to_string(trace[i].init_term) << endl;
                        output << pos_replace_record_to_string(kernel, trace[i]) << endl;
                    }

                    throw;
                }

            }
            else if (ast.head == "CHECKEQ") {
                if (ast.children.size() != 2) {
                    output << "Error: CHECKEQ command should have two arguments." << endl;
                    return false;
                }
                check_eq(ast.children[0], ast.children[1]);
                return true;
            }
        }
        catch (const exception& e) {
            output << "Error: " << e.what() << endl;
            return false;
        }

        // bad command
        output << "Error: the command is not valid." << endl;
        output << "In the command: " << ast.to_string() << endl;
        return false;
    }

    bool Prover::check_eq(const astparser::AST& codeA, const astparser::AST& codeB) {
        
        // Typecheck the terms
        auto termA = kernel.parse(codeA);
        auto termB = kernel.parse(codeB);
        auto typeA = kernel.calc_type(termA);
        auto typeB = kernel.calc_type(termB);
        if (typeA != typeB) {
            output << "The two terms have different types and are not equal." << endl;
            output << "[TYPE A] " << kernel.term_to_string(typeA) << endl;
            output << "[TYPE B] " << kernel.term_to_string(typeB) << endl;
            return false;
        }

        // calculate the normalized term
        vector<PosReplaceRecord> traceA;
        auto tempA = pos_rewrite_repeated(kernel, termA, all_rules, &traceA);
        auto expanded_termA = variable_expand(kernel, tempA);
        tempA = pos_rewrite_repeated(kernel, expanded_termA, rules, &traceA);
        auto normalized_termA = deBruijn_normalize(kernel, tempA);
        auto [sorted_termA, instructA] = sort_CInstruct(normalized_termA, kernel.get_bank(), c_symbols);

        vector<PosReplaceRecord> traceB;
        auto tempB = pos_rewrite_repeated(kernel, termB, all_rules, &traceB);
        auto expanded_termB = variable_expand(kernel, tempB);
        tempB = pos_rewrite_repeated(kernel, expanded_termB, rules, &traceB);
        auto normalized_termB = deBruijn_normalize(kernel, tempB);
        auto [sorted_termB, instructB] = sort_CInstruct(normalized_termB, kernel.get_bank(), c_symbols);

        auto final_termA = sorted_termA;
        auto final_termB = sorted_termB;
        
        // Output the result
        if (final_termA == final_termB) {
            output << "The two terms are equal." << endl;
            output << "[Normalized Term] " << kernel.term_to_string(final_termA) << " : " << kernel.term_to_string(typeA) << endl;


            // generate Coq code
            if (gen_coq) {
                auto original_code = astparser::AST("CHECKEQ", {codeA, codeB});
                append_coq_code("(* " + original_code.to_string() + " *)\n");
                append_coq_code(checkeq_to_coq(kernel, termA, termB, traceA, traceB, instructA, instructB, final_termA) + "\n\n");
            }

            return true;
        }
        else {
            output << "The two terms are not equal." << endl;
            output << "[Normalized Term A] " << kernel.term_to_string(final_termA) << " : " << kernel.term_to_string(typeA) << endl;
            output << "[Normalized Term B] " << kernel.term_to_string(final_termB) << " : " << kernel.term_to_string(typeB) << endl;
            return false;
        }
    }

    Prover* std_prover() {
        auto res = new Prover{std::cout, false};

        res->process(R"(
        (* Trace *)
        Def Tr := idx T => fun O : OTYPE(T T) => Sum i : BASIS(T) in USET(T), <i| @ O @ |i>.

        Def TPB := idx sigma => fun K : KTYPE(sigma) => Sum i : BASIS(sigma) in USET(sigma), ((<i| @ K) . <i|).

        Def TPK := idx sigma => fun B : BTYPE(sigma) => Sum i : BASIS(sigma) in USET(sigma), SCR(DOT(B KET(i)) KET(i)).

        Def TPO := idx sigma => idx tau => fun O : OTYPE(sigma tau) => Sum i : BASIS(sigma) in USET(sigma), Sum j : BASIS(tau) in USET(tau), (<i| @ (O @ |j>)).(|j> @ <i|).
        )");

        return res;
    }

} // namespace diracoq