#ifndef SAT_SOLVER_CLAUSE_CONTAINER_H_
#define SAT_SOLVER_CLAUSE_CONTAINER_H_

#include "sat_solver/Clause.h"
#include <unordered_map>

namespace sat_solver {

    class SharedClausePtr;

    class ClauseContainer {
        using ClauseID = std::size_t;
        struct Entry {
            Clause clause;
            std::size_t use_count{0};
        };

     public:
        ClauseContainer() = default;
        ClauseContainer(const ClauseContainer &) = delete;
        ClauseContainer(ClauseContainer &&) = delete;

        ~ClauseContainer() = default;

        ClauseContainer &operator=(const ClauseContainer &) = delete;
        ClauseContainer &operator=(ClauseContainer &&) = delete;

        SharedClausePtr Attach(Clause);

        friend class SharedClausePtr;
     
     private:
        void UseClause(ClauseID);
        void UnuseClause(ClauseID);

        std::unordered_map<ClauseID, Entry> clauses;
        ClauseID next_id{0};
    };

    class SharedClausePtr {
     public:
        SharedClausePtr() = delete;
        SharedClausePtr(const SharedClausePtr &);
        SharedClausePtr(SharedClausePtr &&);

        ~SharedClausePtr();

        SharedClausePtr &operator=(const SharedClausePtr &);
        SharedClausePtr &operator=(SharedClausePtr &&);

        inline const ClauseView &Get() const {
            return this->clause;
        }

        inline const ClauseView &operator*() const {
            return this->clause;
        }

        inline const ClauseView *operator->() const {
            return std::addressof(this->clause);
        }

        friend void swap(SharedClausePtr &, SharedClausePtr &);
        friend class ClauseContainer;

     private:
        SharedClausePtr(ClauseContainer &, ClauseContainer::ClauseID, ClauseView);

        ClauseContainer *container;
        ClauseContainer::ClauseID clause_id;
        ClauseView clause;
    };
}

#endif
