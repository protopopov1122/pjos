/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#ifndef PJOS_DECISION_TRAIL_H_
#define PJOS_DECISION_TRAIL_H_

#include "pjos/Assignment.h"
#include <functional>
#include <optional>
#include <vector>

// The decision trail keeps track of decisions, assumptions and propagations
// that happened in the process of solving. Each assignment has associated
// assignment information (variable and value), reason or level (number of decisions
// prior to the assignment). Decision trail primarly works as a stack, however
// can also be addressed in read-only mode for analysis purposes.
// Whenever the number of variables in the formula changes,
// decision trail needs to be notified about the change.

namespace pjos {

    class DecisionTrail {
     public:
        using Reason = std::int_fast64_t;
        struct Entry {
            Entry(Literal::UInt, VariableAssignment, Reason, std::size_t);

            Literal::UInt variable;
            VariableAssignment assignment;
            Reason reason;
            std::size_t level;
        };

        DecisionTrail(std::size_t);
        DecisionTrail(const DecisionTrail &) = default;
        DecisionTrail(DecisionTrail &&) = default;

        ~DecisionTrail() = default;

        DecisionTrail &operator=(const DecisionTrail &) = default;
        DecisionTrail &operator=(DecisionTrail &&) = default;

        inline std::size_t Level() const {
            return this->level;
        }

        void Decision(Literal::UInt, VariableAssignment);
        void Propagation(Literal::UInt, VariableAssignment);
        void Propagation(Literal::UInt, VariableAssignment, std::size_t);
        void Assumption(Literal::UInt, VariableAssignment);
        const Entry *Top();
        void Pop();
        void SetNumOfVariables(std::size_t);
        void Reset();

        const Entry *Find(Literal::UInt) const; // Find whether decision trail contains assignment of a variable

        inline std::size_t Length() const {
            return this->trail.size();
        }

        inline const Entry &operator[](std::size_t index) const {
            return this->trail[index];
        }

        // Assignment reason
        static constexpr Reason ReasonAssumption{-3};  // For assumptions that were provided by the user
        static constexpr Reason ReasonPropagation{-2}; // For propagations without any specific reference to reason clause
        static constexpr Reason ReasonDecision{-1}; // For decisions made by solver

        // Other non-negative reasons are considered to be reference to clauses
        static constexpr bool IsPropagatedFromClause(Reason reason) {
            return reason >= 0;
        }

     private:
        std::vector<Entry> trail{};
        std::size_t level{0};
        std::vector<std::size_t> var_index;
    };
}

#endif
