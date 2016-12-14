# GMKalah-AI
Kalah Agent I made for my AI class Spring 2016. Winner of the competition between class members.

Compilation: 
gcc main.c -o GMKalah

Execution:

.\GMKalah 1 -d -h 14

Where "1" is what player the AI will be. (Either 1 or 2)

The "-d" will print a board representation at each ply and move evaluation at each depth.

The "-h 14" is the starting depth to search at. The computer will search for 15secs, while doing interative deepening and return the best result afterwards. In the example above, the agent will start at depth 14 and search deeper until 15 secs are up.

After executing the program, commands must be passed to the agent to update the board.

"move" will start the alpha beta search and make a move for the agent.

"opponent X" will choose the bin 'X'. Bins are labeled as below.
player 1: 0   1   2   3   4   5

player 2: 12  11  10   9   8   7

With Bin 6 being player 1s score and Bin 13 being player 2s score.

Turn rules and bin bounds are enforced by the user and not the program itself. Furthermore, if it is the opponents or Agent's turn again, because the last bean has landed in the player's home, then it is up to the user to initiate their move again. 


