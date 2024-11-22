#include "diracoq.hpp"

namespace diracoq {

    bool Prover::process(const astparser::AST& ast) {
        // Group ( ... )
        try {
            if (ast.head == "Group") {
                for (const auto& cmd : ast.children) {
                    process(cmd);
                }
                return true;
            }
            else if (ast.head == "Def") {
                // Def(x t)
                if (ast.children.size() == 2) {
                    if (!check_id(ast.children[0])) return false;

                    auto name = ast.children[0].head;
                    auto term = kernel.parse(ast.children[1]);
                    kernel.gloabl_def(kernel.register_symbol(name), term);
                    return true;
                }
                // Def(x t T)
                if (ast.children.size() == 3) {
                    if (!check_id(ast.children[0])) return false;

                    auto name = ast.children[0].head;
                    auto term = kernel.parse(ast.children[1]);
                    auto type = kernel.parse(ast.children[2]);
                    kernel.gloabl_def(kernel.register_symbol(name), term, type);
                    return true;
                }
                else {
                    output << "Error: the definition is not valid." << std::endl;
                    output << "In the command: " << ast.to_string() << std::endl;
                    return false;
                }
            }
            else if (ast.head == "Var")  {
                // Assum(x T)
                if (ast.children.size() == 2) {
                    if (!check_id(ast.children[0])) return false;

                    
                    auto name = ast.children[0].head;
                    auto type = kernel.parse(ast.children[1]);
                    kernel.global_assum(kernel.register_symbol(name), type);
                    return true;
                }
                else {
                    output << "Error: the assumption is not valid." << std::endl;
                    output << "In the command: " << ast.to_string() << std::endl;
                    return false;
                }
            }
            else if (ast.head == "Check") {
                // Check(x)
                if (ast.children.size() == 1) {
                    if (!check_id(ast.children[0])) return false;

                    
                    auto name = ast.children[0].head;
                    auto term = kernel.parse(name);
                    try {
                        auto type = kernel.calc_type(term);
                        output << name << " : " << kernel.term_to_string(type) << std::endl;
                        return true;
                    }
                    catch (const std::exception& e) {
                        output << "Error: " << e.what() << std::endl;
                        return false;
                    }
                }
                // Check(x T)
                else if (ast.children.size() == 2) {
                    if (!check_id(ast.children[0])) return false;

                    
                    auto name = ast.children[0].head;
                    auto term = kernel.parse(name);
                    auto type = kernel.parse(ast.children[1]);
                    if (kernel.type_check(term, type)) {
                        output << name << " : " << kernel.term_to_string(type) << std::endl;
                        return true;
                    }
                    else {
                        output << "The term " << name << " is not well-typed with the type " << kernel.term_to_string(type) << std::endl;
                        return false;
                    }
                }
            }
            // ShowAll
            else if (ast.head == "ShowAll") {
                if (ast.children.size() == 0) {
                    output << "Environment:" << std::endl;
                    output << kernel.env_to_string() << std::endl;
                    output << "Context:" << std::endl;
                    output << kernel.context_to_string() << std::endl;
                    return true;
                }
                else {
                    output << "Error: the command is not valid." << std::endl;
                    output << "In the command: " << ast.to_string() << std::endl;
                    return false;
                }
            }
        }
        catch (const std::exception& e) {
            output << "Error: " << e.what() << std::endl;
            return false;
        }
        
        // bad command
        output << "Error: the command is not valid." << std::endl;
        output << "In the command: " << ast.to_string() << std::endl;
        return false;
    }

} // namespace diracoq