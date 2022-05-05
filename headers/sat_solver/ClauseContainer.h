#ifndef SAT_SOLVER_CLAUSE_CONTAINER_H_
#define SAT_SOLVER_CLAUSE_CONTAINER_H_

#include "sat_solver/Clause.h"
#include <unordered_map>

namespace sat_solver {

    class ClauseRef;

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

        ClauseRef Attach(Clause);

        friend class ClauseRef;
     
     private:
        void UseClause(ClauseID);
        void UnuseClause(ClauseID);

        std::unordered_map<ClauseID, Entry> clauses;
        ClauseID next_id{0};
    };

    class ClauseRef {
     public:
        ClauseRef() = delete;
        ClauseRef(const ClauseRef &);
        ClauseRef(ClauseRef &&);

        ~ClauseRef();

        ClauseRef &operator=(const ClauseRef &);
        ClauseRef &operator=(ClauseRef &&);

        inline const ClauseView &Get() const {
            return this->clause;
        }

        inline const ClauseView &operator*() const {
            return this->clause;
        }

        inline const ClauseView *operator->() const {
            return std::addressof(this->clause);
        }

        friend void swap(ClauseRef &, ClauseRef &);
        friend class ClauseContainer;

     private:
        ClauseRef(ClauseContainer &, ClauseContainer::ClauseID, ClauseView);

        ClauseContainer *container;
        ClauseContainer::ClauseID clause_id;
        ClauseView clause;
    };
}

#endif
