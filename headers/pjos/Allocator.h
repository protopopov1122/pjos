/*
    SPDX-License-Identifier: MIT
    SPDX-FileCopyrightText: 2022 Protopopovs Jevgenijs
*/

#ifndef PJOS_ALLOCATOR_H_
#define PJOS_ALLOCATOR_H_

#include "pjos/Clause.h"
#include <array>
#include <vector>

namespace pjos {

    template <std::size_t ChunkSize = 512>
    class LiteralStackAllocator {
        static_assert(ChunkSize > 0);

        struct Chunk {
            Chunk(std::unique_ptr<Chunk> previous)
                : previous{std::move(previous)} {}

            std::size_t top{0};
            std::array<Literal, ChunkSize> content{};
            std::unique_ptr<Chunk> previous;
        };
    
     public:
        LiteralStackAllocator() {
            this->top_chunk = std::make_unique<Chunk>(nullptr);
        }

        LiteralStackAllocator(const LiteralStackAllocator &) = delete;
        LiteralStackAllocator(LiteralStackAllocator &&) = default;

        ~LiteralStackAllocator() = default;

        LiteralStackAllocator &operator=(const LiteralStackAllocator &) = delete;
        LiteralStackAllocator &operator=(LiteralStackAllocator &&) = default;

        auto operator()(std::size_t length) {
            if (length > ChunkSize) {
                return std::unique_ptr<Literal[], Clause::LiteralsDeleter>(new Literal[length], Clause::LiteralsDeleter{true});
            }

            if (ChunkSize - this->top_chunk->top < length) {
                this->top_chunk = std::make_unique<Chunk>(std::move(this->top_chunk));
            }

            auto &chunk = *this->top_chunk;
            Literal *ptr = &chunk.content[chunk.top];
            chunk.top += length;
            return std::unique_ptr<Literal[], Clause::LiteralsDeleter>(ptr, Clause::LiteralsDeleter{false});
        }

     private:
        std::unique_ptr<Chunk> top_chunk;
    };
}

#endif
