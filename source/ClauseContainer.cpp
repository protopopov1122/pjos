#include "sat_solver/ClauseContainer.h"
#include "sat_solver/Error.h"

namespace sat_solver {

    ClauseRef ClauseContainer::Attach(Clause clause) {
        auto clause_id = this->next_id++;
        auto [it, success] = this->clauses.emplace(clause_id, Entry{std::move(clause), 1});
        if (!success) {
            throw SatError{"Failed to attach clause to container"};
        }

        return ClauseRef{*this, clause_id, it->second.clause};
    }

    void ClauseContainer::UseClause(ClauseID clause_id) {
        auto it = this->clauses.find(clause_id);
        if (it == this->clauses.end()) {
            throw SatError{SatErrorCode::InvalidParameter, "Unknown clause ID"};
        }
        it->second.use_count++;
    }

    void ClauseContainer::UnuseClause(ClauseID clause_id) {
        auto it = this->clauses.find(clause_id);
        if (it == this->clauses.end()) {
            throw SatError{SatErrorCode::InvalidParameter, "Unknown clause ID"};
        }

        if (--it->second.use_count == 0) {
            this->clauses.erase(it);
        }
    }

    ClauseRef::ClauseRef(ClauseContainer &container, ClauseContainer::ClauseID clause_id, ClauseView clause)
        : container{std::addressof(container)}, clause_id{clause_id}, clause{std::move(clause)} {}
    
    ClauseRef::ClauseRef(const ClauseRef &other)
        : container{other.container}, clause_id{other.clause_id}, clause{other.clause} {
        if (this->container != nullptr) {
            this->container->UseClause(this->clause_id);
        }
    }

    ClauseRef::ClauseRef(ClauseRef &&other)
        : container{other.container}, clause_id{other.clause_id}, clause{std::move(other.clause)} {
        other.container = nullptr;
    }

    ClauseRef::~ClauseRef() {
        if (this->container != nullptr) {
            this->container->UnuseClause(this->clause_id);
        }
    }

    ClauseRef &ClauseRef::operator=(const ClauseRef &other) {
        if (this->container != nullptr) {
            this->container->UnuseClause(this->clause_id);
        }

        this->container = other.container;
        this->clause_id = other.clause_id;
        this->clause = other.clause;
        if (this->container != nullptr) {
            this->container->UseClause(this->clause_id);
        }
        return *this;
    }

    ClauseRef &ClauseRef::operator=(ClauseRef &&other) {
        if (this->container != nullptr) {
            this->container->UnuseClause(this->clause_id);
        }

        this->container = other.container;
        this->clause_id = other.clause_id;
        this->clause = std::move(other.clause);
        other.container = nullptr;
        return *this;
    }

    void swap(ClauseRef &ref1, ClauseRef &ref2) {
        std::swap(ref1.container, ref2.container);
        std::swap(ref1.clause_id, ref2.clause_id);
        std::swap(ref1.clause, ref2.clause);
    }
}