#include "sat_solver/ClauseContainer.h"
#include "sat_solver/Error.h"

namespace sat_solver {

    SharedClausePtr ClauseContainer::Attach(Clause clause) {
        auto clause_id = this->next_id++;
        auto [it, success] = this->clauses.emplace(clause_id, Entry{std::move(clause), 1});
        if (!success) {
            throw SatError{"Failed to attach clause to container"};
        }

        return SharedClausePtr{*this, clause_id, it->second.clause};
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

    SharedClausePtr::SharedClausePtr(ClauseContainer &container, ClauseContainer::ClauseID clause_id, ClauseView clause)
        : container{std::addressof(container)}, clause_id{clause_id}, clause{std::move(clause)} {}
    
    SharedClausePtr::SharedClausePtr(const SharedClausePtr &other)
        : container{other.container}, clause_id{other.clause_id}, clause{other.clause} {
        if (this->container != nullptr) {
            this->container->UseClause(this->clause_id);
        }
    }

    SharedClausePtr::SharedClausePtr(SharedClausePtr &&other)
        : container{other.container}, clause_id{other.clause_id}, clause{std::move(other.clause)} {
        other.container = nullptr;
    }

    SharedClausePtr::~SharedClausePtr() {
        if (this->container != nullptr) {
            this->container->UnuseClause(this->clause_id);
        }
    }

    SharedClausePtr &SharedClausePtr::operator=(const SharedClausePtr &other) {
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

    SharedClausePtr &SharedClausePtr::operator=(SharedClausePtr &&other) {
        if (this->container != nullptr) {
            this->container->UnuseClause(this->clause_id);
        }

        this->container = other.container;
        this->clause_id = other.clause_id;
        this->clause = std::move(other.clause);
        other.container = nullptr;
        return *this;
    }

    void swap(SharedClausePtr &ref1, SharedClausePtr &ref2) {
        std::swap(ref1.container, ref2.container);
        std::swap(ref1.clause_id, ref2.clause_id);
        std::swap(ref1.clause, ref2.clause);
    }
}