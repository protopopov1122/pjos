/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#ifndef PJOS_ASSIGNMENT_H_
#define PJOS_ASSIGNMENT_H_

#include "pjos/Literal.h"
#include <utility>
#include <vector>

// Assignment contains information of current variable states.
// Each variable can be true, false or unassigned. Assignment
// supports read-write access, as well as traversal of variables
// in assignment. Whenever the number of variables in formula changes,
// assignment needs to be notified of the change.

namespace pjos {

    namespace internal {
        
        template <typename I>
        class AssignmentIterator {
            using difference_type = typename std::iterator_traits<AssignmentIterator<I>>::difference_type;

         public:
            AssignmentIterator(I iter, Literal::UInt variable)
                : iter{std::move(iter)}, variable{variable} {}
            
            AssignmentIterator(const AssignmentIterator &) = default;
            AssignmentIterator(AssignmentIterator &&) = default;

            ~AssignmentIterator() = default;

            AssignmentIterator &operator=(const AssignmentIterator &) = default;
            AssignmentIterator &operator=(AssignmentIterator &&) = default;

            AssignmentIterator<I> &operator++() {
                std::advance(this->iter, 1);
                this->variable++;
                return *this;
            }

            AssignmentIterator<I> &operator--() {
                std::advance(this->iter, -1);
                this->variable--;
                return *this;
            }

            AssignmentIterator<I> operator++(int) {
                auto clone(*this);
                this->operator++();
                return clone;
            }

            AssignmentIterator<I> operator--(int) {
                auto clone(*this);
                this->operator--();
                return clone;
            }

            AssignmentIterator<I> &operator+=(difference_type distance) {
                std::advance(this->iter, distance);
                this->variable += distance;
                return *this;
            }

            AssignmentIterator<I> &operator-=(difference_type distance) {
                std::advance(this->iter, -distance);
                this->variable -= distance;
                return *this;
            }

            AssignmentIterator<I> operator+(difference_type distance) const {
                return AssignmentIterator{std::next(this->iter, distance), this->variable + distance};
            }

            AssignmentIterator<I> operator-(difference_type distance) const {
                return AssignmentIterator{std::prev(this->iter, distance), this->variable - distance};
            }

            difference_type operator-(const AssignmentIterator<I> &other) const {
                return std::distance(this->iter, other.iter);
            }

            bool operator==(const AssignmentIterator<I> &other) const {
                return this->iter == other.iter;
            }

            bool operator!=(const AssignmentIterator<I> &other) const {
                return this->iter != other.iter;
            }

            std::pair<Literal::UInt, VariableAssignment> operator*() const {
                return std::make_pair(this->variable, *this->iter);
            }

            friend void swap(AssignmentIterator<I> &it1, AssignmentIterator<I> &it2) {
                std::swap(it1.iter, it2.iter);
                std::swap(it1.variable, it2.value_type);
            }

         private:
            I iter;
            Literal::UInt variable;
        };
    }

    class Assignment {
     public:
        using IteratorType = internal::AssignmentIterator<std::vector<VariableAssignment>::const_iterator>;

        Assignment(std::size_t);
        Assignment(const Assignment &) = default;
        Assignment(Assignment &&) = default;

        ~Assignment() = default;

        Assignment &operator=(const Assignment &) = default;
        Assignment &operator=(Assignment &&) = default;

        inline std::size_t NumOfVariables() const {
            return this->assignment.size();
        }

        inline VariableAssignment Of(std::size_t index) const {
            return this->assignment.at(index - 1);
        }

        inline Assignment &Set(std::size_t index, VariableAssignment assn) {
            this->assignment.at(index - 1) = assn;
            return *this;
        }

        inline VariableAssignment operator[](std::size_t index) const {
            return this->assignment[index - 1];
        }

        inline VariableAssignment &operator[](std::size_t index) {
            return this->assignment[index - 1];
        }

        IteratorType begin() const;
        IteratorType end() const;

        Assignment &Reset();
        void SetNumOfVariables(std::size_t);

     private:
        std::vector<VariableAssignment> assignment;
    };
}

template <typename I>
struct std::iterator_traits<pjos::internal::AssignmentIterator<I>> {
    using difference_type = std::ptrdiff_t;
    using value_type = std::pair<pjos::Literal::UInt, pjos::VariableAssignment>;
    using iterator_category = std::bidirectional_iterator_tag;
};

#endif
