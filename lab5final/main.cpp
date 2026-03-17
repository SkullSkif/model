#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <iomanip>
#include <cmath>

using namespace std;


vector<vector<double>> generateStochasticMatrix(int n) {
    vector<vector<double>> matrix(n, vector<double>(n));
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, 1.0);
    
    
    
    for (int i = 0; i < n; i++) {
        double row_sum = 0;
        for (int j = 0; j < n; j++) {
            matrix[i][j] = dis(gen);
            row_sum += matrix[i][j];
        }
        
        for (int j = 0; j < n; j++) {
            matrix[i][j] /= row_sum;
        }
    }
    
    
    const int max_iter = 1000;
    const double tolerance = 1e-10;
    
    for (int iter = 0; iter < max_iter; iter++) {
        
        vector<double> col_sum(n, 0);
        for (int j = 0; j < n; j++) {
            for (int i = 0; i < n; i++) {
                col_sum[j] += matrix[i][j];
            }
            for (int i = 0; i < n; i++) {
                matrix[i][j] /= col_sum[j];
            }
        }
        
        
        vector<double> row_sum(n, 0);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                row_sum[i] += matrix[i][j];
            }
            for (int j = 0; j < n; j++) {
                matrix[i][j] /= row_sum[i];
            }
        }
        
        
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


bool checkStochastic(const vector<vector<double>>& matrix, double tolerance = 1e-10) {
    int n = matrix.size();
    
    
    for (int i = 0; i < n; i++) {
        double row_sum = 0;
        for (int j = 0; j < n; j++) {
            if (matrix[i][j] < 0 || matrix[i][j] > 1) return false;
            row_sum += matrix[i][j];
        }
        if (abs(row_sum - 1.0) > tolerance) return false;
    }
    
    
    for (int j = 0; j < n; j++) {
        double col_sum = 0;
        for (int i = 0; i < n; i++) {
            col_sum += matrix[i][j];
        }
        if (abs(col_sum - 1.0) > tolerance) return false;
    }
    
    return true;
}


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
    return n - 1; 
}


void saveMatrixToFile(const vector<vector<double>>& matrix, const string& filename) {
    ofstream file(filename);
    int n = matrix.size();
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            file << fixed << setprecision(6) << matrix[i][j];
            if (j < n - 1) file << " ";
        }
        file << endl;
    }
    file.close();
}


void saveTransitionData(const vector<int>& states, const string& filename) {
    ofstream file(filename);
    for (size_t i = 0; i < states.size(); i++) {
        file << i << " " << states[i] << endl;
    }
    file.close();
}


void saveTransitionFrequency(const vector<vector<int>>& transitions, const string& filename) {
    ofstream file(filename);
    int n = transitions.size();
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            file << transitions[i][j];
            if (j < n - 1) file << " ";
        }
        file << endl;
    }
    file.close();
}


vector<double> computeAutocorrelation(const vector<int>& states, int max_lag) {
    int n = states.size();
    vector<double> autocorr(max_lag + 1, 0);
    
    
    double mean = 0;
    for (int state : states) {
        mean += state;
    }
    mean /= n;
    
    
    double variance = 0;
    for (int state : states) {
        variance += (state - mean) * (state - mean);
    }
    variance /= n;
    
    
    for (int lag = 0; lag <= max_lag; lag++) {
        double sum = 0;
        for (int i = 0; i < n - lag; i++) {
            sum += (states[i] - mean) * (states[i + lag] - mean);
        }
        autocorr[lag] = sum / (n - lag) / variance;
    }
    
    return autocorr;
}


void saveAutocorrelationData(const vector<double>& autocorr, const string& filename) {
    ofstream file(filename);
    for (size_t i = 0; i < autocorr.size(); i++) {
        file << i << " " << autocorr[i] << endl;
    }
    file.close();
}

int main() {
    const int n = 5; 
    const int num_steps = 100; 
    const int max_lag = 20; 
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> state_dis(0, n - 1);
    
    
    cout << "Генерация двух двустохастических матриц размером " << n << "x" << n << ":\n\n";
    
    auto matrix1 = generateStochasticMatrix(n);
    auto matrix2 = generateStochasticMatrix(n);
    
    
    cout << "Проверка матрицы 1: " << (checkStochastic(matrix1) ? "OK" : "FAIL") << endl;
    cout << "Проверка матрицы 2: " << (checkStochastic(matrix2) ? "OK" : "FAIL") << endl;
    
    
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
    
    
    saveMatrixToFile(matrix1, "matrix1.txt");
    saveMatrixToFile(matrix2, "matrix2.txt");
    
    
    int current_state1 = state_dis(gen);
    int current_state2 = state_dis(gen);
    
    cout << "\nНачальное состояние для матрицы 1: " << current_state1 << endl;
    cout << "Начальное состояние для матрицы 2: " << current_state2 << endl;
    
    
    vector<int> states1, states2;
    states1.push_back(current_state1);
    states2.push_back(current_state2);
    
    
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
    
    
    saveTransitionFrequency(transition_count1, "frequencies1.txt");
    saveTransitionFrequency(transition_count2, "frequencies2.txt");
    
    
    saveTransitionData(states1, "states1.dat");
    saveTransitionData(states2, "states2.dat");
    
    
    auto autocorr1 = computeAutocorrelation(states1, max_lag);
    auto autocorr2 = computeAutocorrelation(states2, max_lag);
    
    saveAutocorrelationData(autocorr1, "autocorr1.dat");
    saveAutocorrelationData(autocorr2, "autocorr2.dat");
    
    return 0;
}