CSC3150 Assignment 2 - The Greatest Adventurer Game
===================================================

COMPILATION:
-----------
To compile the program, use the following command:

    g++ hw2.cpp -o hw2 -lpthread

or using the Makefile:

    make

Alternative compilation options:
    g++ -Wall -pthread hw2.cpp -o hw2
    g++ hw2.cpp -lpthread -o hw2


EXECUTION:
---------
To run the game, use:

    ./hw2

or using the Makefile:

    make run


GAME CONTROLS:
-------------
W - Move adventurer UP
S - Move adventurer DOWN
A - Move adventurer LEFT
D - Move adventurer RIGHT
Q - QUIT the game


GAME RULES:
----------
1. The adventurer (represented by '0') starts in the middle of the dungeon
2. Collect all 6 gold shards (represented by '$') to win
3. Avoid hitting the moving walls (represented by '===============')
4. Walls move left and right and wrap around the dungeon
5. If you hit a wall, you lose the game


ENVIRONMENT:
-----------
- Ubuntu 16.04 - 22.04
- Linux Kernel 5.4.x or 5.15.x
- GCC 4.9 or above
- Pthread library


IMPLEMENTATION DETAILS:
----------------------
- The program uses multiple threads (Pthread):
  * 6 threads for moving walls
  * 6 threads for moving gold shards
  * 1 thread for user input handling
  * Main thread for game loop and rendering

- Thread synchronization is achieved using pthread_mutex_t
- Non-blocking terminal input is implemented using termios
- Random positions and directions are generated using srand() and rand()


NOTES:
-----
- The walls traverse the dungeon in approximately 10 seconds
- The game screen is updated smoothly without flickering
- No redundant characters are displayed on the screen
- The adventurer cannot cross or be on the borders of the dungeon

