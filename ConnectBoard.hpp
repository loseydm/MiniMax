#ifndef CONNECTFOUR_CONNECTBOARD_HPP
#define CONNECTFOUR_CONNECTBOARD_HPP

#include <iostream>

using board = unsigned long long;

/*
     * Use 64 bits to store board
     * 8 17 … 63
     * 7 16 … 62
     * …
     * 0 9  … 55
     *
     * using bit operations for constant time board
     */
struct ConnectBoard {
    ConnectBoard() {
        pieces = 1ull << 63ull; // Turn counter bit, player xor with board gives 0 when max and 1 when min
        player = 0;
    }
    
    ConnectBoard(const board new_pieces, const board new_player) {
        pieces = new_pieces;
        player = new_player;
    }

    bool operator== (const ConnectBoard other) const noexcept {
        return other.player == player && other.pieces == pieces;
    }

    [[nodiscard]] inline bool is_player_one() const noexcept {
        return player & (1ull << 63ull);
    }

    [[nodiscard]] inline bool is_invalid_move(unsigned char col, unsigned char max_rows) const noexcept {
        return pieces & (1ull << (8ull * col + max_rows - 1));
    }

    [[nodiscard]] inline bool is_full(unsigned char cols, unsigned char max_rows) const noexcept {
        for (int i = 0; i < cols; ++i) {
            if (!is_invalid_move(i, max_rows))
                return false;
        }

        return true;
    }

    inline void make_move(int col) noexcept {
        player ^= pieces; // Swap the player board stored
        pieces |= pieces + (1ull << 8ull * col); // Make the next move in that column
    }
    
    [[nodiscard]] inline ConnectBoard make_neighbor(int col) const noexcept {
        return ConnectBoard{pieces | (pieces + (1ull << 8ull * col)), player ^ pieces, };
    }

    [[nodiscard]] inline bool game_over(const int chain) const noexcept {
        return (chain == 3 && connect_three_game_over()) || (chain == 4 && connect_four_game_over());
    }

    // Cannot be an operator because rows/cols are not saved for space reasons
    friend void print_board(std::ostream& out, const ConnectBoard& board, int rows, int cols) {
        char one{'X'}, two{'O'};

        if (board.is_player_one()) {
            std::swap(one, two);
        }

        for (int i = rows-1; i >= 0; --i) {
            out << "| ";

            for (int j = 0; j < cols; j++) {
                if (board.player & (1ull << (j * 8ull + i)))
                    out << one;
                else if (board.pieces & (1ull << (j * 8ull + i)))
                    out << two;
                else
                    out << ' ';

                out << " | ";
            }

            out << '\n';
        }
    }

    unsigned long player, pieces;

private:
    [[nodiscard]] inline bool connect_three_game_over() const noexcept {
        // Use carefull bit operations to fold board to detect wins from either player

        board collapsed, check = player ^ pieces; // We only need to check the previous player

        // horizontal win check
        collapsed = check & (check << 8ull);
        if (collapsed & (collapsed << 8ull))
            return true;

        // vertical win check
        collapsed = check & (check << 1ull);
        if (collapsed & (collapsed << 1ull))
            return true;

        // main diagonal win check
        collapsed = check & (check << 7ull);
        if (collapsed & (collapsed << 7ull))
            return true;

        // anti-diagonal win check
        collapsed = check & (check << 9ull);
        return collapsed & (collapsed << 9ull);
    }

    [[nodiscard]] inline bool connect_four_game_over() const noexcept {
        // Use carefull bit operations to fold board to detect wins from either player

        board collapsed, check = player ^ pieces; // We only need to check the previous player

        // horizontal win check
        collapsed = check & (check << 8ull);
        if (collapsed & (collapsed << 16ull))
            return true;

        // vertical win check
        collapsed = check & (check << 1ull);
        if (collapsed & (collapsed << 2ull))
            return true;

        // main diagonal win check
        collapsed = check & (check << 7ull);
        if (collapsed & (collapsed << 14ull))
            return true;

        // anti-diagonal win check
        collapsed = check & (check << 9ull);
        return collapsed & (collapsed << 18ull);
    }
};

template <>
struct std::hash<ConnectBoard> {
    std::size_t operator() (const ConnectBoard game) const {
        return (std::hash<board>{}(game.pieces) ^ (std::hash<board>{}(game.player) << 1ull) >> 1ull);
    }
};

void print_board(std::ostream& out, const ConnectBoard& board, int rows, int cols);

#endif
