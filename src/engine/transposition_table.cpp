#include "transposition_table.hpp"

TranspositionTable::TranspositionTable(size_t size_mb) {
    capacity = (size_mb * 1024 * 1024) / sizeof(TranspositionEntry);
    table.reserve(capacity);
}

void TranspositionTable::clear() {
    table.clear();
    hits = 0;
    misses = 0;
    collisions = 0;
    current_age = 0;
}

void TranspositionTable::store(uint64_t hash_key, int score, TTFlag flag, int depth) {
    TranspositionEntry entry = {score, flag, depth};
    if (table.find(hash_key) != table.end()) {
        auto& existing = table[hash_key];
        if (depth >= existing.depth || flag == TTFlag::EXACT_SCORE || current_age > existing.age + 2) {
            table[hash_key] = entry;
        } else {
            collisions++;
        }
    } else {
        table[hash_key] = entry;
    }
}

std::tuple<bool, int> TranspositionTable::lookup(uint64_t hash_key, int depth, int alpha, int beta) {
    auto it = table.find(hash_key);
    if (it != table.end()) {
        const auto& entry = it->second;
        if (entry.depth >= depth) {
            hits++;
            if (entry.flag == TTFlag::EXACT_SCORE) {
                return {true, entry.score};
            } else if (entry.flag == TTFlag::LOWER_BOUND && entry.score >= beta) {
                return {true, entry.score};
            } else if (entry.flag == TTFlag::UPPER_BOUND && entry.score <= alpha) {
                return {true, entry.score};
            }
        }
    }
    misses++;
    return {false, 0};
}

TTStats TranspositionTable::get_stats() const {
    size_t total_lookups = hits + misses;
    double hit_rate = (total_lookups > 0) ? (static_cast<double>(hits) / total_lookups * 100.0) : 0.0;
    double usage = (capacity > 0) ? (static_cast<double>(table.size()) / capacity * 100.0) : 0.0;

    return TTStats{
        table.size(),   // size
        capacity,       // capacity
        usage,          // usage
        hits,           // hits
        misses,         // misses
        hit_rate,       // hit_rate
        collisions      // collisions
    };
}

void TranspositionTable::increment_age() {
    current_age++;  
    if (current_age == 255) {
        clear();  // Reset table if age reaches max to avoid issues
    }
}