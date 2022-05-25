#ifndef PJOS_DECISION_TRAIL_H_
#define PJOS_DECISION_TRAIL_H_

#include "pjos/Assignment.h"
#include <functional>
#include <optional>
#include <vector>

namespace pjos {

    class DecisionTrail {
     public:
        using Reason = std::int64_t;
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

        const Entry *Find(Literal::UInt) const;

        inline std::size_t Length() const {
            return this->trail.size();
        }

        inline const Entry &operator[](std::size_t index) const {
            return this->trail[index];
        }

        static constexpr Reason ReasonAssumption{-3};
        static constexpr Reason ReasonPropagation{-2};
        static constexpr Reason ReasonDecision{-1};

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
