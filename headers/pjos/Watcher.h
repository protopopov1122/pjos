/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#ifndef PJOS_WATCHER_H_
#define PJOS_WATCHER_H_

#include "pjos/Clause.h"
#include "pjos/Assignment.h"

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

        void Update(const Assignment &, Literal::UInt);
        void Rescan(const Assignment &);

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
