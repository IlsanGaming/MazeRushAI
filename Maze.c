#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h> // COORD 사용
#include <string.h>  // memset 사용
#include <time.h>    // 시간 설정
#include <limits.h>  // INT_MAX 사용

// 맵 크기 설정
#define MAP_WIDTH 51
#define MAP_HEIGHT 31


// 방향 정의 (좌, 상, 우, 하)
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
    point.X = 1 + (rand() % (MAP_WIDTH - 2)); // X를 사용
    point.Y = 1 + (rand() % (MAP_HEIGHT - 2)); // Y를 사용

    // X와 Y가 홀수 좌표가 되도록 조정
    if (point.X % 2 == 0) point.X++;
    if (point.Y % 2 == 0) point.Y++;

    return point;
}
typedef struct {
    int x, y;
    int dist;
} Node;
// 우선순위 큐를 대체할 배열 및 변수
Node queue[MAP_WIDTH * MAP_HEIGHT];
int front = 0, rear = 0;


// 상, 하, 좌, 우 이동 (2칸씩 이동)
const int DIR[4][2] = { {0, -2}, {0, 2}, {-2, 0}, {2, 0} };

// 방향 이동 벡터
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

// 함수 선언
void generateMap(int y, int x, int map[MAP_HEIGHT][MAP_WIDTH]);
void shuffleArray(int array[], int size);
int inRange(int y, int x);
COORD getRandomStartingPoint();
void printMaze(int map[MAP_HEIGHT][MAP_WIDTH]);
void movePlayer(char direction, int map[MAP_HEIGHT][MAP_WIDTH]);
void aiMove(int map[MAP_HEIGHT][MAP_WIDTH]);
int distFW[MAP_HEIGHT][MAP_WIDTH][MAP_HEIGHT][MAP_WIDTH];
int missileActive = 0; // 미사일 활성화 여부

// 미로 생성 및 탐색을 위한 전역 변수
int dist[MAP_HEIGHT][MAP_WIDTH];

// 미로 출력 함수
void printMaze(int map[MAP_HEIGHT][MAP_WIDTH]) {
    system("cls"); // 콘솔 화면 지우기 (Windows 환경)
    for (int i = 0; i < MAP_HEIGHT; ++i) {
        for (int j = 0; j < MAP_WIDTH; ++j) {
            if (i == player.y && j == player.x) {
                printf("P "); // 플레이어
            }
            else if (i == ai.y && j == ai.x) {
                printf("A "); // AI
            }
            else if (missileActive && i == missile.y && j == missile.x) {
                printf("M "); // 미사일
            }
            else if (i == exitPoint.y && j == exitPoint.x) {
                printf("E "); // 출구
            }
            else {
                printf("%s", map[i][j] == MAP_FLAG_WALL ? "# " : "  "); // 벽 및 빈 공간
            }
        }
        printf("\n");
    }
}

// 방향 배열을 무작위로 섞기
void shuffleArray(int array[], int size) {
    for (int i = 0; i < size - 1; ++i) {
        int r = i + rand() % (size - i);
        int temp = array[i];
        array[i] = array[r];
        array[r] = temp;
    }
}

// 범위 체크
int inRange(int y, int x) {
    return (y < MAP_HEIGHT - 1 && y > 0) && (x < MAP_WIDTH - 1 && x > 0);
}

// 깊이 우선 탐색을 사용한 랜덤 미로 생성
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

// 플레이어 이동 함수
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
    // 큐 초기화
    front = 0;
    rear = 0;

    int dist[MAP_HEIGHT][MAP_WIDTH];
    Position prev[MAP_HEIGHT][MAP_WIDTH];
    int visited[MAP_HEIGHT][MAP_WIDTH] = { 0 };

    // 거리 배열 초기화
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            dist[i][j] = INT_MAX;
        }
    }

    // 초기 위치 설정
    dist[ai.y][ai.x] = 0;
    enqueue((Node) { ai.x, ai.y, 0 });
    visited[ai.y][ai.x] = 1; // AI의 시작 위치를 방문 처리

    // 다익스트라 알고리즘을 사용해 최단 경로 탐색
    while (!isQueueEmpty()) {
        Node current = dequeue();

        for (int i = 0; i < 4; i++) {
            int newX = current.x + dx[i];
            int newY = current.y + dy[i];

            // 유효한 좌표인지 확인 및 벽이 아니고 방문하지 않았는지 확인
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

    // 출구로 가는 최단 경로 추적
    Position nextMove = exitPoint;
    while (prev[nextMove.y][nextMove.x].x != ai.x || prev[nextMove.y][nextMove.x].y != ai.y) {
        nextMove = prev[nextMove.y][nextMove.x];
    }

    // AI 이동
    ai.x = nextMove.x;
    ai.y = nextMove.y;
}
// 플로이드-워셜 알고리즘을 이용해 미로 내 모든 위치 간 최단 경로를 계산 (벽 고려)
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

    // 플로이드-워셜 알고리즘 수행
    for (int kY = 0; kY < MAP_HEIGHT; kY++) {
        for (int kX = 0; kX < MAP_WIDTH; kX++) {
            if (map[kY][kX] == MAP_FLAG_WALL) continue; // 벽은 경로로 사용할 수 없음
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

    printf("Missile current position: (%d, %d)\n", missile.x, missile.y);

    int bestDist = INT_MAX;
    Position bestMove = missile;

    for (int i = 0; i < 4; i++) {
        int newX = missile.x + dx[i];
        int newY = missile.y + dy[i];

        // 유효한 좌표인지 확인하고 벽이 아닌지 확인
        if (inRange(newY, newX) && map[newY][newX] != MAP_FLAG_WALL) {
            int distToPlayer = distFW[newY][newX][player.y][player.x];
            if (distToPlayer < bestDist) {
                bestDist = distToPlayer;
                bestMove.x = newX;
                bestMove.y = newY;
            }
        }
    }

    // 미사일 이동
    missile = bestMove;
    printf("Missile moved to: (%d, %d)\n", missile.x, missile.y);

    // 미사일이 플레이어를 잡으면 게임 종료
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

    // 플로이드-워셜 알고리즘 초기화
    floydWarshall(map);

    // 시작 및 종료 지점 설정
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

        // 플레이어 이동
        printf("Move (w/a/s/d): ");
        scanf(" %c", &input);
        movePlayer(input, map);
        playerMoves++;

        // 30턴 후 미사일 활성화
        if (playerMoves == 30) {
            missileActive = 1;
            printf("Missile launched!\n");
        }

        // 미사일 이동
        if (missileActive) {
            moveMissile(map);
        }

        // AI 추적
        if (playerMoves >= 20) {
            dijkstraMove(map);
        }

        printMaze(map);

        // 게임 종료 조건 확인
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



