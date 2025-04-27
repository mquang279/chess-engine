#ifndef TRANSPOSITION_TABLE_HPP
#define TRANSPOSITION_TABLE_HPP

#include <unordered_map>
#include <cstdint>
#include <tuple>
#include "chess.hpp"

enum class TTFlag {
    EXACT_SCORE = 0,
    UPPER_BOUND = 1,
    LOWER_BOUND = 2
};

struct TranspositionEntry {
    int score;
    TTFlag flag;
    int depth;
};

struct TTStats {
    size_t size;       // Số entry hiện tại trong bảng
    size_t capacity;   // Dung lượng tối đa của bảng
    double usage;      // Tỷ lệ sử dụng (%)
    size_t hits;       // Số lần tra cứu thành công
    size_t misses;     // Số lần tra cứu thất bại
    double hit_rate;   // Tỷ lệ hit (%)
    size_t collisions; // Số lần va chạm khi lưu entry
};

class TranspositionTable {
public:
    TranspositionTable(size_t size_mb = 64);
    void clear();
    void store(uint64_t hash_key, int score, TTFlag flag, int depth);
    std::tuple<bool, int> lookup(uint64_t hash_key, int depth, int alpha, int beta);
    TTStats get_stats() const;

private:
    std::unordered_map<uint64_t, TranspositionEntry> table;
    size_t hits = 0;
    size_t misses = 0;
    size_t collisions = 0;
    size_t capacity; 
};

#endif // TRANSPOSITION_TABLE_HPP