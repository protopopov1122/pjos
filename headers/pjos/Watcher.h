/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#ifndef PJOS_WATCHER_H_
#define PJOS_WATCHER_H_

#include "pjos/Clause.h"
#include "pjos/Assignment.h"

// Watchers implement watched literal concept.
// Purpose of watcher is avoiding frequent clause rescans on assignments.
// Instead, it keeps track of current clause status and two literals
// that are representative of the clause.

namespace pjos {

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

        inline std::pair<std::int64_t, std::int64_t> WatchedLiterals() const {
            return this->watched_literals;
        }

        void Update(const Assignment &, Literal::UInt, VariableAssignment, bool); // Perform incremental update of clause status
                                                                            // The method needs to be called on every update of assignment
                                                                            // which involves the clause, otherwise full rescan is needed.
        void Rescan(const Assignment &); // Perform full clause rescan and update state

     private:
        std::int64_t FindUnassigned(const Assignment &, std::int64_t);
        bool IsSatisfied(const Assignment &, std::int64_t);
        bool IsUnsatisfied(const Assignment &, std::int64_t);
        void UpdateStatus(const Assignment &);

        ClauseView clause;
        ClauseStatus status;
        std::pair<std::int64_t, std::int64_t> watched_literals;
    };
}

#endif
