#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <limits>
#include <queue>
#include <iomanip>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <tuple>

struct Point {
    double x, y;
    Point(double x = 0, double y = 0) : x(x), y(y) {}
};

struct Graph {
    int n;
    std::vector<Point> points;
    std::vector<std::vector<int>> adj;
    std::vector<std::vector<double>> distances;
    int startNode; // Для визуализации начальной вершины дерева
    std::string name;
    
    Graph(int n, const std::string& name = "") : n(n), points(n), adj(n), 
                                                  distances(n, std::vector<double>(n, 0)), 
                                                  startNode(0), name(name) {}
    
    void computeDistances() {
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                double dx = points[i].x - points[j].x;
                double dy = points[i].y - points[j].y;
                distances[i][j] = distances[j][i] = std::sqrt(dx*dx + dy*dy);
            }
        }
    }
    
    // Очистка графа
    void clear() {
        for (auto& neighbors : adj) {
            neighbors.clear();
        }
    }
    
    // Получение статистики - делаем метод константным
    std::tuple<int, double, int> getStats() const {
        int maxDegree = 0;
        double avgDegree = 0;
        int edgeCount = 0;
        
        for (int i = 0; i < n; ++i) {
            int degree = adj[i].size();
            maxDegree = std::max(maxDegree, degree);
            avgDegree += degree;
            edgeCount += degree;
        }
        edgeCount /= 2;
        avgDegree /= n;
        
        return {maxDegree, avgDegree, edgeCount};
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
    
    double probabilityExp(double d, double a, double b) const {
        return std::exp(-a * std::pow(d, b));
    }
    
    double probabilityInv(double d, double b) const {
        if (d == 0) return 1.0;
        return 1.0 / std::pow(d, b);
    }
    
    Graph generateGraph(const std::vector<Point>& points, 
                        const std::string& graphName,
                        const std::string& probType, 
                        double a, double b,
                        int maxDegree = -1,
                        double maxDistance = -1,
                        bool normalizeProbs = true) {
        Graph g(points.size(), graphName);
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
            std::vector<int> candidates;
            for (int j = 0; j < g.n; ++j) {
                if (i != j && probs[j] > 0) {
                    candidates.push_back(j);
                }
            }
            
            if (candidates.empty()) continue;
            
            // Случайно выбираем, сколько рёбер создать
            int maxEdges = (maxDegree > 0) ? std::min(maxDegree, (int)candidates.size()) : 
                           std::min(5, (int)candidates.size()); // Ограничиваем максимум 5 рёбрами на вершину для читаемости
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
    
    // Вычисление диаметра графа
    int computeDiameter(const Graph& g) const {
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
    
    // Построение дерева из графа с учётом ограничений
    Graph buildTreeWithConstraints(const Graph& g, int maxDepth, const std::string& treeName) {
        Graph tree(g.n, treeName);
        tree.points = g.points;
        
        std::vector<bool> visited(g.n, false);
        std::queue<int> q;
        
        // Выбираем случайную стартовую вершину
        std::uniform_int_distribution<int> startDist(0, g.n - 1);
        int start = startDist(rng);
        tree.startNode = start;
        
        visited[start] = true;
        q.push(start);
        
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
                    depth[u] = depth[v] + 1;
                    q.push(u);
                    
                    tree.adj[v].push_back(u);
                    tree.adj[u].push_back(v);
                }
            }
        }
        
        return tree;
    }
};

class GraphVizExporter {
private:
    std::string outputDir;
    
    // Делаем метод публичным или оставляем приватным но добавляем публичный интерфейс
    std::string sanitizeName(const std::string& name) const {
        std::string result = name;
        std::replace(result.begin(), result.end(), ' ', '_');
        std::replace(result.begin(), result.end(), '=', '-');
        std::replace(result.begin(), result.end(), '.', '_');
        return result;
    }
    
public:
    GraphVizExporter(const std::string& dir = "graphviz_output") : outputDir(dir) {
        // Создаём директорию для выходных файлов
        std::string command = "mkdir -p " + outputDir;
        system(command.c_str());
    }
    
    // Публичный метод для доступа к sanitizeName
    std::string getSanitizedName(const std::string& name) const {
        return sanitizeName(name);
    }
    
    void exportToDot(const Graph& g, const std::string& filename, bool isTree = false) const {
        std::ofstream dotFile(outputDir + "/" + filename + ".dot");
        
        dotFile << "graph " << sanitizeName(g.name) << " {\n";
        dotFile << "  layout=fdp;\n";  // Используем fdp для лучшего отображения заданных координат
        dotFile << "  overlap=false;\n";
        dotFile << "  splines=true;\n";
        dotFile << "  node [shape=circle, style=filled, fontname=\"Arial\"];\n";
        
        // Задаём фиксированные позиции для узлов на основе их координат
        // Масштабируем для лучшего отображения
        double scale = 0.5;
        for (int i = 0; i < g.n; ++i) {
            dotFile << "  " << i << " [pos=\"" 
                    << g.points[i].x * scale << "," 
                    << g.points[i].y * scale << "!\"";
            
            // Раскраска узлов
            if (isTree && i == g.startNode) {
                dotFile << ", fillcolor=\"red\", fontcolor=\"white\"";
            } else {
                // Градиент от светло-голубого до тёмно-синего в зависимости от степени
                int degree = g.adj[i].size();
                if (degree > 5) degree = 5;
                std::string color;
                switch(degree) {
                    case 0: color = "lightblue"; break;
                    case 1: color = "lightblue2"; break;
                    case 2: color = "lightblue3"; break;
                    case 3: color = "lightblue4"; break;
                    default: color = "skyblue"; break;
                }
                dotFile << ", fillcolor=\"" << color << "\"";
            }
            
            dotFile << "];\n";
        }
        
        // Добавляем рёбра
        std::set<std::pair<int, int>> edges;
        for (int i = 0; i < g.n; ++i) {
            for (int j : g.adj[i]) {
                if (i < j) {
                    edges.insert({i, j});
                }
            }
        }
        
        for (const auto& edge : edges) {
            dotFile << "  " << edge.first << " -- " << edge.second;
            
            // Стиль рёбер для дерева
            if (isTree) {
                dotFile << " [color=\"green\", penwidth=2.0]";
            }
            
            dotFile << ";\n";
        }
        
        dotFile << "}\n";
        dotFile.close();
        
        std::cout << "  Экспортирован: " << filename << ".dot" << std::endl;
    }
    
    void generatePNG(const std::string& filename) const {
        std::string command = "dot -Tpng " + outputDir + "/" + filename + ".dot -o " + 
                              outputDir + "/" + filename + ".png";
        int result = system(command.c_str());
        
        if (result == 0) {
            std::cout << "  Сгенерирован: " << filename << ".png" << std::endl;
        } else {
            std::cout << "  Ошибка генерации PNG. Убедитесь, что Graphviz установлен." << std::endl;
        }
    }
    
    void generateSVG(const std::string& filename) const {
        std::string command = "dot -Tsvg " + outputDir + "/" + filename + ".dot -o " + 
                              outputDir + "/" + filename + ".svg";
        int result = system(command.c_str());
        
        if (result == 0) {
            std::cout << "  Сгенерирован: " << filename << ".svg" << std::endl;
        }
    }
    
    void generateHTMLReport(const std::vector<Graph>& graphs, 
                            const std::vector<Graph>& trees,
                            const std::vector<std::string>& names,
                            const std::vector<int>& diameters,
                            const std::vector<int>& maxDepths) const {
        
        std::ofstream htmlFile(outputDir + "/report.html");
        
        htmlFile << "<!DOCTYPE html>\n";
        htmlFile << "<html>\n";
        htmlFile << "<head>\n";
        htmlFile << "  <title>Отчёт по генерации графов</title>\n";
        htmlFile << "  <style>\n";
        htmlFile << "    body { font-family: Arial, sans-serif; margin: 20px; }\n";
        htmlFile << "    .graph-container { display: flex; flex-wrap: wrap; gap: 20px; }\n";
        htmlFile << "    .graph-card { border: 1px solid #ccc; border-radius: 5px; padding: 10px; width: 600px; }\n";
        htmlFile << "    .graph-title { font-size: 18px; font-weight: bold; margin-bottom: 10px; }\n";
        htmlFile << "    .graph-stats { margin: 10px 0; font-size: 14px; }\n";
        htmlFile << "    .graph-image { width: 100%; height: auto; }\n";
        htmlFile << "    .tree-image { width: 100%; height: auto; margin-top: 10px; }\n";
        htmlFile << "    table { border-collapse: collapse; width: 100%; margin: 20px 0; }\n";
        htmlFile << "    th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n";
        htmlFile << "    th { background-color: #f2f2f2; }\n";
        htmlFile << "  </style>\n";
        htmlFile << "</head>\n";
        htmlFile << "<body>\n";
        
        htmlFile << "<h1>Отчёт по генерации графов</h1>\n";
        
        // Таблица со статистикой
        htmlFile << "<h2>Статистика графов</h2>\n";
        htmlFile << "<table>\n";
        htmlFile << "  <tr>\n";
        htmlFile << "    <th>№</th>\n";
        htmlFile << "    <th>Имя</th>\n";
        htmlFile << "    <th>Вершин</th>\n";
        htmlFile << "    <th>Рёбер</th>\n";
        htmlFile << "    <th>Макс. степень</th>\n";
        htmlFile << "    <th>Ср. степень</th>\n";
        htmlFile << "    <th>Диаметр</th>\n";
        htmlFile << "    <th>Макс. глубина дерева</th>\n";
        htmlFile << "  </tr>\n";
        
        for (size_t i = 0; i < graphs.size(); ++i) {
            auto [maxDeg, avgDeg, edgeCount] = graphs[i].getStats();
            
            htmlFile << "  <tr>\n";
            htmlFile << "    <td>" << (i+1) << "</td>\n";
            htmlFile << "    <td>" << names[i] << "</td>\n";
            htmlFile << "    <td>" << graphs[i].n << "</td>\n";
            htmlFile << "    <td>" << edgeCount << "</td>\n";
            htmlFile << "    <td>" << maxDeg << "</td>\n";
            htmlFile << "    <td>" << std::fixed << std::setprecision(2) << avgDeg << "</td>\n";
            htmlFile << "    <td>" << diameters[i] << "</td>\n";
            htmlFile << "    <td>" << maxDepths[i] << "</td>\n";
            htmlFile << "  </tr>\n";
        }
        
        htmlFile << "</table>\n";
        
        // Визуализация графов
        htmlFile << "<h2>Визуализация графов и деревьев</h2>\n";
        htmlFile << "<div class='graph-container'>\n";
        
        for (size_t i = 0; i < graphs.size(); ++i) {
            std::string safeName = sanitizeName(names[i]);
            
            htmlFile << "  <div class='graph-card'>\n";
            htmlFile << "    <div class='graph-title'>" << (i+1) << ". " << names[i] << "</div>\n";
            
            // Статистика
            auto [maxDeg, avgDeg, edgeCount] = graphs[i].getStats();
            htmlFile << "    <div class='graph-stats'>\n";
            htmlFile << "      Вершин: " << graphs[i].n << " | ";
            htmlFile << "      Рёбер: " << edgeCount << " | ";
            htmlFile << "      Диаметр: " << diameters[i] << " | ";
            htmlFile << "      Глубина дерева: " << maxDepths[i] << "\n";
            htmlFile << "    </div>\n";
            
            // Граф
            htmlFile << "    <div>\n";
            htmlFile << "      <strong>Граф:</strong><br>\n";
            htmlFile << "      <img class='graph-image' src='" << safeName << "_graph.png' alt='" << names[i] << " graph'>\n";
            htmlFile << "    </div>\n";
            
            // Дерево
            htmlFile << "    <div>\n";
            htmlFile << "      <strong>Дерево (начальная вершина: " << trees[i].startNode << "):</strong><br>\n";
            htmlFile << "      <img class='tree-image' src='" << safeName << "_tree.png' alt='" << names[i] << " tree'>\n";
            htmlFile << "    </div>\n";
            
            htmlFile << "  </div>\n";
        }
        
        htmlFile << "</div>\n";
        
        htmlFile << "</body>\n";
        htmlFile << "</html>\n";
        
        htmlFile.close();
        
        std::cout << "Сгенерирован HTML отчёт: " << outputDir << "/report.html" << std::endl;
    }
};

// Добавляем const везде, где это необходимо
void printGraphInfo(const Graph& g, const std::string& name, int diameter, int maxDepth) {
    std::cout << "\n=== " << name << " ===" << std::endl;
    std::cout << "Количество вершин: " << g.n << std::endl;
    std::cout << "Диаметр графа: " << diameter << std::endl;
    std::cout << "Допустимая глубина дерева (половина диаметра): " << maxDepth << std::endl;
    
    auto [maxDeg, avgDeg, edgeCount] = g.getStats();
    
    std::cout << "Максимальная степень вершины: " << maxDeg << std::endl;
    std::cout << "Средняя степень вершины: " << std::fixed << std::setprecision(2) << avgDeg << std::endl;
    std::cout << "Количество рёбер: " << edgeCount << std::endl;
}

int main() {
    GraphGenerator generator;
    
    std::cout << "Генерация 100 случайных точек на плоскости 100x100..." << std::endl;
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
        {"Exp_a0.1_b1.0", "exp", 0.1, 1.0, -1, -1},
        {"Exp_a0.01_b2.0_maxDeg10", "exp", 0.01, 2.0, 10, -1},
        {"Exp_a0.05_b1.5_maxDist50", "exp", 0.05, 1.5, -1, 50},
        {"Exp_a0.1_b0.5_maxDeg15_maxDist80", "exp", 0.1, 0.5, 15, 80},
        {"Exp_a0.001_b3.0_maxDist30", "exp", 0.001, 3.0, -1, 30},
        {"Inv_b1.0", "inv", 0, 1.0, -1, -1},
        {"Inv_b2.0_maxDeg8", "inv", 0, 2.0, 8, -1},
        {"Inv_b1.5_maxDist60", "inv", 0, 1.5, -1, 60},
        {"Inv_b0.5_maxDeg12_maxDist70", "inv", 0, 0.5, 12, 70},
        {"Inv_b2.5_maxDeg5_maxDist40", "inv", 0, 2.5, 5, 40}
    };
    
    std::vector<Graph> graphs;
    std::vector<Graph> trees;
    std::vector<std::string> graphNames;
    std::vector<int> diameters;
    std::vector<int> maxDepths;
    
    // Генерируем графы с разными параметрами
    std::cout << "\nГенерация 10 графов с разными параметрами..." << std::endl;
    
    for (size_t i = 0; i < params.size(); ++i) {
        const auto& p = params[i];
        std::cout << "\nГенерация графа " << (i+1) << ": " << p.name << std::endl;
        
        Graph g = generator.generateGraph(points, p.name, p.probType, p.a, p.b, 
                                          p.maxDegree, p.maxDistance);
        graphs.push_back(g);
        graphNames.push_back(p.name);
        
        int diameter = generator.computeDiameter(g);
        diameters.push_back(diameter);
        int maxDepth = diameter / 2;
        maxDepths.push_back(maxDepth);
        
        printGraphInfo(g, p.name, diameter, maxDepth);
        
        // Строим дерево с ограничением по глубине
        std::string treeName = p.name + "_tree";
        Graph tree = generator.buildTreeWithConstraints(g, maxDepth, treeName);
        trees.push_back(tree);
        
        int treeDiameter = generator.computeDiameter(tree);
        
        std::cout << "--- Дерево из графа ---" << std::endl;
        std::cout << "Диаметр дерева: " << treeDiameter << std::endl;
        std::cout << "Начальная вершина дерева: " << tree.startNode << std::endl;
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
    
    // Экспорт в Graphviz
    std::cout << "\n=== Экспорт в Graphviz ===" << std::endl;
    GraphVizExporter exporter("graphviz_output");
    
    for (size_t i = 0; i < graphs.size(); ++i) {
        std::string safeName = exporter.getSanitizedName(graphNames[i]);
        
        // Экспорт графа
        exporter.exportToDot(graphs[i], safeName + "_graph", false);
        exporter.generatePNG(safeName + "_graph");
        exporter.generateSVG(safeName + "_graph");
        
        // Экспорт дерева
        exporter.exportToDot(trees[i], safeName + "_tree", true);
        exporter.generatePNG(safeName + "_tree");
        exporter.generateSVG(safeName + "_tree");
    }
    
    // Генерация HTML отчёта
    exporter.generateHTMLReport(graphs, trees, graphNames, diameters, maxDepths);
    
    std::cout << "\nГотово! Все файлы сохранены в директории 'graphviz_output/'" << std::endl;
    std::cout << "Для просмотра отчёта откройте graphviz_output/report.html в браузере" << std::endl;
    std::cout << "\nТребования:" << std::endl;
    std::cout << "  - Установите Graphviz: sudo apt-get install graphviz (Ubuntu/Debian)" << std::endl;
    std::cout << "  - Или: brew install graphviz (macOS)" << std::endl;
    std::cout << "  - Или скачайте с https://graphviz.org/download/ (Windows)" << std::endl;
    
    return 0;
}