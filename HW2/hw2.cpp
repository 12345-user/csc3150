#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>

#define DUNGEON_HEIGHT 17
#define DUNGEON_WIDTH 49
#define WALL_LENGTH 15
#define NUM_WALLS 6
#define NUM_GOLDS 6

// Game state
typedef enum {
    GAME_RUNNING,
    GAME_WIN,
    GAME_LOSE,
    GAME_QUIT
} GameState;

// Direction
typedef enum {
    DIR_LEFT = -1,
    DIR_RIGHT = 1
} Direction;

// Wall structure
typedef struct {
    int row;
    int col;
    Direction dir;
    pthread_t thread;
} Wall;

// Gold structure
typedef struct {
    int row;
    int col;
    Direction dir;
    int collected;
    pthread_t thread;
} Gold;

// Adventurer structure
typedef struct {
    int row;
    int col;
} Adventurer;

// Global variables
char dungeon[DUNGEON_HEIGHT][DUNGEON_WIDTH + 1];
Wall walls[NUM_WALLS];
Gold golds[NUM_GOLDS];
Adventurer adventurer;
GameState game_state = GAME_RUNNING;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int collected_gold_count = 0;

// Terminal settings
struct termios old_term, new_term;

// Function prototypes
void init_dungeon();
void init_objects();
void draw_screen();
void* wall_thread(void* arg);
void* gold_thread(void* arg);
void* input_thread(void* arg);
int check_collision();
void clear_screen();
void setup_terminal();
void restore_terminal();
int kbhit();
void move_adventurer(char direction);

// Initialize dungeon
void init_dungeon() {
    // Top border
    dungeon[0][0] = '+';
    for (int i = 1; i < DUNGEON_WIDTH - 1; i++) {
        dungeon[0][i] = '-';
    }
    dungeon[0][DUNGEON_WIDTH - 1] = '+';
    dungeon[0][DUNGEON_WIDTH] = '\0';
    
    // Middle rows
    for (int i = 1; i < DUNGEON_HEIGHT - 1; i++) {
        dungeon[i][0] = '|';
        for (int j = 1; j < DUNGEON_WIDTH - 1; j++) {
            dungeon[i][j] = ' ';
        }
        dungeon[i][DUNGEON_WIDTH - 1] = '|';
        dungeon[i][DUNGEON_WIDTH] = '\0';
    }
    
    // Bottom border
    dungeon[DUNGEON_HEIGHT - 1][0] = '+';
    for (int i = 1; i < DUNGEON_WIDTH - 1; i++) {
        dungeon[DUNGEON_HEIGHT - 1][i] = '-';
    }
    dungeon[DUNGEON_HEIGHT - 1][DUNGEON_WIDTH - 1] = '+';
    dungeon[DUNGEON_HEIGHT - 1][DUNGEON_WIDTH] = '\0';
}

// Initialize game objects
void init_objects() {
    srand(time(NULL));
    
    // Initialize adventurer
    adventurer.row = 8;
    adventurer.col = 24;
    
    // Initialize walls (rows: 2, 4, 6, 10, 12, 14)
    int wall_rows[NUM_WALLS] = {2, 4, 6, 10, 12, 14};
    Direction wall_dirs[NUM_WALLS] = {DIR_RIGHT, DIR_LEFT, DIR_RIGHT, DIR_LEFT, DIR_RIGHT, DIR_LEFT};
    
    for (int i = 0; i < NUM_WALLS; i++) {
        walls[i].row = wall_rows[i];
        walls[i].col = 1 + rand() % (DUNGEON_WIDTH - WALL_LENGTH - 2);
        walls[i].dir = wall_dirs[i];
    }
    
    // Initialize golds (rows: 1, 3, 5, 11, 13, 15)
    int gold_rows[NUM_GOLDS] = {1, 3, 5, 11, 13, 15};
    
    for (int i = 0; i < NUM_GOLDS; i++) {
        golds[i].row = gold_rows[i];
        golds[i].col = 1 + rand() % (DUNGEON_WIDTH - 2);
        golds[i].dir = (rand() % 2 == 0) ? DIR_LEFT : DIR_RIGHT;
        golds[i].collected = 0;
    }
}

// Clear screen using ANSI escape codes
void clear_screen() {
    printf("\033[2J\033[H");
    fflush(stdout);
}

// Draw the game screen
void draw_screen() {
    pthread_mutex_lock(&mutex);
    
    // Create temporary dungeon for drawing
    char temp_dungeon[DUNGEON_HEIGHT][DUNGEON_WIDTH + 1];
    memcpy(temp_dungeon, dungeon, sizeof(dungeon));
    
    // Draw walls
    for (int i = 0; i < NUM_WALLS; i++) {
        for (int j = 0; j < WALL_LENGTH; j++) {
            int col = walls[i].col + j;
            if (col >= 1 && col < DUNGEON_WIDTH - 1) {
                temp_dungeon[walls[i].row][col] = '=';
            }
        }
    }
    
    // Draw golds
    for (int i = 0; i < NUM_GOLDS; i++) {
        if (!golds[i].collected && golds[i].col >= 1 && golds[i].col < DUNGEON_WIDTH - 1) {
            temp_dungeon[golds[i].row][golds[i].col] = '$';
        }
    }
    
    // Draw adventurer
    if (adventurer.row >= 1 && adventurer.row < DUNGEON_HEIGHT - 1 &&
        adventurer.col >= 1 && adventurer.col < DUNGEON_WIDTH - 1) {
        temp_dungeon[adventurer.row][adventurer.col] = '0';
    }
    
    // Clear and draw
    clear_screen();
    for (int i = 0; i < DUNGEON_HEIGHT; i++) {
        printf("%s\n", temp_dungeon[i]);
    }
    
    pthread_mutex_unlock(&mutex);
}

// Check collision with walls
int check_collision() {
    for (int i = 0; i < NUM_WALLS; i++) {
        for (int j = 0; j < WALL_LENGTH; j++) {
            int wall_col = walls[i].col + j;
            if (walls[i].row == adventurer.row && wall_col == adventurer.col) {
                return 1; // Collision detected
            }
        }
    }
    return 0;
}

// Wall thread function
void* wall_thread(void* arg) {
    Wall* wall = (Wall*)arg;
    
    while (game_state == GAME_RUNNING) {
        usleep(80000); // 80ms delay for smooth movement (about 12.5 steps per second)
        
        pthread_mutex_lock(&mutex);
        
        wall->col += wall->dir;
        
        // Wrap around smoothly - allow wall to go beyond border and reappear from opposite side
        if (wall->dir == DIR_RIGHT && wall->col > DUNGEON_WIDTH - 1) {
            // Wall completely disappeared from right, reappear from left
            wall->col = 1 - WALL_LENGTH;
        } else if (wall->dir == DIR_LEFT && wall->col < 1 - WALL_LENGTH) {
            // Wall completely disappeared from left, reappear from right
            wall->col = DUNGEON_WIDTH - 1;
        }
        
        pthread_mutex_unlock(&mutex);
    }
    
    return NULL;
}

// Gold thread function
void* gold_thread(void* arg) {
    Gold* gold = (Gold*)arg;
    
    while (game_state == GAME_RUNNING) {
        usleep(100000); // 100ms delay
        
        pthread_mutex_lock(&mutex);
        
        if (!gold->collected) {
            gold->col += gold->dir;
            
            // Wrap around smoothly - allow gold to go beyond border and reappear from opposite side
            if (gold->col >= DUNGEON_WIDTH) {
                gold->col = 0;
            } else if (gold->col < 0) {
                gold->col = DUNGEON_WIDTH - 1;
            }
            
            // Check if adventurer collected the gold (only if gold is visible in dungeon)
            if (gold->col >= 1 && gold->col < DUNGEON_WIDTH - 1) {
                if (gold->row == adventurer.row && gold->col == adventurer.col) {
                    gold->collected = 1;
                    collected_gold_count++;
                    if (collected_gold_count == NUM_GOLDS) {
                        game_state = GAME_WIN;
                    }
                }
            }
        }
        
        pthread_mutex_unlock(&mutex);
    }
    
    return NULL;
}

// Setup terminal for non-blocking input
void setup_terminal() {
    tcgetattr(STDIN_FILENO, &old_term);
    new_term = old_term;
    new_term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

// Restore terminal settings
void restore_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
}

// Move adventurer
void move_adventurer(char direction) {
    pthread_mutex_lock(&mutex);
    
    int new_row = adventurer.row;
    int new_col = adventurer.col;
    
    switch (direction) {
        case 'W':
        case 'w':
            new_row--;
            break;
        case 'S':
        case 's':
            new_row++;
            break;
        case 'A':
        case 'a':
            new_col--;
            break;
        case 'D':
        case 'd':
            new_col++;
            break;
    }
    
    // Check boundaries
    if (new_row >= 1 && new_row < DUNGEON_HEIGHT - 1 &&
        new_col >= 1 && new_col < DUNGEON_WIDTH - 1) {
        adventurer.row = new_row;
        adventurer.col = new_col;
        
        // Check collision with walls
        if (check_collision()) {
            game_state = GAME_LOSE;
            pthread_mutex_unlock(&mutex);
            return;
        }
        
        // Check if collected gold
        for (int i = 0; i < NUM_GOLDS; i++) {
            if (!golds[i].collected && 
                golds[i].row == adventurer.row && 
                golds[i].col == adventurer.col) {
                golds[i].collected = 1;
                collected_gold_count++;
                if (collected_gold_count == NUM_GOLDS) {
                    game_state = GAME_WIN;
                }
            }
        }
    }
    
    pthread_mutex_unlock(&mutex);
}

// Input thread function
void* input_thread(void* arg) {
    char c;
    
    while (game_state == GAME_RUNNING) {
        if (read(STDIN_FILENO, &c, 1) > 0) {
            if (c == 'Q' || c == 'q') {
                pthread_mutex_lock(&mutex);
                game_state = GAME_QUIT;
                pthread_mutex_unlock(&mutex);
                break;
            } else if (c == 'W' || c == 'w' || c == 'S' || c == 's' ||
                       c == 'A' || c == 'a' || c == 'D' || c == 'd') {
                move_adventurer(c);
            }
        }
        usleep(10000); // 10ms delay
    }
    
    return NULL;
}

int main() {
    // Initialize
    init_dungeon();
    init_objects();
    setup_terminal();
    
    // Create threads for walls
    for (int i = 0; i < NUM_WALLS; i++) {
        pthread_create(&walls[i].thread, NULL, wall_thread, &walls[i]);
    }
    
    // Create threads for golds
    for (int i = 0; i < NUM_GOLDS; i++) {
        pthread_create(&golds[i].thread, NULL, gold_thread, &golds[i]);
    }
    
    // Create input thread
    pthread_t input_tid;
    pthread_create(&input_tid, NULL, input_thread, NULL);
    
    // Main game loop
    while (game_state == GAME_RUNNING) {
        draw_screen();
        
        // Check collision in main thread as well
        pthread_mutex_lock(&mutex);
        if (check_collision()) {
            game_state = GAME_LOSE;
        }
        pthread_mutex_unlock(&mutex);
        
        usleep(50000); // 50ms refresh rate
    }
    
    // Wait for threads to finish
    pthread_join(input_tid, NULL);
    for (int i = 0; i < NUM_WALLS; i++) {
        pthread_join(walls[i].thread, NULL);
    }
    for (int i = 0; i < NUM_GOLDS; i++) {
        pthread_join(golds[i].thread, NULL);
    }
    
    // Draw final screen
    draw_screen();
    
    // Display result
    printf("\n");
    if (game_state == GAME_WIN) {
        printf("You win the game!!\n");
    } else if (game_state == GAME_LOSE) {
        printf("You lose the game!!\n");
    } else if (game_state == GAME_QUIT) {
        printf("You exit the game.\n");
    }
    
    // Restore terminal
    restore_terminal();
    
    return 0;
}

