/**
 * 地铁线路图查询器（学生版）
 * 实验任务：完成所有标记为 // TODO 的函数实现。
 * 编译：gcc -o metro metro_student.c -std=c99
 * 运行：./metro
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>

#define MAX_NAME_LEN 32
#define MAX_LINE_NAME 20

// 邻接表边结点
typedef struct EdgeNode {
    int adjVex;         // 邻接站点编号
    int weight;         // 权值(运行时间，分钟)
    struct EdgeNode *next;
} EdgeNode;

// 顶点结点(站点)
typedef struct VertexNode {
    char name[MAX_NAME_LEN]; // 站点名称
    EdgeNode *firstEdge;     // 第一条边
    int *lineIds;            // 所属线路编号数组(动态分配)
    int lineCount;           // 线路数量
} VertexNode;

// 图结构
typedef struct {
    VertexNode *vertices;   // 顶点数组
    int vertexNum;           // 实际顶点数
    int vertexCapacity;     // 顶点数组容量
    int edgeNum;             // 总边数
    int isDirected;          // 0无向，1有向(本实验0)
} Graph;

// BFS辅助队列
typedef struct Queue {
    int *data;
    int front, rear, size, capacity;
} Queue;

// 函数声明
Graph* createGraph(int initCapacity, int isDirected);
void resizeGraph(Graph *g);
int addVertex(Graph *g, const char *name);
int findVertexIndex(Graph *g, const char *name);
void addEdge(Graph *g, int u, int v, int weight);
void addLineToStation(Graph *g, int stationIdx, int lineId);
void readMetroFile(const char *filename, Graph *g);
void printAdjList(Graph *g);

// TODO待实现函数
void DFSRecursive(Graph *g, int v, int *visited);
void DFSTraversal(Graph *g, int start);
void BFSTraversal(Graph *g, int start);
void connectivityAnalysis(Graph *g);
void dijkstra(Graph *g, int start, int *dist, int *prev);
void printPath(Graph *g, int *prev, int start, int end);
void shortestPathByTime(Graph *g, int start, int end);
void shortestPathByTransfer(Graph *g, int start, int end);
void freeGraph(Graph *g);

void printMenu();

// 队列操作函数
Queue* createQueue(int capacity);
void enqueue(Queue *q, int val);
int dequeue(Queue *q);
int isEmpty(Queue *q);
void freeQueue(Queue *q);

// ===================== 主函数 =====================
int main() {
    Graph *g = createGraph(100, 0); // 无向图
    readMetroFile("metro.txt", g);

    int choice, start, end;
    char startName[MAX_NAME_LEN], endName[MAX_NAME_LEN];
    do {
        printMenu();
        printf("请输入选择：");
        scanf("%d", &choice);
        getchar(); // 吸收换行符

        switch (choice) {
            case 1:
                printAdjList(g);
                break;
            case 2:
                printf("请输入起始站点名称：");
                fgets(startName, MAX_NAME_LEN, stdin);
                startName[strcspn(startName, "\n")] = '\0';
                start = findVertexIndex(g, startName);
                if (start == -1) {
                    fprintf(stderr, "错误：站点 '%s' 不存在。\n", startName);
                } else {
                    printf("\nDFS 遍历序列（从 %s 开始）：\n", startName);
                    DFSTraversal(g, start);
                }
                break;
            case 3:
                printf("请输入起始站点名称：");
                fgets(startName, MAX_NAME_LEN, stdin);
                startName[strcspn(startName, "\n")] = '\0';
                start = findVertexIndex(g, startName);
                if (start == -1) {
                    fprintf(stderr, "错误：站点 '%s' 不存在。\n", startName);
                } else {
                    printf("\nBFS 遍历序列（从 %s 开始）：\n", startName);
                    BFSTraversal(g, start);
                }
                break;
            case 4:
                connectivityAnalysis(g);
                break;
            case 5:
                printf("请输入起点站：");
                fgets(startName, MAX_NAME_LEN, stdin);
                startName[strcspn(startName, "\n")] = '\0';
                printf("请输入终点站：");
                fgets(endName, MAX_NAME_LEN, stdin);
                endName[strcspn(endName, "\n")] = '\0';
                start = findVertexIndex(g, startName);
                end = findVertexIndex(g, endName);
                if (start == -1) {
                    fprintf(stderr, "错误：起点 '%s' 不存在。\n", startName);
                } else if (end == -1) {
                    fprintf(stderr, "错误：终点 '%s' 不存在。\n", endName);
                } else {
                    shortestPathByTime(g, start, end);
                }
                break;
            case 6:
                printf("请输入起点站：");
                fgets(startName, MAX_NAME_LEN, stdin);
                startName[strcspn(startName, "\n")] = '\0';
                printf("请输入终点站：");
                fgets(endName, MAX_NAME_LEN, stdin);
                endName[strcspn(endName, "\n")] = '\0';
                start = findVertexIndex(g, startName);
                end = findVertexIndex(g, endName);
                if (start == -1) {
                    fprintf(stderr, "错误：起点 '%s' 不存在。\n", startName);
                } else if (end == -1) {
                    fprintf(stderr, "错误：终点 '%s' 不存在。\n", endName);
                } else {
                    shortestPathByTransfer(g, start, end);
                }
                break;
            case 0:
                printf("退出程序。\n");
                break;
            default:
                printf("无效选择，请重新输入。\n");
        }
        printf("\n");
    } while (choice != 0);

    freeGraph(g);
    return 0;
}

// ===================== 已实现基础图函数（截图原样保留） =====================
Graph* createGraph(int initCapacity, int isDirected) {
    Graph *g = (Graph*)malloc(sizeof(Graph));
    g->vertexCapacity = initCapacity;
    g->vertexNum = 0;
    g->edgeNum = 0;
    g->isDirected = isDirected;
    g->vertices = (VertexNode*)malloc(sizeof(VertexNode) * initCapacity);
    for (int i = 0; i < initCapacity; i++) {
        g->vertices[i].name[0] = '\0';
        g->vertices[i].firstEdge = NULL;
        g->vertices[i].lineIds = NULL;
        g->vertices[i].lineCount = 0;
    }
    return g;
}

void resizeGraph(Graph *g) {
    int newCap = g->vertexCapacity * 2;
    g->vertices = (VertexNode*)realloc(g->vertices, sizeof(VertexNode) * newCap);
    for (int i = g->vertexCapacity; i < newCap; i++) {
        g->vertices[i].name[0] = '\0';
        g->vertices[i].firstEdge = NULL;
        g->vertices[i].lineIds = NULL;
        g->vertices[i].lineCount = 0;
    }
    g->vertexCapacity = newCap;
}

int addVertex(Graph *g, const char *name) {
    int idx = findVertexIndex(g, name);
    if (idx != -1) return idx;
    if (g->vertexNum >= g->vertexCapacity) {
        resizeGraph(g);
    }
    strcpy(g->vertices[g->vertexNum].name, name);
    g->vertices[g->vertexNum].firstEdge = NULL;
    g->vertices[g->vertexNum].lineIds = NULL;
    g->vertices[g->vertexNum].lineCount = 0;
    return g->vertexNum++;
}

int findVertexIndex(Graph *g, const char *name) {
    for (int i = 0; i < g->vertexNum; i++) {
        if (strcmp(g->vertices[i].name, name) == 0)
            return i;
    }
    return -1;
}

void addEdge(Graph *g, int u, int v, int weight) {
    if (u < 0 || u >= g->vertexNum || v < 0 || v >= g->vertexNum) return;
    EdgeNode *e = (EdgeNode*)malloc(sizeof(EdgeNode));
    e->adjVex = v;
    e->weight = weight;
    e->next = g->vertices[u].firstEdge;
    g->vertices[u].firstEdge = e;

    if (!g->isDirected) {
        e = (EdgeNode*)malloc(sizeof(EdgeNode));
        e->adjVex = u;
        e->weight = weight;
        e->next = g->vertices[v].firstEdge;
        g->vertices[v].firstEdge = e;
    }
    g->edgeNum++;
}

void addLineToStation(Graph *g, int stationIdx, int lineId) {
    if (stationIdx < 0 || stationIdx >= g->vertexNum) return;
    for (int i = 0; i < g->vertices[stationIdx].lineCount; i++) {
        if (g->vertices[stationIdx].lineIds[i] == lineId)
            return;
    }
    g->vertices[stationIdx].lineCount++;
    g->vertices[stationIdx].lineIds = (int*)realloc(g->vertices[stationIdx].lineIds, sizeof(int) * g->vertices[stationIdx].lineCount);
    g->vertices[stationIdx].lineIds[g->vertices[stationIdx].lineCount - 1] = lineId;
}

void readMetroFile(const char *filename, Graph *g) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "无法打开文件 %s\n", filename);
        exit(1);
    }
    char line[256];
    int routeCount = 0;
    // 跳过注释行读取线路总数
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        sscanf(line, "%d", &routeCount);
        break;
    }
    // 读取线路数字行
    fgets(line, sizeof(line), fp);
    for (int rid = 0; rid < routeCount; rid++) {
        if (!fgets(line, sizeof(line), fp)) break;
        if (line[0] == '#' || line[0] == '\n') {
            rid--;
            continue;
        }
        char lineName[MAX_LINE_NAME];
        int stationCount;
        char *token = strtok(line, " \t\n");
        if (!token) continue;
        strcpy(lineName, token);
        token = strtok(NULL, " \t\n");
        if (!token) continue;
        stationCount = atoi(token);
        int prevStation = -1;
        int timeVal = 1;
        for (int i = 0; i < stationCount; i++) {
            token = strtok(NULL, " \t\n");
            if (!token) break;
            // 判断是否为数字(时间)
            int isTime = 1;
            for (char *n = token; *n; n++) {
                if (!isdigit(*n)) {
                    isTime = 0;
                    break;
                }
            }
            if (isTime) {
                timeVal = atoi(token);
                continue;
            }
            // 新增站点
            int cur = addVertex(g, token);
            addLineToStation(g, cur, rid);
            // 相邻站点建边
            if (prevStation != -1) {
                addEdge(g, prevStation, cur, timeVal);
                timeVal = 1;
            }
            prevStation = cur;
        }
    }
    fclose(fp);
    printf("成功读取地铁数据：共 %d 个站点，%d 条边。\n", g->vertexNum, g->edgeNum);
}

void printAdjList(Graph *g) {
    printf("\n===== 邻接表 =====\n");
    for (int i = 0; i < g->vertexNum; i++) {
        printf("%s (%d条线路): ", g->vertices[i].name, g->vertices[i].lineCount);
        EdgeNode *p = g->vertices[i].firstEdge;
        while (p != NULL) {
            printf("-> %s(%dmin) ", g->vertices[p->adjVex].name, p->weight);
            p = p->next;
        }
        printf("\n");
    }
    // 打印换乘站
    printf("\n===== 换乘站 =====\n");
    for (int i = 0; i < g->vertexNum; i++) {
        if (g->vertices[i].lineCount > 1) {
            printf("%s：%d 条线路\n", g->vertices[i].name, g->vertices[i].lineCount);
        }
    }
}

void printMenu() {
    printf("\n===== 地铁查询系统 =====\n");
    printf("1. 输出邻接表和换乘站\n");
    printf("2. DFS 遍历（从指定站点）\n");
    printf("3. BFS 遍历（从指定站点）\n");
    printf("4. 连通分量分析\n");
    printf("5. 最短路径（最少时间）\n");
    printf("6. 最短路径（最少换乘）\n");
    printf("0. 退出\n");
}

// ===================== 队列完整实现（截图原样） =====================
Queue* createQueue(int capacity) {
    Queue *q = (Queue*)malloc(sizeof(Queue));
    q->data = (int*)malloc(sizeof(int) * capacity);
    q->front = q->rear = q->size = 0;
    q->capacity = capacity;
    return q;
}

void enqueue(Queue *q, int val) {
    if (q->size == q->capacity) return;
    q->data[q->rear] = val;
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
}

int dequeue(Queue *q) {
    if (q->size == 0) return -1;
    int val = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return val;
}

int isEmpty(Queue *q) {
    return q->size == 0;
}

void freeQueue(Queue *q) {
    if (q == NULL) return;
    free(q->data);
    free(q);
}

// ===================== 全部TODO函数（按实验要求完整实现） =====================
// 1. DFS递归核心
void DFSRecursive(Graph *g, int v, int *visited) {
    visited[v] = 1;
    printf("%s ", g->vertices[v].name);
    EdgeNode *p = g->vertices[v].firstEdge;
    while(p != NULL)
    {
        int adj = p->adjVex;
        if(!visited[adj])
        {
            DFSRecursive(g, adj, visited);
        }
        p = p->next;
    }
}
void DFSTraversal(Graph *g, int start) {
    if(g == NULL || start < 0 || start >= g->vertexNum)
        return;
    int *visited = (int*)calloc(g->vertexNum, sizeof(int));
    DFSRecursive(g, start, visited);
    printf("\n");
    fflush(stdout); // 强制刷新缓冲区，立刻打印
    free(visited);
}

// DFS入口，分配释放visited数组
void DFSTraversal(Graph *g, int start) {
    if (g == NULL || start < 0 || start >= g->vertexNum) return;
    int *visited = (int*)calloc(g->vertexNum, sizeof(int));
    if (visited == NULL) {
        fprintf(stderr, "内存分配失败\n");
        return;
    }
    DFSRecursive(g, start, visited);
    printf("\n");
    free(visited);
}

// 2. BFS广度优先遍历
void BFSTraversal(Graph *g, int start) {
    if(g == NULL || start < 0 || start >= g->vertexNum)
        return;
    int *visited = (int*)calloc(g->vertexNum, sizeof(int));
    Queue *q = createQueue(g->vertexNum);
    visited[start] = 1;
    enqueue(q, start);
    while(!isEmpty(q))
    {
        int cur = dequeue(q);
        printf("%s ", g->vertices[cur].name);
        EdgeNode *p = g->vertices[cur].firstEdge;
        while(p != NULL)
        {
            int adj = p->adjVex;
            if(!visited[adj])
            {
                visited[adj] = 1;
                enqueue(q, adj);
            }
            p = p->next;
        }
    }
    printf("\n");
    fflush(stdout);
    freeQueue(q);
    free(visited);
}
// 3. 连通分量分析
void connectivityAnalysis(Graph *g) {
    if(g == NULL || g->vertexNum == 0)
        return;
    int *visited = (int*)calloc(g->vertexNum, sizeof(int));
    int compCnt = 0;
    for(int i = 0; i < g->vertexNum; i++)
    {
        if(!visited[i])
        {
            compCnt++;
            printf("===== 连通分量 %d =====\n站点列表：", compCnt);
            DFSRecursive(g, i, visited);
            printf("\n");
        }
    }
    printf("总连通分量数量：%d\n", compCnt);
    fflush(stdout);
    free(visited);
}
// 4. Dijkstra 数组实现
void dijkstra(Graph *g, int start, int *dist, int *prev) {
    int n = g->vertexNum;
    int *vis = (int*)calloc(n, sizeof(int));
    // 初始化
    for (int i = 0; i < n; i++) {
        dist[i] = INT_MAX;
        prev[i] = -1;
    }
    dist[start] = 0;
    for (int i = 0; i < n; i++) {
        // 选未访问最小dist顶点
        int u = -1;
        int minDis = INT_MAX;
        for (int j = 0; j < n; j++) {
            if (!vis[j] && dist[j] < minDis) {
                minDis = dist[j];
                u = j;
            }
        }
        if (u == -1) break;
        vis[u] = 1;
        // 松弛边
        EdgeNode *p = g->vertices[u].firstEdge;
        while (p != NULL) {
            int v = p->adjVex;
            int w = p->weight;
            if (!vis[v] && dist[u] != INT_MAX && dist[v] > dist[u] + w) {
                dist[v] = dist[u] + w;
                prev[v] = u;
            }
            p = p->next;
        }
    }
    free(vis);
}

// 5. 递归打印路径
void printPath(Graph *g, int *prev, int start, int end) {
    if(end == -1)
    {
        printf("无法到达");
        return;
    }
    if(end == start)
    {
        printf("%s", g->vertices[start].name);
        return;
    }
    printPath(g, prev, start, prev[end]);
    printf(" -> %s", g->vertices[end].name);
}
// 6. 最少时间路径查询
void shortestPathByTime(Graph *g, int start, int end) {
    int n = g->vertexNum;
    int *dist = (int*)malloc(n * sizeof(int));
    int *prev = (int*)malloc(n * sizeof(int));
    dijkstra(g, start, dist, prev);
    if (dist[end] == INT_MAX) {
        printf("两点无连通路径\n");
    } else {
        printf("最短时间路径（总时间 %d 分钟）：", dist[end]);
        printPath(g, prev, start, end);
        printf("\n");
    }
    free(dist);
    free(prev);
}

// 7. 最少换乘路径（临时权重置1，计算后复原）
void shortestPathByTransfer(Graph *g, int start, int end) {
    int n = g->vertexNum;
    int totalEdge = g->edgeNum;
    int *originalWeights = (int*)malloc(totalEdge * sizeof(int));
    int idx = 0;
    // 备份所有权重，临时改为1
    for (int i = 0; i < g->vertexNum; i++) {
        EdgeNode *p = g->vertices[i].firstEdge;
        while (p != NULL) {
            originalWeights[idx++] = p->weight;
            p->weight = 1;
            p = p->next;
        }
    }
    // 跑dijkstra
    int *dist = (int*)malloc(n * sizeof(int));
    int *prev = (int*)malloc(n * sizeof(int));
    dijkstra(g, start, dist, prev);
    // 输出结果
    if (dist[end] == INT_MAX) {
        printf("两点无连通路径\n");
    } else {
        printf("最少换乘路径（换乘 %d 次）：", dist[end] - 1);
        printPath(g, prev, start, end);
        printf("\n");
    }
    // 恢复原始权重
    idx = 0;
    for (int i = 0; i < g->vertexNum; i++) {
        EdgeNode *p = g->vertices[i].firstEdge;
        while (p != NULL) {
            p->weight = originalWeights[idx++];
            p = p->next;
        }
    }
    free(dist);
    free(prev);
    free(originalWeights);
}

// 8. 完整释放整张图内存 freeGraph
void freeGraph(Graph *g) {
    if (g == NULL) return;
    // 释放每个顶点的边链表 + lineIds数组
    for (int i = 0; i < g->vertexNum; i++) {
        EdgeNode *p = g->vertices[i].firstEdge;
        while (p != NULL) {
            EdgeNode *tmp = p;
            p = p->next;
            free(tmp);
        }
        free(g->vertices[i].lineIds);
    }
    free(g->vertices);
    free(g);
}