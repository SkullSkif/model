#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <queue>
#include <stack>
#include <limits>
#include <fstream>
#include <iomanip>
#include "matplotlibcpp.h"

namespace plt = matplotlibcpp;

struct Point {
    double x, y;
    Point(double x = 0, double y = 0) : x(x), y(y) {}
};

struct Edge {
    int from, to;
    double weight;
    double probability;
    Edge(int f, int t, double w, double p) : from(f), to(t), weight(w), probability(p) {}
};

class Graph {
private:
    std::vector<Point> points;
    std::vector<std::vector<Edge>> adjacencyList;
    int numPoints;
    
    // DFS для поиска циклов
    bool hasCycleUtil(int v, std::vector<bool>& visited, std::vector<bool>& recStack, std::vector<int>& parent) const {
        if (!visited[v]) {
            visited[v] = true;
            recStack[v] = true;
            
            for (const auto& edge : adjacencyList[v]) {
                if (!visited[edge.to]) {
                    parent[edge.to] = v;
                    if (hasCycleUtil(edge.to, visited, recStack, parent))
                        return true;
                } else if (recStack[edge.to] && edge.to != parent[v]) {
                    return true; // Найден цикл
                }
            }
        }
        recStack[v] = false;
        return false;
    }
    
public:
    Graph(int n) : numPoints(n) {
        points.reserve(n);
        adjacencyList.resize(n);
    }
    
    void addPoint(const Point& p) {
        points.push_back(p);
    }
    
    double distance(int i, int j) const {
        double dx = points[i].x - points[j].x;
        double dy = points[i].y - points[j].y;
        return std::sqrt(dx*dx + dy*dy);
    }
    
    void buildGraph(double a, double b, const std::string& probabilityType) {
        adjacencyList.clear();
        adjacencyList.resize(numPoints);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        for (int i = 0; i < numPoints; ++i) {
            for (int j = i + 1; j < numPoints; ++j) {
                double d = distance(i, j);
                double prob;
                
                if (probabilityType == "exp") {
                    prob = std::exp(-a * std::pow(d, b));
                } else { // inverse power
                    prob = 1.0 / (std::pow(d, b));
                }
                
                // Случайное решение о добавлении ребра
                if (dis(gen) < prob) {
                    adjacencyList[i].emplace_back(i, j, d, prob);
                    adjacencyList[j].emplace_back(j, i, d, prob);
                }
            }
        }
        
        // Удаляем циклы, чтобы получить дерево
        makeAcyclic();
    }
    
    void makeAcyclic() {
        std::vector<std::vector<Edge>> tree(numPoints);
        std::vector<bool> visited(numPoints, false);
        std::queue<int> q;
        
        // Начинаем с первой вершины
        if (numPoints > 0) {
            visited[0] = true;
            q.push(0);
            
            while (!q.empty()) {
                int current = q.front();
                q.pop();
                
                for (const auto& edge : adjacencyList[current]) {
                    if (!visited[edge.to]) {
                        visited[edge.to] = true;
                        tree[current].push_back(edge);
                        tree[edge.to].emplace_back(edge.to, current, edge.weight, edge.probability);
                        q.push(edge.to);
                    }
                }
            }
        }
        
        // Добавляем ребра для изолированных вершин
        for (int i = 0; i < numPoints; ++i) {
            if (!visited[i]) {
                // Находим ближайшую вершину
                double minDist = std::numeric_limits<double>::max();
                int nearest = -1;
                
                for (int j = 0; j < numPoints; ++j) {
                    if (i != j && visited[j]) {
                        double dist = distance(i, j);
                        if (dist < minDist) {
                            minDist = dist;
                            nearest = j;
                        }
                    }
                }
                
                if (nearest != -1) {
                    visited[i] = true;
                    tree[i].emplace_back(i, nearest, minDist, 1.0);
                    tree[nearest].emplace_back(nearest, i, minDist, 1.0);
                }
            }
        }
        
        adjacencyList = tree;
    }
    
    bool hasCycles() const {
        std::vector<bool> visited(numPoints, false);
        std::vector<bool> recStack(numPoints, false);
        std::vector<int> parent(numPoints, -1);
        
        for (int i = 0; i < numPoints; ++i) {
            if (hasCycleUtil(i, visited, recStack, parent)) {
                return true;
            }
        }
        return false;
    }
    
    void applyConstraints(int maxDegree = -1, double maxDistance = -1) {
        if (maxDegree <= 0 && maxDistance <= 0) return;
        
        std::vector<std::vector<Edge>> newAdjacencyList(numPoints);
        
        for (int i = 0; i < numPoints; ++i) {
            for (const auto& edge : adjacencyList[i]) {
                if (edge.from < edge.to) { // Обрабатываем каждое ребро один раз
                    bool keepEdge = true;
                    
                    // Проверка на максимальное расстояние
                    if (maxDistance > 0 && edge.weight > maxDistance) {
                        keepEdge = false;
                    }
                    
                    // Проверка на максимальную степень
                    if (maxDegree > 0) {
                        int degree1 = adjacencyList[edge.from].size();
                        int degree2 = adjacencyList[edge.to].size();
                        
                        if (degree1 > maxDegree || degree2 > maxDegree) {
                            keepEdge = false;
                        }
                    }
                    
                    if (keepEdge) {
                        newAdjacencyList[edge.from].push_back(edge);
                        newAdjacencyList[edge.to].emplace_back(edge.to, edge.from, edge.weight, edge.probability);
                    }
                }
            }
        }
        
        adjacencyList = newAdjacencyList;
    }
    
    std::vector<int> bfs(int start) const {
        std::vector<int> dist(numPoints, -1);
        std::queue<int> q;
        
        dist[start] = 0;
        q.push(start);
        
        while (!q.empty()) {
            int current = q.front();
            q.pop();
            
            for (const auto& edge : adjacencyList[current]) {
                if (dist[edge.to] == -1) {
                    dist[edge.to] = dist[current] + 1;
                    q.push(edge.to);
                }
            }
        }
        
        return dist;
    }
    
    double computeDiameter() const {
        double maxDist = 0;
        
        for (int i = 0; i < numPoints; ++i) {
            auto dist = bfs(i);
            for (int j = 0; j < numPoints; ++j) {
                if (dist[j] > maxDist) {
                    maxDist = dist[j];
                }
            }
        }
        
        return maxDist;
    }
    
    std::pair<double, double> computeTreeProperties() const {
        double diameter = computeDiameter();
        double allowedDepth = diameter / 2.0;
        
        return {diameter, allowedDepth};
    }
    
    void visualize(const std::string& title, const std::string& filename = "", 
                   double a = 0, double b = 0, const std::string& probType = "") const {
        plt::figure_size(1200, 1000);
        
        std::string fullTitle = title;
        if (probType != "") {
            fullTitle += "\na=" + std::to_string(a).substr(0, 4) + 
                        ", b=" + std::to_string(b).substr(0, 4) + 
                        ", тип=" + probType;
        }
        plt::title(fullTitle);
        
        // Рисуем вершины
        std::vector<double> x_coords, y_coords;
        for (const auto& p : points) {
            x_coords.push_back(p.x);
            y_coords.push_back(p.y);
        }
        
        // Рисуем ребра
        for (int i = 0; i < numPoints; ++i) {
            for (const auto& edge : adjacencyList[i]) {
                if (edge.from < edge.to) {
                    std::vector<double> x = {points[edge.from].x, points[edge.to].x};
                    std::vector<double> y = {points[edge.from].y, points[edge.to].y};
                    
                    // Толщина линии пропорциональна вероятности
                    float lineWidth = 1.0f + static_cast<float>(edge.probability * 3);
                    plt::plot(x, y, {{"color", "blue"}, {"linewidth", std::to_string(lineWidth)}});
                }
            }
        }
        
        plt::scatter(x_coords, y_coords, 50);
        plt::xlim(0, 100);
        plt::ylim(0, 100);
        
        if (!filename.empty()) {
            plt::save(filename);
            std::cout << "Граф сохранен в файл: " << filename << std::endl;
        } else {
            std::string safe_filename = title;
            std::replace(safe_filename.begin(), safe_filename.end(), ' ', '_');
            safe_filename += ".png";
            plt::save(safe_filename);
            std::cout << "Граф сохранен в файл: " << safe_filename << std::endl;
        }
        
        plt::close();
    }
    
    void printStats() const {
        auto [diameter, allowedDepth] = computeTreeProperties();
        
        std::cout << "Диаметр графа: " << diameter << std::endl;
        std::cout << "Допустимая глубина дерева: " << allowedDepth << std::endl;
        std::cout << "Наличие циклов: " << (hasCycles() ? "ДА" : "НЕТ") << std::endl;
        
        // Статистика по степеням вершин
        std::vector<int> degrees(numPoints);
        int totalEdges = 0;
        
        for (int i = 0; i < numPoints; ++i) {
            degrees[i] = adjacencyList[i].size();
            totalEdges += degrees[i];
        }
        
        totalEdges /= 2; // Каждое ребро посчитано дважды
        
        std::cout << "Количество ребер: " << totalEdges << std::endl;
        std::cout << "Средняя степень: " << static_cast<double>(totalEdges * 2) / numPoints << std::endl;
        std::cout << "Максимальная степень: " << *std::max_element(degrees.begin(), degrees.end()) << std::endl;
        std::cout << "Минимальная степень: " << *std::min_element(degrees.begin(), degrees.end()) << std::endl;
    }
};

class GraphGenerator {
private:
    std::vector<Point> generateRandomPoints(int numPoints) {
        std::vector<Point> points;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 100.0);
        
        for (int i = 0; i < numPoints; ++i) {
            points.emplace_back(dis(gen), dis(gen));
        }
        
        return points;
    }
    
public:
    std::vector<Graph> generatePattern(const std::string& patternName, 
                                        const std::vector<double>& aValues,
                                        const std::vector<double>& bValues,
                                        const std::string& probType,
                                        int numPoints = 50,
                                        int graphsPerPattern = 3) {
        std::vector<Graph> graphs;
        
        for (int i = 0; i < graphsPerPattern; ++i) {
            // Генерируем случайные точки
            auto points = generateRandomPoints(numPoints);
            
            // Создаем граф с этими точками
            Graph g(numPoints);
            for (const auto& p : points) {
                g.addPoint(p);
            }
            
            // Выбираем случайные параметры из заданного диапазона
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> aDis(aValues[0], aValues[1]);
            std::uniform_real_distribution<> bDis(bValues[0], bValues[1]);
            
            double a = aDis(gen);
            double b = bDis(gen);
            
            std::cout << "\n=== Паттерн: " << patternName << ", Граф " << i+1 << " ===" << std::endl;
            std::cout << "Параметры: a = " << a << ", b = " << b << std::endl;
            
            g.buildGraph(a, b, probType);
            
            // Применяем ограничения
            g.applyConstraints(10, 70);
            
            g.printStats();
            graphs.push_back(g);
        }
        
        return graphs;
    }
    
    void visualizeAllPatterns(const std::vector<std::vector<Graph>>& allGraphs,
                              const std::vector<std::vector<double>>& allParams,
                              const std::vector<std::string>& probTypes) {
        std::vector<std::string> patternNames = {
            "Exp_LowDensity",
            "Exp_HighDensity",
            "Inv_LowDensity",
            "Inv_HighDensity",
            "Exp_SmallA_LargeB",
            "Exp_LargeA_SmallB",
            "Inv_SmallB",
            "Inv_LargeB"
        };
        
        // Создаем папку для графиков
        system("mkdir -p graphs");
        
        for (size_t p = 0; p < allGraphs.size(); ++p) {
            std::cout << "\n=== Визуализация паттерна: " << patternNames[p] << " ===" << std::endl;
            for (size_t g = 0; g < allGraphs[p].size(); ++g) {
                std::string filename = "graphs/" + patternNames[p] + "_graph_" + std::to_string(g+1) + ".png";
                std::string title = patternNames[p] + " - Graph " + std::to_string(g+1);
                
                double a = allParams[p][g*2];
                double b = allParams[p][g*2 + 1];
                
                allGraphs[p][g].visualize(title, filename, a, b, probTypes[p]);
            }
        }
        
        std::cout << "\nВсе графики сохранены в папке 'graphs/'" << std::endl;
    }
};

int main() {
    GraphGenerator generator;
    std::vector<std::vector<Graph>> allGraphs;
    std::vector<std::vector<double>> allParams;
    std::vector<std::string> probTypes;
    
    // Паттерн 1: Экспоненциальная вероятность, низкая плотность (маленькие a и b)
    auto pattern1 = generator.generatePattern(
        "Pattern1_Exp_LowDensity",
        {0.01, 0.05}, {1.0, 1.5},
        "exp", 50, 3
    );
    allGraphs.push_back(pattern1);
    allParams.push_back({0.03, 1.2, 0.02, 1.3, 0.04, 1.1});
    probTypes.push_back("exp");
    
    // Паттерн 2: Экспоненциальная вероятность, высокая плотность (большие a и b)
    auto pattern2 = generator.generatePattern(
        "Pattern2_Exp_HighDensity",
        {0.5, 1.0}, {2.5, 3.0},
        "exp", 50, 3
    );
    allGraphs.push_back(pattern2);
    allParams.push_back({0.7, 2.8, 0.8, 2.6, 0.6, 2.9});
    probTypes.push_back("exp");
    
    // Паттерн 3: Обратная степенная, низкая плотность (большой b)
    auto pattern3 = generator.generatePattern(
        "Pattern3_Inv_LowDensity",
        {0.0, 0.0}, {2.5, 3.0},
        "inv", 50, 3
    );
    allGraphs.push_back(pattern3);
    allParams.push_back({0.0, 2.8, 0.0, 2.6, 0.0, 2.9});
    probTypes.push_back("inv");
    
    // Паттерн 4: Обратная степенная, высокая плотность (маленький b)
    auto pattern4 = generator.generatePattern(
        "Pattern4_Inv_HighDensity",
        {0.0, 0.0}, {1.0, 1.5},
        "inv", 50, 3
    );
    allGraphs.push_back(pattern4);
    allParams.push_back({0.0, 1.2, 0.0, 1.3, 0.0, 1.1});
    probTypes.push_back("inv");
    
    // Паттерн 5: Экспоненциальная, маленькая a, большая b (дальние связи с сильным затуханием)
    auto pattern5 = generator.generatePattern(
        "Pattern5_Exp_SmallA_LargeB",
        {0.01, 0.05}, {3.0, 4.0},
        "exp", 50, 3
    );
    allGraphs.push_back(pattern5);
    allParams.push_back({0.03, 3.5, 0.02, 3.7, 0.04, 3.2});
    probTypes.push_back("exp");
    
    // Паттерн 6: Экспоненциальная, большая a, маленькая b (локальные связи, слабое затухание)
    auto pattern6 = generator.generatePattern(
        "Pattern6_Exp_LargeA_SmallB",
        {0.5, 1.0}, {1.0, 1.5},
        "exp", 50, 3
    );
    allGraphs.push_back(pattern6);
    allParams.push_back({0.7, 1.2, 0.8, 1.3, 0.6, 1.1});
    probTypes.push_back("exp");
    
    // Паттерн 7: Степенная, маленькая b (слабая зависимость от расстояния - более равномерные связи)
    auto pattern7 = generator.generatePattern(
        "Pattern7_Inv_SmallB",
        {0.0, 0.0}, {1.0, 1.5},
        "inv", 50, 3
    );
    allGraphs.push_back(pattern7);
    allParams.push_back({0.0, 1.2, 0.0, 1.3, 0.0, 1.1});
    probTypes.push_back("inv");
    
    // Паттерн 8: Степенная, большая b (сильная зависимость от расстояния - только ближайшие связи)
    auto pattern8 = generator.generatePattern(
        "Pattern8_Inv_LargeB",
        {0.0, 0.0}, {3.0, 4.0},
        "inv", 50, 3
    );
    allGraphs.push_back(pattern8);
    allParams.push_back({0.0, 3.5, 0.0, 3.7, 0.0, 3.2});
    probTypes.push_back("inv");
    
    std::cout << "\n=== СТАТИСТИКА ПО ВСЕМ ПАТТЕРНАМ ===" << std::endl;
    for (size_t p = 0; p < allGraphs.size(); ++p) {
        std::cout << "\nПаттерн " << p+1 << ":" << std::endl;
        for (size_t g = 0; g < allGraphs[p].size(); ++g) {
            std::cout << "  Граф " << g+1 << " - ";
            allGraphs[p][g].printStats();
        }
    }
    
    // Визуализация
    std::cout << "\nНачинаем визуализацию графов..." << std::endl;
    generator.visualizeAllPatterns(allGraphs, allParams, probTypes);
    
    return 0;
}