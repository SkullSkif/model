#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <queue>
#include <limits>
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
                    prob = 1.0 / std::pow(d, b);
                }
                
                // Случайное решение о добавлении ребра
                if (dis(gen) < prob) {
                    adjacencyList[i].emplace_back(i, j, d, prob);
                    adjacencyList[j].emplace_back(j, i, d, prob);
                }
            }
        }
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
    
    void visualize(const std::string& title, const std::string& filename = "") const {
    plt::figure_size(1000, 1000);
    plt::title(title);
    
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
                plt::plot(x, y, "b-");
            }
        }
    }
    
    plt::scatter(x_coords, y_coords, 50);
    plt::xlim(0, 100);
    plt::ylim(0, 100);
    
    // Сохраняем в файл вместо показа
    if (!filename.empty()) {
        plt::save(filename);
        std::cout << "Граф сохранен в файл: " << filename << std::endl;
    } else {
        // Создаем имя файла из заголовка
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
                                        int numPoints = 100,
                                        int graphsPerPattern = 5) {
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
            
            // Применяем ограничения в зависимости от паттерна
            if (patternName == "Pattern1_Exp_LowDensity") {
                g.applyConstraints(5, 30); // ограничение степени и расстояния
            } else if (patternName == "Pattern2_Exp_HighDensity") {
                g.applyConstraints(15, 50);
            } else if (patternName == "Pattern3_Inverse_LowDensity") {
                g.applyConstraints(5, 30);
            } else if (patternName == "Pattern4_Inverse_HighDensity") {
                g.applyConstraints(15, 50);
            }
            
            g.printStats();
            graphs.push_back(g);
        }
        
        return graphs;
    }
    
void visualizeAllPatterns(const std::vector<std::vector<Graph>>& allGraphs) {
    std::vector<std::string> patternNames = {
        "Exp_LowDensity",
        "Exp_HighDensity",
        "Inv_LowDensity",
        "Inv_HighDensity"
    };
    
    // Создаем папку для графиков
    system("mkdir -p graphs");
    
    for (size_t p = 0; p < allGraphs.size(); ++p) {
        std::cout << "\n=== Визуализация паттерна: " << patternNames[p] << " ===" << std::endl;
        for (size_t g = 0; g < allGraphs[p].size(); ++g) {
            std::string filename = "graphs/" + patternNames[p] + "_graph_" + std::to_string(g+1) + ".png";
            std::string title = patternNames[p] + " - Graph " + std::to_string(g+1);
            allGraphs[p][g].visualize(title, filename);
        }
    }
    
    std::cout << "\nВсе графики сохранены в папке 'graphs/'" << std::endl;
}
};

int main() {
    GraphGenerator generator;
    std::vector<std::vector<Graph>> allGraphs;
    
    // Паттерн 1: Экспоненциальная вероятность, низкая плотность (маленькие a и b)
    auto pattern1 = generator.generatePattern(
        "Pattern1_Exp_LowDensity",
        {0.01, 0.05}, {1.0, 2.0},
        "exp", 100, 5
    );
    allGraphs.push_back(pattern1);
    
    // Паттерн 2: Экспоненциальная вероятность, высокая плотность (большие a и b)
    auto pattern2 = generator.generatePattern(
        "Pattern2_Exp_HighDensity",
        {0.1, 0.5}, {2.0, 3.0},
        "exp", 100, 5
    );
    allGraphs.push_back(pattern2);
    
    // Паттерн 3: Обратная степенная, низкая плотность
    auto pattern3 = generator.generatePattern(
        "Pattern3_Inverse_LowDensity",
        {0.01, 0.05}, {2.0, 3.0},
        "inv", 100, 5
    );
    allGraphs.push_back(pattern3);
    
    // Паттерн 4: Обратная степенная, высокая плотность
    auto pattern4 = generator.generatePattern(
        "Pattern4_Inverse_HighDensity",
        {0.1, 0.5}, {1.0, 2.0},
        "inv", 100, 5
    );
    allGraphs.push_back(pattern4);
    
    std::cout << "\n=== Статистика по всем паттернам ===" << std::endl;
    for (size_t p = 0; p < allGraphs.size(); ++p) {
        std::cout << "\nПаттерн " << p+1 << ":" << std::endl;
        for (size_t g = 0; g < allGraphs[p].size(); ++g) {
            std::cout << "  Граф " << g+1 << " - ";
            allGraphs[p][g].printStats();
        }
    }

    
    
    // Визуализация
    std::cout << "\nНачинаем визуализацию графов..." << std::endl;
    generator.visualizeAllPatterns(allGraphs);
    
    return 0;
}