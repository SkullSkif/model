#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <limits>
#include <queue>
#include <iomanip>

struct Point {
    double x, y;
    Point(double x = 0, double y = 0) : x(x), y(y) {}
};

struct Graph {
    int n;
    std::vector<Point> points;
    std::vector<std::vector<int>> adj;
    std::vector<std::vector<double>> distances;
    
    Graph(int n) : n(n), points(n), adj(n), distances(n, std::vector<double>(n, 0)) {}
    
    void computeDistances() {
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                double dx = points[i].x - points[j].x;
                double dy = points[i].y - points[j].y;
                distances[i][j] = distances[j][i] = std::sqrt(dx*dx + dy*dy);
            }
        }
    }
};

class GraphGenerator {
private:
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist01;
    
public:
    GraphGenerator() : rng(std::random_device{}()), dist01(0.0, 1.0) {}
    
    std::vector<Point> generateRandomPoints(int n, double maxCoord = 100.0) {
        std::vector<Point> points(n);
        std::uniform_real_distribution<double> coordDist(0, maxCoord);
        
        for (int i = 0; i < n; ++i) {
            points[i] = Point(coordDist(rng), coordDist(rng));
        }
        return points;
    }
    
    double probabilityExp(double d, double a, double b) {
        return std::exp(-a * std::pow(d, b));
    }
    
    double probabilityInv(double d, double b) {
        if (d == 0) return 1.0;
        return 1.0 / std::pow(d, b);
    }
    
    Graph generateGraph(const std::vector<Point>& points, 
                        std::string probType, 
                        double a, double b,
                        int maxDegree = -1,
                        double maxDistance = -1,
                        bool normalizeProbs = true) {
        Graph g(points.size());
        g.points = points;
        g.computeDistances();
        
        // Строим рёбра для каждой вершины
        for (int i = 0; i < g.n; ++i) {
            std::vector<double> probs(g.n, 0.0);
            double sumProbs = 0.0;
            
            // Вычисляем вероятности для всех возможных рёбер из i
            for (int j = 0; j < g.n; ++j) {
                if (i == j) continue;
                
                double d = g.distances[i][j];
                
                // Проверяем ограничение на максимальное расстояние
                if (maxDistance > 0 && d > maxDistance) {
                    probs[j] = 0;
                    continue;
                }
                
                // Вычисляем вероятность по выбранной формуле
                if (probType == "exp") {
                    probs[j] = probabilityExp(d, a, b);
                } else if (probType == "inv") {
                    probs[j] = probabilityInv(d, b);
                }
                
                sumProbs += probs[j];
            }
            
            // Нормализуем вероятности, если требуется
            if (normalizeProbs && sumProbs > 0) {
                for (int j = 0; j < g.n; ++j) {
                    probs[j] /= sumProbs;
                }
            }
            
            // Разыгрываем количество рёбер для вершины i
            // Количество рёбер определяется случайно на основе вероятностей
            std::vector<int> candidates;
            for (int j = 0; j < g.n; ++j) {
                if (i != j && probs[j] > 0) {
                    candidates.push_back(j);
                }
            }
            
            if (candidates.empty()) continue;
            
            // Случайно выбираем, сколько рёбер создать (от 1 до min(maxDegree, candidates.size()))
            int maxEdges = (maxDegree > 0) ? std::min(maxDegree, (int)candidates.size()) : candidates.size();
            if (maxEdges == 0) continue;
            
            std::uniform_int_distribution<int> edgeCountDist(1, maxEdges);
            int numEdges = edgeCountDist(rng);
            
            // Выбираем вершины для соединения на основе вероятностей
            std::vector<double> candidateProbs;
            for (int j : candidates) {
                candidateProbs.push_back(probs[j]);
            }
            
            std::discrete_distribution<> dist(candidateProbs.begin(), candidateProbs.end());
            
            for (int k = 0; k < numEdges; ++k) {
                int idx = dist(rng);
                int j = candidates[idx];
                
                // Добавляем ребро в обе стороны
                if (std::find(g.adj[i].begin(), g.adj[i].end(), j) == g.adj[i].end()) {
                    g.adj[i].push_back(j);
                    g.adj[j].push_back(i);
                }
            }
        }
        
        return g;
    }
    
    // Вычисление диаметра графа (максимальное кратчайшее расстояние между любыми двумя вершинами)
    int computeDiameter(const Graph& g) {
        int diameter = 0;
        
        for (int start = 0; start < g.n; ++start) {
            std::vector<int> dist(g.n, -1);
            std::queue<int> q;
            
            dist[start] = 0;
            q.push(start);
            
            while (!q.empty()) {
                int v = q.front();
                q.pop();
                
                for (int u : g.adj[v]) {
                    if (dist[u] == -1) {
                        dist[u] = dist[v] + 1;
                        q.push(u);
                    }
                }
            }
            
            for (int i = 0; i < g.n; ++i) {
                if (dist[i] > diameter) {
                    diameter = dist[i];
                }
            }
        }
        
        return diameter;
    }
    
    // Поиск в глубину с ограничением глубины
    void dfs(const Graph& g, int v, std::vector<bool>& visited, int depth, int maxDepth) {
        if (depth > maxDepth) return;
        visited[v] = true;
        
        for (int u : g.adj[v]) {
            if (!visited[u]) {
                dfs(g, u, visited, depth + 1, maxDepth);
            }
        }
    }
    
    // Проверка, что все вершины достижимы из start с глубиной не более maxDepth
    bool checkDepthConstraint(const Graph& g, int start, int maxDepth) {
        std::vector<bool> visited(g.n, false);
        dfs(g, start, visited, 0, maxDepth);
        
        return std::all_of(visited.begin(), visited.end(), [](bool v) { return v; });
    }
    
    // Построение дерева из графа с учётом ограничений
    Graph buildTreeWithConstraints(const Graph& g, int maxDepth) {
        Graph tree(g.n);
        tree.points = g.points;
        
        std::vector<bool> visited(g.n, false);
        std::queue<int> q;
        
        // Выбираем случайную стартовую вершину
        std::uniform_int_distribution<int> startDist(0, g.n - 1);
        int start = startDist(rng);
        
        visited[start] = true;
        q.push(start);
        
        std::vector<int> parent(g.n, -1);
        std::vector<int> depth(g.n, 0);
        
        while (!q.empty()) {
            int v = q.front();
            q.pop();
            
            if (depth[v] >= maxDepth) continue;
            
            // Перемешиваем соседей для случайности
            std::vector<int> neighbors = g.adj[v];
            std::shuffle(neighbors.begin(), neighbors.end(), rng);
            
            for (int u : neighbors) {
                if (!visited[u]) {
                    visited[u] = true;
                    parent[u] = v;
                    depth[u] = depth[v] + 1;
                    q.push(u);
                    
                    tree.adj[v].push_back(u);
                    tree.adj[u].push_back(v);
                }
            }
        }
        
        // Проверяем, все ли вершины достигнуты
        bool allVisited = std::all_of(visited.begin(), visited.end(), [](bool v) { return v; });
        
        if (!allVisited) {
            std::cout << "Предупреждение: не все вершины достигнуты при maxDepth = " << maxDepth << std::endl;
        }
        
        return tree;
    }
};

void printGraphInfo(const Graph& g, const std::string& name, int diameter, int maxDepth) {
    std::cout << "\n=== " << name << " ===" << std::endl;
    std::cout << "Количество вершин: " << g.n << std::endl;
    std::cout << "Диаметр графа: " << diameter << std::endl;
    std::cout << "Допустимая глубина дерева (половина диаметра): " << maxDepth << std::endl;
    
    // Статистика по степеням вершин
    int maxDegree = 0;
    double avgDegree = 0;
    for (int i = 0; i < g.n; ++i) {
        int degree = g.adj[i].size();
        maxDegree = std::max(maxDegree, degree);
        avgDegree += degree;
    }
    avgDegree /= g.n;
    
    std::cout << "Максимальная степень вершины: " << maxDegree << std::endl;
    std::cout << "Средняя степень вершины: " << std::fixed << std::setprecision(2) << avgDegree << std::endl;
    std::cout << "Количество рёбер: " << avgDegree * g.n / 2 << std::endl;
}

int main() {
    GraphGenerator generator;
    
    // Генерируем 100 случайных точек
    std::vector<Point> points = generator.generateRandomPoints(100, 100.0);
    
    // Различные параметры для генерации графов
    struct ParamSet {
        std::string name;
        std::string probType;
        double a;
        double b;
        int maxDegree;
        double maxDistance;
    };
    
    std::vector<ParamSet> params = {
        {"Экспоненциальный 1", "exp", 0.1, 1.0, -1, -1},
        {"Экспоненциальный 2", "exp", 0.01, 2.0, 10, -1},
        {"Экспоненциальный 3", "exp", 0.05, 1.5, -1, 50},
        {"Экспоненциальный 4", "exp", 0.1, 0.5, 15, 80},
        {"Экспоненциальный 5", "exp", 0.001, 3.0, -1, 30},
        {"Обратный 1", "inv", 0, 1.0, -1, -1},
        {"Обратный 2", "inv", 0, 2.0, 8, -1},
        {"Обратный 3", "inv", 0, 1.5, -1, 60},
        {"Обратный 4", "inv", 0, 0.5, 12, 70},
        {"Обратный 5", "inv", 0, 2.5, 5, 40}
    };
    
    std::vector<Graph> graphs;
    
    // Генерируем графы с разными параметрами
    for (const auto& p : params) {
        Graph g = generator.generateGraph(points, p.probType, p.a, p.b, p.maxDegree, p.maxDistance);
        graphs.push_back(g);
        
        int diameter = generator.computeDiameter(g);
        int maxDepth = diameter / 2;
        
        printGraphInfo(g, p.name, diameter, maxDepth);
        
        // Строим дерево с ограничением по глубине
        Graph tree = generator.buildTreeWithConstraints(g, maxDepth);
        int treeDiameter = generator.computeDiameter(tree);
        
        std::cout << "--- Дерево из графа ---" << std::endl;
        std::cout << "Диаметр дерева: " << treeDiameter << std::endl;
        std::cout << "Проверка глубины: " 
                  << (generator.checkDepthConstraint(tree, 0, maxDepth) ? "OK" : "Не все вершины достигнуты") 
                  << std::endl;
    }
    
    // Дополнительная статистика по точкам
    std::cout << "\n=== Статистика по точкам ===" << std::endl;
    double minDist = std::numeric_limits<double>::max();
    double maxDist = 0;
    double avgDist = 0;
    int pairCount = 0;
    
    for (size_t i = 0; i < points.size(); ++i) {
        for (size_t j = i + 1; j < points.size(); ++j) {
            double dx = points[i].x - points[j].x;
            double dy = points[i].y - points[j].y;
            double dist = std::sqrt(dx*dx + dy*dy);
            
            minDist = std::min(minDist, dist);
            maxDist = std::max(maxDist, dist);
            avgDist += dist;
            pairCount++;
        }
    }
    avgDist /= pairCount;
    
    std::cout << "Минимальное расстояние между точками: " << std::fixed << std::setprecision(2) << minDist << std::endl;
    std::cout << "Максимальное расстояние между точками: " << maxDist << std::endl;
    std::cout << "Среднее расстояние между точками: " << avgDist << std::endl;
    
    return 0;
}