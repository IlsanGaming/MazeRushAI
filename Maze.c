#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#define MAP_WIDTH 51
#define MAP_HEIGHT 31

typedef enum _Direction {
    DIRECTION_LEFT,
    DIRECTION_UP,
    DIRECTION_RIGHT,
    DIRECTION_DOWN
} Direction;

typedef enum _MapFlag {
    MAP_FLAG_WALL,
    MAP_FLAG_EMPTY,
    MAP_FLAG_VISITED,
} MapFlag;

COORD getRandomStartingPoint() {
    COORD point;
    point.X = 1 + (rand() % (MAP_WIDTH - 2));
    point.Y = 1 + (rand() % (MAP_HEIGHT - 2));
    if (point.X % 2 == 0) point.X++;
    if (point.Y % 2 == 0) point.Y++;
    return point;
}

typedef struct {
    int x, y;
    int dist;
} Node;

Node queue[MAP_WIDTH * MAP_HEIGHT];
int front = 0, rear = 0;

const int DIR[4][2] = { {0, -2}, {0, 2}, {-2, 0}, {2, 0} };

int dx[4] = { -1, 1, 0, 0 };
int dy[4] = { 0, 0, -1, 1 };

typedef struct {
    int x, y;
} Position;

void enqueue(Node n) {
    if (rear >= MAP_WIDTH * MAP_HEIGHT) {
        printf("Queue is full! Cannot enqueue.\n");
        return;
    }
    queue[rear++] = n;
}

Node dequeue() {
    if (isQueueEmpty()) {
        return (Node) { -1, -1, -1 };
    }
    return queue[front++];
}

int isQueueEmpty() {
    return front == rear;
}

Position player, ai, exitPoint, missile;

void generateMap(int y, int x, int map[MAP_HEIGHT][MAP_WIDTH]);
void shuffleArray(int array[], int size);
int inRange(int y, int x);
COORD getRandomStartingPoint();
void printMaze(int map[MAP_HEIGHT][MAP_WIDTH]);
void movePlayer(char direction, int map[MAP_HEIGHT][MAP_WIDTH]);
void aiMove(int map[MAP_HEIGHT][MAP_WIDTH]);
int distFW[MAP_HEIGHT][MAP_WIDTH][MAP_HEIGHT][MAP_WIDTH];
int missileActive = 0;

int dist[MAP_HEIGHT][MAP_WIDTH];

void printMaze(int map[MAP_HEIGHT][MAP_WIDTH]) {
    system("cls");
    for (int i = 0; i < MAP_HEIGHT; ++i) {
        for (int j = 0; j < MAP_WIDTH; ++j) {
            if (i == player.y && j == player.x) {
                printf("P ");
            }
            else if (i == ai.y && j == ai.x) {
                printf("A ");
            }
            else if (missileActive && i == missile.y && j == missile.x) {
                printf("M ");
            }
            else if (i == exitPoint.y && j == exitPoint.x) {
                printf("E ");
            }
            else {
                printf("%s", map[i][j] == MAP_FLAG_WALL ? "# " : "  ");
            }
        }
        printf("\n");
    }
}

void shuffleArray(int array[], int size) {
    for (int i = 0; i < size - 1; ++i) {
        int r = i + rand() % (size - i);
        int temp = array[i];
        array[i] = array[r];
        array[r] = temp;
    }
}

int inRange(int y, int x) {
    return (y < MAP_HEIGHT - 1 && y > 0) && (x < MAP_WIDTH - 1 && x > 0);
}

void generateMap(int y, int x, int map[MAP_HEIGHT][MAP_WIDTH]) {
    int directions[4] = { DIRECTION_UP, DIRECTION_RIGHT, DIRECTION_DOWN, DIRECTION_LEFT };
    shuffleArray(directions, 4);
    map[y][x] = MAP_FLAG_VISITED;
    for (int i = 0; i < 4; i++) {
        int nx = x + DIR[directions[i]][0];
        int ny = y + DIR[directions[i]][1];
        if (inRange(ny, nx) && map[ny][nx] == MAP_FLAG_WALL) {
            generateMap(ny, nx, map);
            if (ny != y)
                map[(ny + y) / 2][x] = MAP_FLAG_EMPTY;
            else
                map[y][(x + nx) / 2] = MAP_FLAG_EMPTY;
            map[ny][nx] = MAP_FLAG_EMPTY;
        }
    }
}

void movePlayer(char direction, int map[MAP_HEIGHT][MAP_WIDTH]) {
    int newX = player.x;
    int newY = player.y;
    if (direction == 'w') newY--;
    else if (direction == 's') newY++;
    else if (direction == 'a') newX--;
    else if (direction == 'd') newX++;
    if (inRange(newY, newX) && map[newY][newX] != MAP_FLAG_WALL) {
        player.x = newX;
        player.y = newY;
    }
}

void dijkstraMove(int map[MAP_HEIGHT][MAP_WIDTH]) {
    front = 0;
    rear = 0;
    int dist[MAP_HEIGHT][MAP_WIDTH];
    Position prev[MAP_HEIGHT][MAP_WIDTH];
    int visited[MAP_HEIGHT][MAP_WIDTH] = { 0 };
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            dist[i][j] = INT_MAX;
        }
    }
    dist[ai.y][ai.x] = 0;
    enqueue((Node) { ai.x, ai.y, 0 });
    visited[ai.y][ai.x] = 1;
    while (!isQueueEmpty()) {
        Node current = dequeue();
        for (int i = 0; i < 4; i++) {
            int newX = current.x + dx[i];
            int newY = current.y + dy[i];
            if (inRange(newY, newX) && map[newY][newX] != MAP_FLAG_WALL && !visited[newY][newX]) {
                visited[newY][newX] = 1;
                int newDist = dist[current.y][current.x] + 1;
                if (newDist < dist[newY][newX]) {
                    dist[newY][newX] = newDist;
                    prev[newY][newX] = (Position){ current.x, current.y };
                    enqueue((Node) { newX, newY, newDist });
                }
            }
        }
    }
    Position nextMove = exitPoint;
    while (prev[nextMove.y][nextMove.x].x != ai.x || prev[nextMove.y][nextMove.x].y != ai.y) {
        nextMove = prev[nextMove.y][nextMove.x];
    }
    ai.x = nextMove.x;
    ai.y = nextMove.y;
}

void floydWarshall(int map[MAP_HEIGHT][MAP_WIDTH]) {
    for (int y1 = 0; y1 < MAP_HEIGHT; y1++) {
        for (int x1 = 0; x1 < MAP_WIDTH; x1++) {
            for (int y2 = 0; y2 < MAP_HEIGHT; y2++) {
                for (int x2 = 0; x2 < MAP_WIDTH; x2++) {
                    if (y1 == y2 && x1 == x2) {
                        distFW[y1][x1][y2][x2] = 0;
                    }
                    else if (abs(y1 - y2) + abs(x1 - x2) == 1 && map[y2][x2] != MAP_FLAG_WALL) {
                        distFW[y1][x1][y2][x2] = 1;
                    }
                    else {
                        distFW[y1][x1][y2][x2] = INT_MAX;
                    }
                }
            }
        }
    }
    for (int kY = 0; kY < MAP_HEIGHT; kY++) {
        for (int kX = 0; kX < MAP_WIDTH; kX++) {
            if (map[kY][kX] == MAP_FLAG_WALL) continue;
            for (int iY = 0; iY < MAP_HEIGHT; iY++) {
                for (int iX = 0; iX < MAP_WIDTH; iX++) {
                    for (int jY = 0; jY < MAP_HEIGHT; jY++) {
                        for (int jX = 0; jX < MAP_WIDTH; jX++) {
                            if (distFW[iY][iX][kY][kX] != INT_MAX && distFW[kY][kX][jY][jX] != INT_MAX) {
                                if (distFW[iY][iX][jY][jX] > distFW[iY][iX][kY][kX] + distFW[kY][kX][jY][jX]) {
                                    distFW[iY][iX][jY][jX] = distFW[iY][iX][kY][kX] + distFW[kY][kX][jY][jX];
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void moveMissile(int map[MAP_HEIGHT][MAP_WIDTH]) {
    if (!missileActive) return;
    int bestDist = INT_MAX;
    Position bestMove = missile;
    for (int i = 0; i < 4; i++) {
        int newX = missile.x + dx[i];
        int newY = missile.y + dy[i];
        if (inRange(newY, newX) && map[newY][newX] != MAP_FLAG_WALL) {
            int distToPlayer = distFW[newY][newX][player.y][player.x];
            if (distToPlayer < bestDist) {
                bestDist = distToPlayer;
                bestMove.x = newX;
                bestMove.y = newY;
            }
        }
    }
    missile = bestMove;
    if (missile.x == player.x && missile.y == player.y) {
        printf("Missile hit the player! Game over.\n");
        exit(0);
    }
}

int main() {
    int map[MAP_HEIGHT][MAP_WIDTH];
    srand((unsigned int)time(NULL));
    memset(map, MAP_FLAG_WALL, sizeof(map));
    COORD startPoint = getRandomStartingPoint();
    generateMap(startPoint.Y, startPoint.X, map);
    floydWarshall(map);
    player.x = 1;
    player.y = 1;
    ai.x = player.x;
    ai.y = player.y;
    exitPoint.x = MAP_WIDTH - 2;
    exitPoint.y = MAP_HEIGHT - 2;
    missile.x = player.x;
    missile.y = player.y;
    int playerMoves = 0;
    char input;
    while (1) {
        printMaze(map);
        printf("Move (w/a/s/d): ");
        scanf(" %c", &input);
        movePlayer(input, map);
        playerMoves++;
        if (playerMoves == 30) {
            missileActive = 1;
            printf("Missile launched!\n");
        }
        if (missileActive) {
            moveMissile(map);
        }
        if (playerMoves >= 20) {
            dijkstraMove(map);
        }
        printMaze(map);
        if (player.x == exitPoint.x && player.y == exitPoint.y) {
            printf("Player wins!\n");
            break;
        }
        if (ai.x == exitPoint.x && ai.y == exitPoint.y) {
            printf("AI wins!\n");
            break;
        }
    }
    return 0;
}