#include <iostream>
#include <functional>

#include "MiniMax.hpp"
#include "ConnectBoard.hpp"

template <typename MiniMax>
void play_game(MiniMax &game, const int rows, const int cols, const int chain) {
    ConnectBoard board;

    std::cout << "\nPlaying Connect-" << chain << " with a " << rows << 'x' << cols << " board.\n\n";

    auto game_over = [&](ConnectBoard b) {
        return b.game_over(chain) || b.is_full(cols, rows);
    };

    int move;
    while (!game_over(board)) {
        auto [score, column] = game(board); // Computes MiniMax on the specified game

        board.make_move(column);

        if (game_over(board))
            break;

        print_board(std::cout, board, rows, cols);

        move = -1;
        while (board.is_invalid_move(move, rows) || move < 0 || move >= cols) {
            std::cout << "Enter your column from 0 to " << cols - 1 << std::endl;
            std::cin >> move;
        }

        board.make_move(move);

        std::cout << '\n';
    }

    print_board(std::cout, board, rows, cols);

    if (board.game_over(chain)) {
        if (board.is_player_one())
            std::cout << "The computer won!" << std::endl;
        else
            std::cout << "You won!" << std::endl;
    } else if (board.is_full(cols, rows)) {
        std::cout << "Players tied!" << std::endl;
    }
}

int main() {
    char choice{};
    int rows{-1}, cols{-1}, chain{-1};

    std::cout << "Part A uses MiniMax with a transposition table to brute force the solutions to Connect Three of Four with "
                 "board sizes ranging from 3 to 7 in either dimension.\n";

    std::cout << "Part B uses MiniMax with αβ pruning, transposition tables, and a heuristic function to estimate "
                 "solutions to Connect Three or Four.\n\n";

    while (choice != 'a' && choice != 'b') {
        std::cout << "Which part would you like to play? Enter A or B: ";
        std::cin >> choice;

        choice = static_cast<char>(std::tolower(choice));
    }

    while (rows < 1 || rows > 7) {
        std::cout << "Rows must be in [2, 7]. Enter rows: ";
        std::cin >> rows;
    }

    while (cols < 1 || cols > 7) {
        std::cout << "Columns must be in [2, 7]. Enter columns: ";
        std::cin >> cols;
    }

    while (chain != 3 && chain != 4) {
        std::cout << "You can play connect 3 or connect 4. Enter n-in-a-row: ";
        std::cin >> chain;
    }

    if (choice == 'a') {
        std::cout << "\nI created an optimized version of part A, but it will not have the same number of transposition table entries because it does not cache leaf nodes and reduces recursion stack usage." << '\n' <<
                     "It also uses the observation that we do not need to check the neighbors of a state once we find a winning move from that parent in exactly one move." << '\n' <<
                     "I left in the unoptimized version for ease of grading.\n" << std::endl;

        std::string optimized{"x"};
        while (optimized != "no" && optimized != "yes") {
            std::cout << "Would you like the optimized version of the program? (yes or no) ";
            std::cin >> optimized;
            std::transform(optimized.begin(), optimized.end(), optimized.begin(), ::tolower);
        }



        FullMiniMax game{rows, cols, chain, optimized == "yes"};
        play_game(game, rows, cols, chain);
    } else {
        int depth{};
        while (depth < 1) {
            std::cout << "Maximum depth must be a positive integer. Enter maximum depth: ";
            std::cin >> depth;
        }

        HeuristicMiniMax game{rows, cols, chain, depth};
        play_game(game, rows, cols, chain);
    }
}
