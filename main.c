/**
 * Kalah Agent, by John V. Flickinger
 * 4/12/2016
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <time.h>

typedef struct {
    int v;
    int move;
} Node;

void input_loop();
int calculate_agent_turn();
int eval(int state[]);
int get_free_turns_for_playerX(int state [], int pNum);
int count_marbles(int state[], int pNum);
Node alpha_beta_search(int state[], int depth, int alpha, int beta, bool maximizingPlayer);
void get_all_avail_actions(int state[], int ret[], int pNum);
int move(int state[], int bowl, int pNum);
void special_condition_in_empty_bowl(int state[], int lastIndex);
int no_more_moves(int state[], int pNum);
int terminal_test(int children[]);
int max(int val1, int val2);
int min(int val1, int val2);
void init_board();
void print_out_board(int state[]);
int getLine(char* prmpt, char* buff, size_t sz);

/* Variables */
int board[14]; // game board
int playerNum; // either 1 or 2
int debug = 0;
int SEARCH_DEPTH = 10; // start depth
int s_depth = 0;
int PLAYER1_KALAH = 6;
int PLAYER2_KALAH = 13;
bool search_broken;
clock_t start, diff;

int main(int argc, char** argv)
{
    if(argc > 1) {
        int i;
        
        char* arg1 = argv[1];

        if(strcmp(arg1, "1") == 0) { // player 1
            playerNum = 1;
        } else if(strcmp(arg1, "2") == 0) { // player 2
            playerNum = 2;
        } else {
            printf("Not a valid command\n");
            exit(0);
        }
        if(argc > 2) {
            for(i = 0; i < argc; i++) {
                char * arg = argv[i];
                if(strcmp(arg, "-d") == 0) { // debug option
                    debug = 1;
                    printf("In debugging mode\n");
                } else if(strcmp(arg, "-h") == 0) {
                    i++;
                    SEARCH_DEPTH = atoi(argv[i]);
                    if(debug) {
                        printf("\nDepth is %d\n", SEARCH_DEPTH);
                    }
                }
            }            
        }
    } else {
        printf("Please specify agent number\n");
        exit(0);
    }

    init_board();
    if(debug) print_out_board(board);
    input_loop(); // start taking in commands

    return 0;
}

/**
 * @brief The main loop to recieve game commands
 */
void input_loop()
{
    int readResult;
    char buff[15]; // buffer to read user data

    while(1) { // TODO while not end of the game
        // get one line of input from the user
        readResult = getLine("", buff, sizeof(buff));
        if(readResult == 1) {
            printf("\nNo input\n");
            exit(0);
        } else if(readResult == 2) {
            printf("Input too long [%s]\n", buff);
            exit(0);
        } // end user-input

        // handle quit command
        if(strcmp(buff, "quit") == 0) {
            exit(0);
        }

        // handle move command
        else if(strcmp(buff, "move") == 0) {
            int suggestedMove = calculate_agent_turn();
            move(board, suggestedMove, (playerNum == 1) ? 1 : 2);
            if(debug) {
                printf("Move made: %d", suggestedMove);
                print_out_board(board);
            } else {
                printf("%d", suggestedMove);
            }
            fflush(stdout);
        }

        // handle opponent command
        else if(strstr(buff, "opponent") != '\0') {
            char* b;
            b = strtok(buff, " ");
            b = strtok(NULL, " "); // get the second argument
            int bowl = atoi(b);
            move(board, bowl, (playerNum == 1) ? 2 : 1);
            if(debug)
                print_out_board(board);
        }

        // printf("Eval of state: %d", eval(board));

        int i;
        for(i = 0; i < 15; i++) {
            buff[i] = 0;
        }
    }
}

int calculate_agent_turn()
{
    int fake_state[14];
    // make a copy of the board
    int i;
    for(i = 0; i < 14; i++) {
        fake_state[i] = board[i];
    }
    
    int best_move;
    start = clock();
    search_broken = false;
    s_depth = 0;
    do {
        if(debug) printf("At depth: %d:\n ----------------------------\n", SEARCH_DEPTH + s_depth);
        int move  = alpha_beta_search(fake_state, SEARCH_DEPTH + s_depth, INT_MIN, INT_MAX, true).move;
        diff = clock()-start;
        if(!search_broken) {
            best_move = move;
        }
        if(debug) printf("\ntime taken: %d secs\n---------------------\n\n", (diff/CLOCKS_PER_SEC));
        s_depth++;
    }while(!search_broken);
    
    return best_move;
    
}

int eval(int state[])
{
    int score = 0;
    int p1Stones = count_marbles(state, 1);
    int p2Stones = count_marbles(state, 2);
    
    score = ((state[6] - state[13]) + (p1Stones - p2Stones));
    
    //check if it's for sure win.
    if(state[6] > 36) {
        score+=1000;
    } else if(state[13] > 36) {
        score-=1000;
    }
    
    int i, countEmpty=0, mostOpponent = 0, aiMin = INT_MAX, aiMost = INT_MIN;
    for(i = 0; i < 6; i++) {
        int oppositeIndex = (12 - 2 * i) + i;
        if(state[i] < aiMin) {
            aiMin = state[i];
        }
        if(state[i] > aiMost) {
            aiMost = state[i];
        }
        if(state[i] == 0) {
            if(state[oppositeIndex] > 0) {
                
                //do small cost checks
                int canSteal = 0;
                int j;
                for(j = 0; j < 6; j++) {
                    if((state[j] - (i-j)) % 13 == 0) {
                        canSteal = 1;
                    }
                }
                if(canSteal) {
                    score += state[oppositeIndex];
                }
            }
            countEmpty++;
        }
        if(state[oppositeIndex] > mostOpponent)
            mostOpponent = state[oppositeIndex];
        if(state[oppositeIndex] == 0) {
            int j, canOpponentSteal = 0;
            if(state[oppositeIndex] > 5) {
                for(j = 7; j < 13; j++) {
                    if((state[j] - (i-j)) % 13 == 0) {
                        canOpponentSteal = 1;
                    }
                }
                if(canOpponentSteal) {
                    score -= state[i];
                }
            }
        }
    }
    
    if((aiMost - aiMin) > 10) {
        score--;
    }

//    if(countEmpty== 3 && (p1Stones + p2Stones) < 20) {
//        score--;
//    } else if(countEmpty == 4 && (p1Stones + p2Stones) < 20) {
//        score-=3;
//    }
//    else if(countEmpty == 3 && (p1Stones + p2Stones) < 10) {
//        score-=2;
//    } else if(countEmpty == 4 && (p1Stones + p2Stones) < 10) {
//        score-=5;
//    }
//    
//    if(mostOpponent > 10 && (p1Stones + p2Stones) < 30) {
//        score--;
//    }
//    else if(mostOpponent > 14 && (p1Stones + p2Stones) < 40) {
//        score-=4;
//    } else if(mostOpponent > 17) {
//        score-=10;
//    }
//    
//    if((p1Stones + p2Stones) > 60 && (state[5] < 4)) {
//        score += 1;
//    }
    
    
    float ratio = (float)p1Stones/p2Stones;
    if(ratio > .72) {
        score-=mostOpponent*2;
    }
    
    float inverseRatio = (float)p2Stones/p1Stones;
    
    if(inverseRatio > .85) {
        score-=mostOpponent/2;
    }
    
    score-=get_free_turns_for_playerX(state, 2);
    score+=get_free_turns_for_playerX(state, 1);
    
    if(playerNum == 2) score *= -1;

    return score;
}

int get_free_turns_for_playerX(int state [], int pNum) {
    int count = 0;
    if(pNum == 1) {
        int i;
        for(i = 0; i < 6; i++) {
            if(state[i] == (6-i)) {
                count++;
            }
        }
    } else if(pNum == 2) {
        int i;
        for(i = 7; i < 13; i++) {
            if(state[i] == (13-i)) {
                count++;
            }
        }
    }
    return count;
}

int count_marbles(int state[], int pNum)
{
    int sum = 0;
    if(pNum == 1) {
        int i;
        for(i = 0; i < 6; i++) {
            sum += state[i];
        }
    } else {
        int i;
        for(i = 7; i < 13; i++) {
            sum += state[i];
        }
    }
}

Node alpha_beta_search(int state[], int depth, int alpha, int beta, bool maximizingPlayer)
{
    Node node;
    if(depth == 0) {
        node.v = eval(state);
        return node;
    } 
    int children[14];
    // make copy of state
    int n_state[14];
    int i;
    int k;

    if(maximizingPlayer) {
        get_all_avail_actions(state, children, playerNum);

        if(terminal_test(children)) {
            node.v = eval(state);
            return node;
        }
        // print_out_board(children);
        node.v = INT_MIN;
        for(k = 13; k >= 0; k--) { // for every action in action
            if(children[k] == 1) {
                diff = clock()-start;
                if(((diff) / CLOCKS_PER_SEC) > 14 || search_broken) {
                    if(debug) printf("Broke off search.\n");
                    search_broken = true;
                    break;
                }
                // make a copy of the game state
                for(i = 0; i < 14; i++) {
                    n_state[i] = state[i];
                }
                
                int goAgain = move(n_state, k, playerNum);
                int val = alpha_beta_search(n_state, depth - 1, alpha, beta, (goAgain == 1) ? true : false).v;
                if(debug) {
                    if(depth == (SEARCH_DEPTH + s_depth)) {
                        printf("move %d with eval: %d\n", k, val);
                    }
                }
                if(node.v < val) {
                    node.v = val;
                    node.move = k;
                }
                alpha = max(alpha, node.v);
                            
                if(beta <= alpha) {
                    break;
                }
            }
        }

        return node;
    } else {
        if(playerNum == 1) {
            get_all_avail_actions(state, children, 2);
        } else {
            get_all_avail_actions(state, children, 1);
        }

        if(terminal_test(children)) {
            node.v = eval(state);
            return node;
        }

        node.v = INT_MAX;
        for(k = 13; k >=0; k--) { // for every action in action
            if(children[k] == 1) {
                diff = clock()-start;
                if(((diff) / CLOCKS_PER_SEC) > 14 || search_broken) {
                    if(debug) printf("Broke off search.\n");
                    search_broken = true;
                    break;
                }
                
                // make a copy of the game state
                for(i = 0; i < 14; i++) {
                    n_state[i] = state[i];
                }
                int goAgain = move(n_state, k, (playerNum == 1) ? 2 : 1);
                int val = alpha_beta_search(n_state, depth - 1, alpha, beta, (goAgain == 1) ? false : true).v;
                if(node.v >= val) {
                    node.v = val;
                    node.move = k;
                }
                beta = min(beta, node.v);
                if(beta <= alpha) {
                    break;
                }
                
                
                diff = clock()-start;
            }
        }

        return node;
    }
}

/**
 * @brief
 * @param state, the board state to look at.
 * @param ret, an array of all the actions containing either a 1 or a 0.
 *        1 for available action, 0 for unavailable.
 */
void get_all_avail_actions(int state[], int ret[], int pNum)
{

    if(pNum == 1) {
        int i;
        for(i = 0; i < 6; i++) {
            if(state[i] > 0) {
                ret[i] = 1;
            } else {
                ret[i] = 0;
            }
        }

        for(i = 7; i < 13; i++) {
            ret[i] = 0;
        }
    } else {
        int i;
        for(i = 7; i < 13; i++) {
            if(state[i] > 0) {
                ret[i] = 1;
            } else {
                ret[i] = 0;
            }
        }
        for(i = 0; i < 6; i++) {
            ret[i] = 0;
        }
    }
    ret[6] = 0;
    ret[13] = 0;
}

/**
 * @brief Move a stone.
 * @param state, the state of the board
 * @param bowl, which bowl number to move
 * @param pNum, who's the player moving?
 * @return 1 = Go again, 0 = next player's turn
 */
int move(int state[], int bowl, int pNum)
{
    // pick up stones from bowl
    int numStones = state[bowl];
    state[bowl] = 0;
    int index = bowl + 1;

    do {
        // Don't add stones to opponent's Kalah
        if(pNum == 1) {
            if(index == 13) {
                index = 0;
            }
        }
        if(pNum == 2) {
            if(index == 6) {
                index++;
            }
        }

        state[index]++;
        numStones--;

        index++;
        if(index > 13) {
            index = 0; // go in circle
        }
    } while(numStones > 0);

    // Reset index to last bowl where a stone was dropped
    if(index == 0) {
        index = 13;
    } else {
        index--;
    }

    // The bowl was previously empty
    if(state[index] == 1) {
        if(pNum == 1 && index < 6) {
            special_condition_in_empty_bowl(state, index); // empty bowl last stone in player's own side
        }

        if(pNum == 2 && index > 6 && index != 13) {
            special_condition_in_empty_bowl(state, index);
        }
    }

   //check to see if that player is out of stones
   if(no_more_moves(state, pNum)) {
        if(pNum == 1) {
            int i;
            for(i = 7; i < 13; i++) {
                int nStones = state[i];
                state[i] = 0;
                state[PLAYER2_KALAH] += nStones; 
            }
        } else {
            int i;
            for(i = 0; i < 6; i++) {
                int nStones = state[i];
                state[i] = 0;
                state[PLAYER1_KALAH] += nStones; 
            }
        }
        return 0;
    }

    // Special condition last stone in own's kalah
    if(pNum == 1 && index == 6) {
        return 1;
    }
    if(pNum == 2 && index == 13) {
        return 1;
    }

    return 0; // next player's turn
}

int no_more_moves(int state[], int pNum) {
    if(pNum == 1) {
        int i;
        for(i = 0; i < 6; i++) {
            if(state[i] > 0) {
                return false;
            }
        }
    } else {
        int i;
        for(i = 7; i < 13; i++) {
            if(state[i] > 0) {
                return false;
            }
        }
    }
    return true;
}

int terminal_test(int children[])
{
    int i;
    for(i = 0; i < 6; i++) {
        if(children[i] == 1) {
            return false;
        }
    }

    for(i = 7; i < 13; i++) {
        if(children[i] == 1) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Handle the special condition when player drops last stone into one of his/her bowls.
 * @param state, the board to modify
 * @param lastIndex, the place where the last stone went into
 */
void special_condition_in_empty_bowl(int state[], int lastIndex)
{

    if(lastIndex < 6) { // move to P1's Kalah
        int oppositeIndex = (12 - 2 * lastIndex) + lastIndex;
        int totStones = state[oppositeIndex] + 1;
        state[6] += totStones; // deposit to Kalah
        state[oppositeIndex] = 0;
        state[lastIndex] = 0;
    } else if(lastIndex > 6) { // move to P2's kalah
        int oppositeIndex = 12 - lastIndex;
        int totStones = state[oppositeIndex] + 1;
        state[13] += totStones; // deposit to Kalah
        state[oppositeIndex] = 0;
        state[lastIndex] = 0;
    }
}

int max(int val1, int val2)
{
    if(val1 > val2) {
        return val1;
    } else {
        return val2;
    }
}

int min(int val1, int val2)
{
    if(val1 < val2) {
        return val1;
    } else {
        return val2;
    }
}

/**
 * @brief Used to initialize the game state
 */
void init_board()
{
    int i;
    for(i = 0; i < 14; i++) {
        if(i == 13 || i == 6) {
            continue;
        }
        board[i] = 6; // beans per bowl
    }
}

/**
 * @brief Print out the representation of a game board.
 * @param state, the state of the game board.
 */
void print_out_board(int state[])
{
    // Gap size
    char* gap = "\t";

    // top row
    printf("\n%s%d%s%d%s%d", gap, state[12], gap, state[11], gap, state[10]);
    printf("%s%d%s%d%s%d\n", gap, state[9], gap, state[8], gap, state[7]);
    // middle
    printf("%d%s%s%s%s%s%s%s%d", state[13], gap, gap, gap, gap, gap, gap, gap, state[6]);
    // bottom row
    printf("\n%s%d%s%d%s%d", gap, state[0], gap, state[1], gap, state[2]);
    printf("%s%d%s%d%s%d\n", gap, state[3], gap, state[4], gap, state[5]);
}

/**
 * @brief Get a line of input from the user
 * @param prmpt, what to ask the user
 * @param buff, the buffer for temporary reading
 * @param sz, the size of the buffer
 * @return OK = 0; NO input = 1, too long = 2
 */
int getLine(char* prmpt, char* buff, size_t sz)
{
    int ch, extra;

    // Get line with buffer overrun protection.
    if(prmpt != NULL) {
        printf("%s", prmpt);
        fflush(stdout);
    }
    if(fgets(buff, sz, stdin) == NULL)
        return 1;

    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if(buff[strlen(buff) - 1] != '\n') {
        extra = 0;
        while(((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? 2 : 0;
    }

    // Otherwise remove newline and give string back to caller.
    buff[strlen(buff) - 1] = '\0';
    return 0;
}

