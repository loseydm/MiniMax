My implementation of MiniMax was designed with one goal: speed. I used two 64 bit integers to represent the state of the board. One of the integers
represents the current player's pieces, and the other is all the pieces on the board. By xor'ing the two integers you are left with the other player's pieces.

The integers as boards are laid out like this:
(Each number corresponds to that bit in the number)

7	15	23	31	39	47	55	63 <- This bit is use to represent whose turn it is.
6	14	22	30	38	46	54	62
5	13	21	29	37	45	53	61
4	12	20	28	36	44	52	60
3	11	19	27	35	43	51	59
2	10	18	26	34	42	50	58
1	9	17	25	33	41	49	57
0	8	16	24	32	40	48	56

This has two advantages:
    1.) Space, this takes up exactly 16 bytes per state, as opposed to a typical matrix implementation that would use
        about 49 bytes for a 7x7 board and 32 bytes for all the pointers for a total of 81 bytes.

    2.) Board operations are the primary benefit here. I used bit shifting, bitwise xor/and/or/negation, and addition to
        have constant time board operations (~8 arithmetic operations per function). As fixed precision arithmetic is
        the fastest operation a computer can perform, this is quite the speedup over using a matrix.

Some other simple optimizations:
    - My transposition table uses an unsigned char and int to store values, minimum number of bits needed.


    Optimized version of Part A:
    - Win states are tested for before calling MiniMax to reduce the stack space and allow early termination of the loop
      because there is no better move that can be made than to win immediately.
    - Considered storing the best starting move (which is always a middle column) to reduce the search space by a factor
      of columns but decided this went against the spirit of the program.

Failed optimization:
    - I tried parallelizing MiniMax by running the initial children on different cores of the computer; however,
      transposition tables could not be shared across threads without using locking mechanisms which throttled performance.
      Giving each thread its own transposition table was significantly better, but the redundant exploration the threads
      performed without having access to the other transposition tables proved to be less efficient than the sequential
      program.

Heuristic:
    - My heuristic looks for singleton pieces and chained pieces of length 2/3 with enough empty spaces to become wins.
      Singletons are worth 500, doubles 2,000, and triples 5,000. A win is worth 100,000 / depth from current board state.

