#ifndef CONNECTFOUR_MINIMAX_HPP
#define CONNECTFOUR_MINIMAX_HPP

#include <functional>
#include <unordered_map>

#include "timer.hpp"
#include "ConnectBoard.hpp"

/* FullMiniMax implements a full search of the game tree. Heuristic MiniMax uses all available tricks to search the game tree efficiently.
 * Side note: The two classes are not good candidates for inheritance due to significant virtual method overhead for the 100,000's to 1,000,000's of time those functions are called. */

struct FullMiniMax {
    FullMiniMax(int rows, int cols, int chain, bool optimized=true, bool verbose=true): rows(rows), cols(cols), chain(chain), verbose(verbose),
    optimized(optimized) {}

    auto operator() (ConnectBoard board) {
        if (table.find(board) == table.end()) {
            Stopwatch timer;

            if (optimized) {
                efficient_traverse(board, true);
            } else {
                traverse(board, true);
            }

            if (verbose) {
                std::cout << "MiniMax search completed in " << timer << ".\n";
                std::cout << table.size() << " states in the table." << std::endl;
            }
        }

        if (verbose) {
            auto [score, column] = table[board];

            std::cout << "This state has a value of " << score << ".\n";

            if (score > 0)
                std::cout << "First player will win with optimal play.";
            else if (score > 0)
                std::cout << "Second player will win with optimal play.";
            else
                std::cout << "The players will tie with optimal play.";

            std::cout << "\n\n";
        }

        return table[board];
    }

private:
    int efficient_traverse(const ConnectBoard board, bool max, int depth=0) {
        // MiniMax traversal with transposition table

        // Memoized states needn't be explored again
        if (table.find(board) != table.end())
            return table[board].first;

        // Filled the whole board without a win
        if (depth == rows * cols)
            return 0;

        // Keep generic for min/max in same loop
        int best_score;
        unsigned char best_move;
        std::function<bool(int, int)> compare;

        if (max) {
            best_score = std::numeric_limits<int>::min();
            compare = std::greater<>();
        } else {
            best_score = std::numeric_limits<int>::max();
            compare = std::less<>();
        }

        // Examine all neighboring boards and take min/max
        int current;
        for (int col = 0; col < cols; ++col) {
            if (board.is_invalid_move(col, rows))
                continue;

            auto next = board.make_neighbor(col);

            /* We always take the move we can win
             * Initially, I thought this could cause a problem with a min player not choosing
             * a win move if possible, but min player simply cannot win in our implementation */
            if (next.game_over(chain)) {
                best_move = col;
                best_score = max? score(depth + 1) : -score(depth + 1);
                break;
            }

            current = efficient_traverse(board.make_neighbor(col), not max, depth + 1);

            if (compare(current, best_score)) {
                best_score = current;
                best_move = col;
            }
        }

        table[board] = std::make_pair(best_score, best_move);
        return best_score;
    }

    int traverse(const ConnectBoard board, bool max, int depth=0) {
        // MiniMax traversal with transposition table

        // Memoized states needn't be explored again
        if (table.find(board) != table.end())
            return table[board].first;

        if (board.game_over(chain)) {
            int best_score = max? -score(depth + 1) : score(depth + 1);
            table[board] = std::make_pair(best_score, 0);
            return best_score;
        }

        // Filled the whole board without a win
        if (depth == rows * cols) {
            table[board] = std::make_pair(0, 0);
            return 0;
        }

        // Keep generic for min/max in same loop
        int best_score;
        unsigned char best_move;
        std::function<bool(int, int)> compare;

        if (max) {
            best_score = std::numeric_limits<int>::min();
            compare = std::greater<>();
        } else {
            best_score = std::numeric_limits<int>::max();
            compare = std::less<>();
        }

        // Examine all neighboring boards and take min/max
        int current;
        for (int col = 0; col < cols; ++col) {
            if (board.is_invalid_move(col, rows))
                continue;

            current = traverse(board.make_neighbor(col), not max, depth + 1);

            if (compare(current, best_score)) {
                best_score = current;
                best_move = col;
            }
        }

        table[board] = std::make_pair(best_score, best_move);
        return best_score;
    }

    [[nodiscard]] inline int score(int depth) const noexcept {
        return 10'000 * rows * cols / depth;
    }

    const int rows, cols, chain;
    const bool verbose, optimized;
    std::unordered_map<ConnectBoard, std::pair<int, unsigned char>> table;
};

struct HeuristicMiniMax {
    HeuristicMiniMax(int rows, int cols, int chain, int max_depth, bool verbose=true): cols(cols), rows(rows), chain(chain), max_depth(max_depth), verbose(verbose) {
        // Sets boundary spaces for use in heuristic to turn off spaces that cannot hold pieces
        boundary_spaces = std::numeric_limits<unsigned long long>::max();

        // Gets a columns worth of 1's
        unsigned long long column = static_cast<unsigned long long>(std::pow(2, rows)) - 1ull;
        for (int i = 0; i < cols; ++i) {
            boundary_spaces ^= (column << (8ull * i)); // Toggles off columns in boundary bits
        }
    }

    auto operator() (ConnectBoard board) {
        table.clear(); // Remove table entries because of depth

        Stopwatch timer;
        traverse(board, true);

        if (verbose) {
            std::cout << "MiniMax search completed in " << timer << ".\n";
            std::cout << table.size() << " states in transposition table." << std::endl;
            std::cout << "This state has a score of " << table[board].first << ".\n\n";
        }

        return table[board];
    }

private:
    int traverse(const ConnectBoard board, bool max, int depth = 1,
                 int alpha = std::numeric_limits<int>::min(),
                 int beta = std::numeric_limits<int>::max()) {
        // MiniMax traversal with αβ pruning, transposition tables, and a heuristic function

        // Memoized states needn't be explored again
        if (table.find(board) != table.end())
            return table[board].first;

        // Evaluate board by counting usable chained pieces of length 1/2/3
        if (depth == max_depth)
            return max? heuristic(board): -heuristic(board);

        // Keep generic for min/max in same loop
        unsigned char best_move;
        int best_score, *boundary, *update;
        std::function<bool(int, int)> compare;

        if (max) {
            update = &alpha;
            boundary = &beta;
            compare = std::greater<>();
            best_score = std::numeric_limits<int>::min();
        } else {
            update = &beta;
            boundary = &alpha;
            compare = std::less<>();
            best_score = std::numeric_limits<int>::max();
        }

        // Iterate through valid children
        /* If we've made it to this point, nobody has won. If there are no valid moves,
           we have a tied board. */
        int current;
        bool moved{false};
        for (int i = 0; i < cols; ++i) {
            if (board.is_invalid_move(i, rows)) {
                continue;
            }

            moved = true;
            auto child = board.make_neighbor(i);

            if (child.game_over(chain)) {
                best_move = i;
                best_score = max? win_score / depth : -win_score / depth;
                break;
            }

            current = traverse(child, not max, depth + 1, alpha, beta);

            if (compare(current, best_score)) {
                best_move = i;
                best_score = current;

                // Best score is outside our αβ bound -> quit
                if (compare(best_score, *boundary))
                    break;

                // If best > α -> α = best; best < β -> β = best
                if (compare(best_score, *update))
                    *update = best_score;
            }
        }

        // Tie
        if (!moved)
            return 0;

        table[board] = std::make_pair(best_score, best_move);
        return best_score;
    }

    [[nodiscard]] int heuristic(ConnectBoard board) const noexcept {
        // Opponent and Self; 1, 2, 3 chains; 7 directions (42 total checks)

        const int offsets[]{1ull, // Vertical
                            7ull, // Anti-diagonal
                            8ull, // Horizontal
                            9ull  // Diagonal
        };

        int player_total{}, opponent_total{};

        unsigned long long pieces = board.pieces;
        unsigned long long player = board.player, opponent = board.player ^ board.pieces;

        if (chain == 4) {
            for (auto offset : offsets) {
                player_total += count_left_four_chains(player, pieces, offset);
                player_total += count_right_four_chains(player, pieces, offset);

                opponent_total += count_left_four_chains(opponent, pieces, offset);
                opponent_total += count_right_four_chains(opponent, pieces, offset);
            }
        } else {
            for (auto offset : offsets) {
                player_total += count_left_three_chains(player, pieces, offset);
                player_total += count_right_three_chains(player, pieces, offset);

                opponent_total += count_left_three_chains(opponent, pieces, offset);
                opponent_total += count_right_three_chains(opponent, pieces, offset);
            }
        }

        return player_total - opponent_total;
    }

    [[nodiscard]] inline int count_right_three_chains(unsigned long long player,
                                                      unsigned long long pieces,
                                                      unsigned long long offset) const noexcept {
        auto empty_spaces = ~pieces ^ boundary_spaces;

        // Gets two empty spaces
        auto two_spaces = (empty_spaces & (empty_spaces >> offset)) >> (2 * offset);

        // Gets all singleton pieces that could form a chain of 3
        auto usable_pieces = count_bits(player & two_spaces);

        // Gets right chains of two pieces
        auto two_chains = player & (player >> offset);

        // Gets double empty spaces and shifts over by two slots
        auto empty = empty_spaces >> (2 * offset);

        // Counts chains of two that can be turned into a chain of 4
        int valid_twos = count_bits(two_chains & empty);

        return two_chain_value * valid_twos + singleton_value * usable_pieces;
    }

    [[nodiscard]] inline int count_left_three_chains(unsigned long long player,
                                                     unsigned long long pieces,
                                                     unsigned long long offset) const noexcept {
        auto empty_spaces = ~pieces ^ boundary_spaces;

        // Gets two empty spaces
        auto two_spaces = (empty_spaces & (empty_spaces << offset)) << (2 * offset);

        // Gets all singleton pieces that could form a chain of 3
        auto usable_pieces = count_bits(player & two_spaces);

        // Gets right chains of two pieces
        auto two_chains = player & (player << offset);

        // Gets double empty spaces and shifts over by two slots
        auto empty = empty_spaces << (2 * offset);

        // Counts chains of two that can be turned into a chain of 4
        int valid_twos = count_bits(two_chains & empty);

        return two_chain_value * valid_twos + singleton_value * usable_pieces;
    }

    [[nodiscard]] inline int count_right_four_chains(unsigned long long player,
                                                     unsigned long long pieces,
                                                     unsigned long long offset) const noexcept {

        auto empty_spaces = ~pieces ^ boundary_spaces;

        // Gets three empty spaces
        auto two_spaces = (empty_spaces & (empty_spaces >> offset));
        auto three_spaces = (two_spaces & (two_spaces >> offset)) >> (3 * offset);

        // Gets all singleton pieces that could form a chain of 4
        auto usable_pieces = count_bits(player & three_spaces);

        // Gets right chains of two pieces
        auto two_chains = player & (player >> offset);

        // Gets double empty spaces and shifts over by two slots
        auto two_empty = (empty_spaces & (empty_spaces >> offset)) >> (2 * offset);

        // Counts chains of two that can be turned into a chain of 4
        int valid_twos = count_bits(two_chains & two_empty);

        // Gets right chains of three pieces
        auto three_chains = two_chains & (two_chains >> offset);

        // Gets single empty space slotted over by 3
        auto single_space = empty_spaces >> (3 * offset);

        int valid_threes = count_bits(three_chains & single_space);

        return three_chain_value * valid_threes + two_chain_value * valid_twos + singleton_value * usable_pieces;
    }

    [[nodiscard]] inline int count_left_four_chains(unsigned long long player,
                                                    unsigned long long pieces,
                                                    unsigned long long offset) const noexcept {

        auto empty_spaces = ~pieces ^ boundary_spaces;

        // Gets right chains of two pieces
        auto two_chains = player & (player << offset);

        // Gets double empty spaces and shifts over by two slots
        auto two_empty = (empty_spaces & (empty_spaces << offset)) << (2 * offset);

        // Counts chains of two that can be turned into a chain of 4
        auto valid_twos = count_bits(two_chains & two_empty);

        // Gets right chains of three pieces
        auto three_chains = two_chains & (two_chains << offset);

        // Gets single empty space slotted over by 3
        auto single_space = empty_spaces << (3 * offset);

        auto valid_threes = count_bits(three_chains & single_space);

        return three_chain_value * valid_threes + two_chain_value * valid_twos;
    }

    /* Bit counting algorithm that iterates the same number of times as bits,
    credit to "The C Programming Language" by Brian Kernighan and Dennis Ritchie */
    static inline unsigned char count_bits(board n) noexcept {
        unsigned char c{};
        for (; n; ++c) {
            n &= n - 1;
        }

        return c;
    }

    const bool verbose;
    unsigned long long boundary_spaces;
    const int max_depth, chain, cols, rows;
    std::unordered_map<ConnectBoard, std::pair<int, unsigned char>> table;
    const int win_score=100'000, singleton_value=500, two_chain_value=2'000, three_chain_value=5'000;
};

#endif
