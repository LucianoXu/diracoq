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
        (* Trace
        DNTr[M_, T_]:=Module[{i}, SUMS[IDX[{i, USET[T]}], Bra[{i}]\[SmallCircle]M\[SmallCircle]Ket[{i}]]];
        *)
        Def Tr := idx T => fun O : OTYPE[T, T] => Sum i in USET[T], <i| O |i>.

        (* SWAP
        SWAP[term_, T1_, T2_, T3_, T4_]:=
	Module[{i, j, k, l},
	SUMO[IDX[{i,USET[T1]}, {j,USET[T2]}, {k,USET[T3]}, {l,USET[T4]}],
		(Bra[{PAIR[i,j]}]\[SmallCircle]term\[SmallCircle]Ket[{PAIR[k,l]}])(Ket[{PAIR[j,i]}]\[SmallCircle]Bra[{PAIR[l,k]}])]];
        *)
        Def SWAP := idx T1 => idx T2 => idx T3 => idx T4 => fun O : OTYPE[T1 * T2, T3 * T4] => Sum i in USET[T1], Sum j in USET[T2], Sum k in USET[T3], Sum l in USET[T4], (<(i, j)| O |(k, l)>).(|(j, i)> <(l, k)|).

        (* Partial Trace 1
        DNPTr1[M_, T_, T1_, T2_]:= 
	Module[{i, j, k}, 
	SUMO[IDX[{i, USET[T1]}, {j, USET[T2]}, {k, USET[T]}], 
		(Bra[{PAIR[k, i]}]\[SmallCircle]M\[SmallCircle]Ket[{PAIR[k, j]}])(Ket[{i}]\[SmallCircle]Bra[{j}])]];
        *)
        Def DNPTr1 := idx T => idx T1 => idx T2 => fun O : OTYPE[T * T1, T * T2] => Sum i in USET[T1], Sum j in USET[T2], Sum k in USET[T], (<(k, i)| O |(k, j)>).(|i> <j|).

        (* Partial Trace 2
        DNPTr2[M_, T_, T1_, T2_]:=
	Module[{i, j, k},
	SUMO[IDX[{i, USET[T1]}, {j, USET[T2]}, {k, USET[T]}],
		(Bra[{PAIR[i, k]}]\[SmallCircle]M\[SmallCircle]Ket[{PAIR[j, k]}])(Ket[{i}]\[SmallCircle]Bra[{j}])]];
        *)
        Def DNPTr2 := idx T => idx T1 => idx T2 => fun O : OTYPE[T1 * T, T2 * T] => Sum i in USET[T1], Sum j in USET[T2], Sum k in USET[T], (<(i, k)| O |(j, k)>).(|i> <j|).

        (* Transpose of Ket
        TPK[B_, T_]:= Module[{i}, SUMK[IDX[{i, USET[T]}], (B\[SmallCircle]Ket[{i}])Ket[{i}]]];
        *)
        Def TPK := idx sigma => fun B : BTYPE[sigma] => Sum i in USET[sigma], (B |i>) . |i>.

        (* Transpose of Bra
        TPB[K_, T_]:= Module[{i}, SUMB[IDX[{i, USET[T]}], (Bra[{i}]\[SmallCircle]K)Bra[{i}]]];
        *)
        Def TPB := idx sigma => fun K : KTYPE[sigma] => Sum i in USET[sigma], (<i| K) . <i|.

        (* Transpose of Operator
        TPO[O_, T1_, T2_] := Module[{i, j}, 
	SUMO[IDX[{i, USET[T2]}, {j, USET[T1]}], (Ket[{i}]\[SmallCircle]Bra[{j}])\[SmallCircle]O\[SmallCircle](Ket[{i}]\[SmallCircle]Bra[{j}])]];
        *)
        Def TPO := idx sigma => idx tau => fun O : OTYPE[sigma, tau] => Sum i in USET[sigma], Sum j in USET[tau], (<i| O |j>).(|j> <i|).


        (* Conjugate of Operator 
        CONJO[O_, T1_, T2_] := TPO[ADJO[O],T2,T1];
        *)
        Def CONJO := idx sigma => idx tau => fun O : OTYPE[sigma, tau] => TPO tau sigma O^D.

        )");

        return res;
    }

} // namespace diracoq