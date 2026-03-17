#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <iomanip>
#include <cmath>

using namespace std;

// Функция для генерации случайной дважды стохастической матрицы
vector<vector<double>> generateDoublyStochasticMatrix(int n) {
    vector<vector<double>> matrix(n, vector<double>(n));
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, 1.0);
    
    // Алгоритм Sinkhorn-Knopp для нормализации до дважды стохастической
    // Сначала заполняем случайными числами
    for (int i = 0; i < n; i++) {
        double row_sum = 0;
        for (int j = 0; j < n; j++) {
            matrix[i][j] = dis(gen);
            row_sum += matrix[i][j];
        }
        // Нормируем строки
        for (int j = 0; j < n; j++) {
            matrix[i][j] /= row_sum;
        }
    }
    
    // Итеративно нормализуем строки и столбцы
    const int max_iter = 1000;
    const double tolerance = 1e-10;
    
    for (int iter = 0; iter < max_iter; iter++) {
        // Нормализация столбцов
        vector<double> col_sum(n, 0);
        for (int j = 0; j < n; j++) {
            for (int i = 0; i < n; i++) {
                col_sum[j] += matrix[i][j];
            }
            for (int i = 0; i < n; i++) {
                matrix[i][j] /= col_sum[j];
            }
        }
        
        // Нормализация строк
        vector<double> row_sum(n, 0);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                row_sum[i] += matrix[i][j];
            }
            for (int j = 0; j < n; j++) {
                matrix[i][j] /= row_sum[i];
            }
        }
        
        // Проверка сходимости
        double max_deviation = 0;
        for (int j = 0; j < n; j++) {
            double col_sum_check = 0;
            for (int i = 0; i < n; i++) {
                col_sum_check += matrix[i][j];
            }
            max_deviation = max(max_deviation, abs(col_sum_check - 1.0));
        }
        
        if (max_deviation < tolerance) {
            break;
        }
    }
    
    return matrix;
}

// Функция проверки дважды стохастичности
bool checkDoublyStochastic(const vector<vector<double>>& matrix, double tolerance = 1e-10) {
    int n = matrix.size();
    
    // Проверка строк
    for (int i = 0; i < n; i++) {
        double row_sum = 0;
        for (int j = 0; j < n; j++) {
            if (matrix[i][j] < 0 || matrix[i][j] > 1) return false;
            row_sum += matrix[i][j];
        }
        if (abs(row_sum - 1.0) > tolerance) return false;
    }
    
    // Проверка столбцов
    for (int j = 0; j < n; j++) {
        double col_sum = 0;
        for (int i = 0; i < n; i++) {
            col_sum += matrix[i][j];
        }
        if (abs(col_sum - 1.0) > tolerance) return false;
    }
    
    return true;
}

// Функция для выбора следующего состояния на основе текущего и матрицы переходов
int nextState(int current_state, const vector<vector<double>>& matrix, mt19937& gen) {
    int n = matrix.size();
    uniform_real_distribution<> dis(0.0, 1.0);
    double r = dis(gen);
    
    double cumulative = 0;
    for (int j = 0; j < n; j++) {
        cumulative += matrix[current_state][j];
        if (r < cumulative) {
            return j;
        }
    }
    return n - 1; // На случай ошибок округления
}

// Функция для сохранения графика перехода состояний
void saveTransitionData(const vector<int>& states, const string& filename) {
    ofstream file(filename);
    for (size_t i = 0; i < states.size(); i++) {
        file << i << " " << states[i] << endl;
    }
    file.close();
}

// Функция для вычисления автокорреляции
vector<double> computeAutocorrelation(const vector<int>& states, int max_lag) {
    int n = states.size();
    vector<double> autocorr(max_lag + 1, 0);
    
    // Вычисляем среднее
    double mean = 0;
    for (int state : states) {
        mean += state;
    }
    mean /= n;
    
    // Вычисляем дисперсию
    double variance = 0;
    for (int state : states) {
        variance += (state - mean) * (state - mean);
    }
    variance /= n;
    
    // Вычисляем автокорреляцию для каждого lag
    for (int lag = 0; lag <= max_lag; lag++) {
        double sum = 0;
        for (int i = 0; i < n - lag; i++) {
            sum += (states[i] - mean) * (states[i + lag] - mean);
        }
        autocorr[lag] = sum / (n - lag) / variance;
    }
    
    return autocorr;
}

// Функция для сохранения данных автокорреляции
void saveAutocorrelationData(const vector<double>& autocorr, const string& filename) {
    ofstream file(filename);
    for (size_t i = 0; i < autocorr.size(); i++) {
        file << i << " " << autocorr[i] << endl;
    }
    file.close();
}

// Функция для создания gnuplot скрипта
void createGnuplotScript(const string& script_name, 
                         const string& data1, const string& data2,
                         const string& output, const string& title,
                         const string& ylabel, const string& xlabel) {
    ofstream script(script_name);
    script << "set terminal png size 800,600\n";
    script << "set output '" << output << "'\n";
    script << "set xlabel '" << xlabel << "'\n";
    script << "set ylabel '" << ylabel << "'\n";
    script << "set grid\n";
    script << "plot '" << data1 << "' with lines title 'Матрица 1', \\\n";
    script << "     '" << data2 << "' with lines title 'Матрица 2'\n";
    script.close();
}

int main() {
    const int n = 5; // Размерность матриц
    const int num_steps = 100; // Количество шагов цепи Маркова
    const int max_lag = 20; // Максимальный лаг для автокорреляции
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> state_dis(0, n - 1);
    
    // 1) Генерируем две случайные дважды стохастические матрицы
    cout << "Генерация двух дважды стохастических матриц размером " << n << "x" << n << ":\n\n";
    
    auto matrix1 = generateDoublyStochasticMatrix(n);
    auto matrix2 = generateDoublyStochasticMatrix(n);
    
    // 2) Проверяем что матрицы дважды стохастические
    cout << "Проверка матрицы 1: " << (checkDoublyStochastic(matrix1) ? "OK" : "FAIL") << endl;
    cout << "Проверка матрицы 2: " << (checkDoublyStochastic(matrix2) ? "OK" : "FAIL") << endl;
    
    // Выводим матрицы
    cout << "\nМатрица 1:\n";
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            cout << fixed << setprecision(4) << matrix1[i][j] << " ";
        }
        cout << endl;
    }
    
    cout << "\nМатрица 2:\n";
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            cout << fixed << setprecision(4) << matrix2[i][j] << " ";
        }
        cout << endl;
    }
    
    // 4) Выбираем начальное состояние случайно
    int current_state1 = state_dis(gen);
    int current_state2 = state_dis(gen);
    
    cout << "\nНачальное состояние для матрицы 1: " << current_state1 << endl;
    cout << "Начальное состояние для матрицы 2: " << current_state2 << endl;
    
    // 5) Генерируем последовательности состояний
    vector<int> states1, states2;
    states1.push_back(current_state1);
    states2.push_back(current_state2);
    
    // Матрицы для подсчета частоты переходов
    vector<vector<int>> transition_count1(n, vector<int>(n, 0));
    vector<vector<int>> transition_count2(n, vector<int>(n, 0));
    
    for (int step = 1; step < num_steps; step++) {
        int prev1 = states1.back();
        int prev2 = states2.back();
        
        int next1 = nextState(prev1, matrix1, gen);
        int next2 = nextState(prev2, matrix2, gen);
        
        states1.push_back(next1);
        states2.push_back(next2);
        
        transition_count1[prev1][next1]++;
        transition_count2[prev2][next2]++;
    }
    
    // Выводим частоты переходов
    cout << "\nЧастота переходов для матрицы 1:\n";
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            cout << transition_count1[i][j] << " ";
        }
        cout << endl;
    }
    
    cout << "\nЧастота переходов для матрицы 2:\n";
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            cout << transition_count2[i][j] << " ";
        }
        cout << endl;
    }
    
    // Сохраняем данные для графиков
    saveTransitionData(states1, "states1.dat");
    saveTransitionData(states2, "states2.dat");
    
    // Вычисляем и сохраняем автокорреляцию
    auto autocorr1 = computeAutocorrelation(states1, max_lag);
    auto autocorr2 = computeAutocorrelation(states2, max_lag);
    
    saveAutocorrelationData(autocorr1, "autocorr1.dat");
    saveAutocorrelationData(autocorr2, "autocorr2.dat");
    
    // Создаем gnuplot скрипты
    createGnuplotScript("plot_transitions.gp", 
                       "states1.dat", "states2.dat",
                       "transitions.png",
                       "Сравнение траекторий цепи Маркова",
                       "Состояние", "Номер перехода");
    
    createGnuplotScript("plot_autocorr.gp",
                       "autocorr1.dat", "autocorr2.dat",
                       "autocorrelation.png",
                       "Сравнение автокорреляции",
                       "Автокорреляция", "Лаг");
    
    cout << "\nДанные сохранены в файлы:\n";
    cout << "states1.dat, states2.dat - данные траекторий\n";
    cout << "autocorr1.dat, autocorr2.dat - данные автокорреляции\n";
    cout << "plot_transitions.gp - скрипт для графика переходов\n";
    cout << "plot_autocorr.gp - скрипт для графика автокорреляции\n";
    cout << "\nДля построения графиков выполните команды:\n";
    cout << "gnuplot plot_transitions.gp\n";
    cout << "gnuplot plot_autocorr.gp\n";
    
    return 0;
}