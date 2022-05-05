#ifndef SAT_SOLVER_CLAUSE_H_
#define SAT_SOLVER_CLAUSE_H_

#include "sat_solver/Literal.h"
#include <memory>
#include <set>

namespace sat_solver {

    class ClauseView {
     public:
        ClauseView() = delete;
        ClauseView(const ClauseView &) = default;
        ClauseView(ClauseView &&) = default;

        ~ClauseView() = default;

        ClauseView &operator=(const ClauseView &) = default;
        ClauseView &operator=(ClauseView &&) = default;

        bool HasVariable(Literal::Int) const;
        bool HasLiteral(Literal) const;
        std::size_t Length() const;
        Literal::Int NumOfVariables() const;

        Literal At(std::size_t) const;

        const Literal *begin() const;
        const Literal *end() const;
    
        friend void swap(ClauseView &, ClauseView &);

     protected:
        ClauseView(const Literal *, std::size_t, Literal::Int);

        const Literal *clause;
        std::size_t clause_length;
        Literal::Int num_of_variables;
    };

    class Clause : public ClauseView {
     public:
        Clause(const Clause &);
        Clause(Clause &&) = default;

        ~Clause() = default;

        Clause &operator=(const Clause &);
        Clause &operator=(Clause &&) = default;

        ClauseView View() const;

        friend void swap(Clause &, Clause &);
        friend class ClauseBuilder;

     private:
        Clause(std::unique_ptr<Literal[]>, std::size_t, Literal::Int);

        std::unique_ptr<Literal[]> clause;
    };

    class ClauseBuilder {
     public:
        ClauseBuilder() = default;
        ClauseBuilder(const ClauseBuilder &) = delete;
        ClauseBuilder(ClauseBuilder &&) = delete;

        ~ClauseBuilder() = default;

        ClauseBuilder &operator=(const ClauseBuilder &) = delete;
        ClauseBuilder &operator=(ClauseBuilder &&) = delete;

        ClauseBuilder &Add(Literal);
        ClauseBuilder &Reset();
        Clause Make();

     private:
        std::set<Literal> literals;
        Literal::Int num_of_variables{Literal::Terminator};
    };
}

#endif
