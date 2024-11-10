# 🕹️ Maze Rush: AI vs Player

## 📌 프로젝트 소개
**Maze Rush: AI vs Player**는 **플레이어와 AI가 무작위로 생성된 미로에서 출구를 향해 경쟁하는** C 언어 기반 콘솔 게임입니다. 이 게임은 AI와 플레이어가 동시에 출발하여 **누가 더 빨리 미로를 탈출할 수 있는지** 겨루는 방식으로 진행됩니다. 각 게임은 매번 새로운 미로를 생성하므로, 플레이어는 매번 다른 도전에 직면하게 됩니다.

## 🎮 주요 기능
- **랜덤 미로 생성**: 게임이 시작될 때마다 무작위로 미로가 생성됩니다.
- **플레이어 vs AI 경쟁**: 플레이어와 AI는 동시에 출발하여 출구를 향해 경쟁합니다.
- **AI 경로 탐색**: AI는 다익스트라(Dijkstra's Algorithm)를 사용하여 최단 경로를 찾아갑니다.
- **유도 미사일 기능**: 플레이어가 일정 횟수 이동한 후, 유도 미사일이 발사되어 플레이어를 추적합니다.
  - 유도 미사일은 플로이드-워셜(Floyd-Warshall Algorithm)을 사용해 플레이어의 위치를 추적합니다.
  - 미사일은 벽을 피해 미로의 길을 따라 이동합니다.
  
## 🕹️ 조작 방법
- **w**: 위로 이동
- **a**: 왼쪽으로 이동
- **s**: 아래로 이동
- **d**: 오른쪽으로 이동
- **q**: 게임 종료

## ⚙️ 시스템 요구 사항
- Windows 환경
- C 언어 컴파일러 (예: GCC, Visual Studio)

## 🛠️ 설치 및 실행 방법
1. 이 저장소를 클론합니다.
    ```bash
    git clone https://github.com/IlsanGaming/MazeRushAI
    cd maze-rush-ai-vs-player
    ```
2. 프로젝트를 컴파일합니다.
    ```bash
    gcc -o MazeRush main.c
    ```
3. 게임을 실행합니다.
    ```bash
    ./MazeRush
    ```

## 📂 프로젝트 구조
```
maze-rush-ai-vs-player/
├── main.c                # 게임 메인 코드
├── maze_generator.c      # 미로 생성 코드 (DFS 기반)
├── ai_algorithm.c        # AI 경로 탐색 코드 (Dijkstra)
├── missile_tracker.c     # 유도 미사일 코드 (Floyd-Warshall)
├── README.md             # 프로젝트 설명 파일
└── LICENSE               # 라이선스 파일
```

## 📈 향후 개선 사항
- AI의 경로 탐색 알고리즘 개선 (A* 알고리즘 도입)
- 멀티플레이어 모드 추가
- 그래픽 사용자 인터페이스(GUI)로 전환하여 시각적 효과 강화

## 📄 라이선스
이 프로젝트는 **MIT 라이선스**에 따라 제공됩니다. 자세한 내용은 `LICENSE` 파일을 참고하세요.