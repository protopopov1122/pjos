#ifndef SAT_SOLVER_WATCHER_H_
#define SAT_SOLVER_WATCHER_H_

#include "sat_solver/Clause.h"
#include "sat_solver/Assignment.h"

namespace sat_solver {

    enum class ClauseStatus {
        Satisfied,
        Unsatisfied,
        Unit,
        Undecided
    };

    class Watcher {
     public:
        Watcher(const ClauseView &);
        Watcher(const Watcher &) = default;
        Watcher(Watcher &&) = default;

        ~Watcher() = default;

        Watcher &operator=(const Watcher &) = default;
        Watcher &operator=(Watcher &&) = default;

        inline ClauseStatus Status() const {
            return this->status;
        }

        inline std::pair<std::int64_t, std::int64_t> GetWatchedLiteralIndices() const {
            return this->watched_literals;
        }

        void Update(const Assignment &);

     private:
        ClauseView clause;
        ClauseStatus status;
        std::pair<std::int64_t, std::int64_t> watched_literals;
    };
}

#endif
