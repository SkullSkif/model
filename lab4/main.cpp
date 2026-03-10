#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <iomanip>

using namespace std;

// Функция для вывода матрицы
void printMatrix(const vector<vector<int>>& matrix, const string& title) {
    cout << "\n" << title << ":\n";
    int m = matrix.size();
    int n = matrix[0].size();
    
    // Вывод заголовков столбцов
    cout << "    ";
    for (int j = 0; j < n; j++) {
        cout << "n" << j+1 << " ";
    }
    cout << "\n";
    
    // Вывод строк матрицы
    for (int i = 0; i < m; i++) {
        cout << "m" << i+1 << " ";
        for (int j = 0; j < n; j++) {
            cout << setw(2) << matrix[i][j] << " ";
        }
        cout << "\n";
    }
}

// Функция для генерации случайного числа в диапазоне [min, max]
int getRandomInt(int min, int max) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> dist(min, max);
    return dist(gen);
}

// Генерация случайного массива n
vector<int> generateNArray(int size) {
    vector<int> arr(size);
    for (int i = 0; i < size; i++) {
        arr[i] = getRandomInt(1, 9); // числа от 1 до 9 для наглядности
    }
    return arr;
}

// Вывод массива
void printArray(const vector<int>& arr, const string& name) {
    cout << name << " = {";
    for (size_t i = 0; i < arr.size(); i++) {
        cout << arr[i];
        if (i < arr.size() - 1) cout << ", ";
    }
    cout << "}\n";
}

// Выборка с возвращением (повторения разрешены)
vector<vector<int>> samplingWithReplacement(const vector<int>& n, int m) {
    int n_size = n.size();
    vector<vector<int>> matrix(m, vector<int>(n_size, 0));
    
    for (int i = 0; i < m; i++) {
        int index = getRandomInt(0, n_size - 1);
        matrix[i][index] = 1;
    }
    
    return matrix;
}

// Выборка без возвращения (повторения запрещены)
vector<vector<int>> samplingWithoutReplacement(const vector<int>& n, int m) {
    int n_size = n.size();
    vector<vector<int>> matrix(m, vector<int>(n_size, 0));
    
    vector<int> available_indices(n_size);
    for (int i = 0; i < n_size; i++) {
        available_indices[i] = i;
    }
    
    for (int i = 0; i < m; i++) {
        if (available_indices.empty()) break;
        
        int random_pos = getRandomInt(0, available_indices.size() - 1);
        int index = available_indices[random_pos];
        matrix[i][index] = 1;
        
        // Удаляем использованный индекс
        available_indices.erase(available_indices.begin() + random_pos);
    }
    
    return matrix;
}

// Размещение (m == n, без возвращения)
vector<vector<int>> placement(const vector<int>& n) {
    int size = n.size();
    vector<vector<int>> matrix(size, vector<int>(size, 0));
    
    vector<int> indices(size);
    for (int i = 0; i < size; i++) {
        indices[i] = i;
    }
    
    random_shuffle(indices.begin(), indices.end());
    
    for (int i = 0; i < size; i++) {
        matrix[i][indices[i]] = 1;
    }
    
    return matrix;
}

// Выборка с несколькими прогонами (накопление результатов)
vector<vector<int>> multipleRuns(const vector<int>& n, int m, int runs) {
    int n_size = n.size();
    vector<vector<int>> result_matrix(m, vector<int>(n_size, 0));
    
    for (int run = 0; run < runs; run++) {
        auto matrix = samplingWithReplacement(n, m);
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n_size; j++) {
                result_matrix[i][j] += matrix[i][j];
            }
        }
    }
    
    return result_matrix;
}

int main() {
    // Устанавливаем локаль для корректного вывода
    setlocale(LC_ALL, "Russian");
    
    cout << "Лабораторная работа: Выборка с возвращением и без\n";
    cout << "================================================\n\n";
    
    // Генерируем n и m
    int n_size, m;
    cout << "Введите размер массива n: ";
    cin >> n_size;
    cout << "Введите размер выборки m (m <= n): ";
    cin >> m;
    
    if (m > n_size) {
        cout << "Ошибка: m должно быть <= n. Устанавливаю m = n.\n";
        m = n_size;
    }
    
    // Генерируем массив n
    vector<int> n_array = generateNArray(n_size);
    cout << "\nСгенерированный массив:\n";
    printArray(n_array, "n");
    cout << "m = " << m << "\n\n";
    
    cout << "1. ТРИ ПРИМЕРА ВЫБОРКИ С ВОЗВРАЩЕНИЕМ:\n";
    cout << "----------------------------------------\n";
    for (int i = 1; i <= 3; i++) {
        auto matrix = samplingWithReplacement(n_array, m);
        printMatrix(matrix, "Пример " + to_string(i));
    }
    
    cout << "\n2. ТРИ ПРИМЕРА ВЫБОРКИ БЕЗ ВОЗВРАЩЕНИЯ:\n";
    cout << "-------------------------------------------\n";
    for (int i = 1; i <= 3; i++) {
        auto matrix = samplingWithoutReplacement(n_array, m);
        printMatrix(matrix, "Пример " + to_string(i));
    }
    
    cout << "\n3. ТРИ ПРИМЕРА РАЗМЕЩЕНИЯ (m = n, без возвращения):\n";
    cout << "------------------------------------------------------\n";
    for (int i = 1; i <= 3; i++) {
        auto matrix = placement(n_array);
        printMatrix(matrix, "Пример " + to_string(i));
    }
    
    cout << "\n4. ТРИ ПРИМЕРА С НЕСКОЛЬКИМИ ПРОГОНАМИ:\n";
    cout << "------------------------------------------\n";
    cout << "(Значения в матрице показывают количество попаданий на позицию)\n";
    
    int runs;
    cout << "Введите количество прогонов: ";
    cin >> runs;
    
    for (int i = 1; i <= 3; i++) {
        auto matrix = multipleRuns(n_array, m, runs);
        printMatrix(matrix, "Пример " + to_string(i) + " (" + to_string(runs) + " прогонов)");
    }
    
    cout << "\nПояснения:\n";
    cout << "1. Выборка с возвращением: элементы в выборке m могут повторяться\n";
    cout << "2. Выборка без возвращения: все элементы в выборке m уникальны\n";
    cout << "3. Размещение: частный случай выборки без возвращения при m = n\n";
    cout << "4. Многократные прогоны: суммирование результатов нескольких выборок\n";
    
    return 0;
}