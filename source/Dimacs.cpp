#include "sat_solver/Dimacs.h"
#include <iostream>
#include <string>
#include <regex>

namespace sat_solver {

    static std::regex CommentRegex{R"(^\s*c.*$)"};
    static std::regex ProblemRegex{R"(^\s*p\s+cnf\s+([0-9]+)\s+([0-9]+)\s*$)"};
    static std::regex LiteralRegex{R"((-?[0-9]+))"};

    enum class LoaderMode {
        Comments,
        Literals
    };

    DimacsLoader::DimacsLoader(std::istream &is)
        : input{is} {}
    
    void DimacsLoader::LoadInto(Formula &formula) {
        FormulaBuilder builder{formula};
        std::string buffer{};
        std::smatch match;
        LoaderMode lmode{LoaderMode::Comments};

        while (this->input.good() && !this->input.eof()) {
            std::getline(this->input, buffer);
            if (lmode == LoaderMode::Comments && std::regex_match(buffer, match, CommentRegex)) {
                // Skip comment
            } else if (lmode == LoaderMode::Comments && std::regex_match(buffer, match, ProblemRegex)) {
                // Ignore problem parameters and switch mode
                lmode = LoaderMode::Literals;
            } else {
                for (auto it = std::sregex_iterator(buffer.begin(), buffer.end(), LiteralRegex); it != std::sregex_iterator(); it = std::next(it)) {
                    auto lit = std::strtoll(it->str().c_str(), nullptr, 10);
                    if (lit == 0) {
                        builder.EndClause();
                    } else {
                        builder.AppendLiteral(lit);
                    }
                }
            }
        }
    }
}