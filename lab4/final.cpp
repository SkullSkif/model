#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <iomanip>

using namespace std;

void printMatrix(const vector<vector<int>>& matrix, const string& title) {
    cout << "\n" << title << ":\n";
    int m = matrix.size();
    int n = matrix[0].size();
    
    cout << "    ";
    for (int j = 0; j < n; j++) {
        cout << "n" << j+1 << " ";
    }
    cout << "\n";
    
    for (int i = 0; i < m; i++) {
        cout << "m" << i+1 << " ";
        for (int j = 0; j < n; j++) {
            cout << setw(2) << matrix[i][j] << " ";
        }
        cout << "\n";
    }
}


int getRandomInt(int min, int max) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> dist(min, max);
    return dist(gen);
}


vector<int> generateNArray(int size) {
    vector<int> arr(size);
    for (int i = 0; i < size; i++) {
        arr[i] = getRandomInt(1, 9); 
    }
    return arr;
}


void printArray(const vector<int>& arr, const string& name) {
    cout << name << " = {";
    for (size_t i = 0; i < arr.size(); i++) {
        cout << arr[i];
        if (i < arr.size() - 1) cout << ", ";
    }
    cout << "}\n";
}


vector<vector<int>> matrixWithReturn(const vector<int>& n, int m) {
    int n_size = n.size();
    vector<vector<int>> matrix(m, vector<int>(n_size, 0));
    
    for (int i = 0; i < m; i++) {
        int index = getRandomInt(0, n_size - 1);
        matrix[i][index] = 1;
    }
    
    return matrix;
}


vector<vector<int>> matrixWithoutReturn(const vector<int>& n, int m) {
    int n_size = n.size();
    vector<vector<int>> matrix(m, vector<int>(n_size, 0));
    
    vector<int> available_indexes(n_size);
    for (int i = 0; i < n_size; i++) {
        available_indexes[i] = i;
    }
    
    for (int i = 0; i < m; i++) {
        if (available_indexes.empty()) break;
        
        int random_pos = getRandomInt(0, available_indexes.size() - 1);
        int index = available_indexes[random_pos];
        matrix[i][index] = 1;
        
        
        available_indexes.erase(available_indexes.begin() + random_pos);
    }
    
    return matrix;
}


vector<vector<int>> placement(const vector<int>& n) {
    return matrixWithoutReturn(n, n.size());
}


vector<vector<int>> multipleRuns(const vector<int>& n, int m, int runs) {
    int n_size = n.size();
    vector<vector<int>> result_matrix(m, vector<int>(n_size, 0));
    
    for (int run = 0; run < runs; run++) {
        auto matrix = matrixWithReturn(n, m);
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n_size; j++) {
                result_matrix[i][j] += matrix[i][j];
            }
        }
    }
    
    return result_matrix;
}

int main() {    
    int n_size, m;
    cout << "Введите размер массива n: ";
    cin >> n_size;
    cout << "Введите размер выборки m (m <= n): ";
    cin >> m;
    
    if (m > n_size) {
        cout << "Ошибка: m должно быть <= n. Устанавливаю m = n.\n";
        m = n_size;
    }
    
    
    vector<int> n_array = generateNArray(n_size);
    cout << "\nСгенерированный массив:\n";
    printArray(n_array, "n");
    cout << "m = " << m << "\n\n";
    
    cout << "1. ВЫБОРКА С ВОЗВРАЩЕНИЕМ:\n";
    for (int i = 1; i <= 3; i++) {
        auto matrix = matrixWithReturn(n_array, m);
        printMatrix(matrix, "Пример " + to_string(i));
    }
    
    cout << "\n2. ВЫБОРКА БЕЗ ВОЗВРАЩЕНИЯ:\n";
    for (int i = 1; i <= 3; i++) {
        auto matrix = matrixWithoutReturn(n_array, m);
        printMatrix(matrix, "Пример " + to_string(i));
    }
    
    cout << "\n3. РАЗМЕЩЕНИЕ (m = n, без возвращения):\n";
    for (int i = 1; i <= 3; i++) {
        auto matrix = placement(n_array);
        printMatrix(matrix, "Пример " + to_string(i));
    }
    

    int runs;
    cout << "Введите количество прогонов: ";
    cin >> runs;

    cout << "\n4. С НЕСКОЛЬКИМИ ПРОГОНАМИ:\n";
    for (int i = 1; i <= 3; i++) {
        auto matrix = multipleRuns(n_array, m, runs);
        printMatrix(matrix, "Пример " + to_string(i) + " (" + to_string(runs) + " прогонов)");
    }
    return 0;
}