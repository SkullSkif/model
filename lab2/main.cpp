#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <climits>

using namespace std;

struct Work {
    int start;           // i - начало работы
    int end;             // j - конец работы
    int duration;        // tij - продолжительность
    
    // Временные параметры
    int t_early_start;   // t^РН_ij - раннее начало
    int t_early_finish;  // t^РО_ij - раннее окончание
    int t_late_start;    // t^ПН_ij - позднее начало
    int t_late_finish;   // t^ПО_ij - позднее окончание
    int total_float;     // R_ij - полный резерв
    int free_float;      // r_ij - свободный резерв
    
    Work(int i, int j, int d) : start(i), end(j), duration(d),
                                t_early_start(0), t_early_finish(0),
                                t_late_start(0), t_late_finish(0),
                                total_float(0), free_float(0) {}
};

class NetworkGraph {
private:
    int num_events;                 // количество событий
    vector<Work> works;              // список работ
    vector<vector<int>> adj_list;    // список смежности для прямых связей
    vector<vector<int>> pred_list;   // список предшественников
    
    // Поиск максимального пути (ранние сроки)
    void calculateEarlyTimes() {
        vector<int> early_time(num_events + 1, 0);
        
        // Топологический порядок
        for (int i = 1; i <= num_events; i++) {
            for (int work_idx : adj_list[i]) {
                Work& w = works[work_idx];
                int new_time = early_time[w.start] + w.duration;
                if (new_time > early_time[w.end]) {
                    early_time[w.end] = new_time;
                }
            }
        }
        
        // Установка ранних сроков для работ
        for (Work& w : works) {
            w.t_early_start = early_time[w.start];
            w.t_early_finish = w.t_early_start + w.duration;
        }
    }
    
    // Поиск поздних сроков
    void calculateLateTimes() {
        // Находим критическое время (макс раннее окончание)
        int critical_time = 0;
        for (const Work& w : works) {
            critical_time = max(critical_time, w.t_early_finish);
        }
        
        vector<int> late_time(num_events + 1, critical_time);
        
        // Обратный проход
        for (int i = num_events; i >= 1; i--) {
            for (int work_idx : pred_list[i]) {
                Work& w = works[work_idx];
                int new_time = late_time[w.end] - w.duration;
                if (new_time < late_time[w.start]) {
                    late_time[w.start] = new_time;
                }
            }
        }
        
        // Установка поздних сроков для работ
        for (Work& w : works) {
            w.t_late_finish = late_time[w.end];
            w.t_late_start = w.t_late_finish - w.duration;
        }
    }
    
    // Расчет резервов времени
    void calculateFloats() {
        for (Work& w : works) {
            // Полный резерв R_ij
            w.total_float = w.t_late_start - w.t_early_start;
            
            // Свободный резерв r_ij
            int min_early_start_next = INT_MAX;
            for (int next_idx : adj_list[w.end]) {
                const Work& next_work = works[next_idx];
                min_early_start_next = min(min_early_start_next, next_work.t_early_start);
            }
            
            if (min_early_start_next != INT_MAX) {
                w.free_float = min_early_start_next - w.t_early_finish;
            } else {
                w.free_float = 0; // Если нет последующих работ
            }
        }
    }

public:
    NetworkGraph(int events) : num_events(events) {
        adj_list.resize(events + 1);
        pred_list.resize(events + 1);
    }
    
    // Добавление работы
    void addWork(int i, int j, int duration) {
        works.push_back(Work(i, j, duration));
        int index = works.size() - 1;
        adj_list[i].push_back(index);
        pred_list[j].push_back(index);
    }
    
    // Расчет всех параметров
    void calculateAll() {
        calculateEarlyTimes();
        calculateLateTimes();
        calculateFloats();
    }
    
    // Вывод таблицы сетевого графика
    void printTable() {
        cout << "\n" << string(100, '=') << endl;
        cout << "ТАБЛИЦА СЕТЕВОГО ГРАФИКА" << endl;
        cout << string(100, '=') << endl;
        
        cout << left 
             << setw(10) << "Шифр"
             << setw(12) << "t(i,j)"
             << setw(15) << "t^РН_ij"
             << setw(15) << "t^РО_ij"
             << setw(15) << "t^ПН_ij"
             << setw(15) << "t^ПО_ij"
             << setw(12) << "R_ij"
             << setw(12) << "r_ij"
             << "Критич." << endl;
        
        cout << string(100, '-') << endl;
        
        int critical_path_length = 0;
        for (const Work& w : works) {
            critical_path_length = max(critical_path_length, w.t_early_finish);
        }
        
        for (size_t k = 0; k < works.size(); k++) {
            const Work& w = works[k];
            
            bool is_critical = (w.total_float == 0);
            
            cout << left
                 << setw(10) << (to_string(w.start) + "-" + to_string(w.end))
                 << setw(12) << w.duration
                 << setw(15) << w.t_early_start
                 << setw(15) << w.t_early_finish
                 << setw(15) << w.t_late_start
                 << setw(15) << w.t_late_finish
                 << setw(12) << w.total_float
                 << setw(12) << w.free_float;
            
            if (is_critical) {
                cout << "   Да";
            } else {
                cout << "   Нет";
            }
            cout << endl;
        }
        
        cout << string(100, '-') << endl;
        cout << "Длина критического пути: " << critical_path_length << endl;
        
        // Вывод критического пути
        cout << "\nКритический путь: ";
        findAndPrintCriticalPath();
        cout << endl;
    }
    
    // Поиск и вывод критического пути
    void findAndPrintCriticalPath() {
        vector<int> path;
        vector<bool> visited(num_events + 1, false);
        
        // Начинаем с начального события
        int current = 1;
        path.push_back(current);
        
        while (current != num_events) {
            bool found = false;
            for (int work_idx : adj_list[current]) {
                Work& w = works[work_idx];
                if (w.total_float == 0 && w.start == current) {
                    if (!visited[w.end]) {
                        current = w.end;
                        path.push_back(current);
                        visited[current] = true;
                        found = true;
                        break;
                    }
                }
            }
            if (!found) break;
        }
        
        // Вывод пути
        for (size_t i = 0; i < path.size(); i++) {
            cout << path[i];
            if (i < path.size() - 1) cout << " -> ";
        }
    }
};

int main() {
    
    // Пример 1: Простой график
    NetworkGraph graph1(7); // 7 событий
    
    // Добавление работ (начало, конец, продолжительность)
    /*
    A - 1
    B - 2
    C - 3
    D - 4
    E - 5
    F - 6
    G - 7
    */
    graph1.addWork(1, 2, 4);
    graph1.addWork(1, 3, 6);
    graph1.addWork(2, 4, 3);
    graph1.addWork(3, 5, 5);
    graph1.addWork(4, 6, 4);
    graph1.addWork(5, 6, 4);
    graph1.addWork(6, 7, 3);
    
    graph1.calculateAll();
    graph1.printTable();
    
    return 0;
}