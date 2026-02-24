#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <climits>
#include <fstream>
#include <sstream>
#include <map>

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
    map<int, string> event_names;    // названия событий (если есть)
    
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
    NetworkGraph() : num_events(0) {}
    
    // Загрузка графа из файла
    bool loadFromFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Ошибка: Не удалось открыть файл " << filename << endl;
            return false;
        }
        
        // Очищаем текущие данные
        works.clear();
        adj_list.clear();
        pred_list.clear();
        event_names.clear();
        
        string line;
        int max_event = 0;
        vector<tuple<int, int, int>> temp_works; // временное хранение работ
        
        // Чтение файла
        while (getline(file, line)) {
            // Пропускаем пустые строки и комментарии
            if (line.empty() || line[0] == '#') continue;
            
            istringstream iss(line);
            int vertex, predecessor, weight;
            
            if (iss >> vertex >> predecessor >> weight) {
                // Обновляем максимальный номер события
                max_event = max(max_event, vertex);
                if (predecessor > 0) {
                    max_event = max(max_event, predecessor);
                }
                
                temp_works.push_back(make_tuple(predecessor, vertex, weight));
            }
        }
        
        file.close();
        
        if (temp_works.empty()) {
            cerr << "Ошибка: Файл не содержит данных" << endl;
            return false;
        }
        
        // Устанавливаем количество событий
        num_events = max_event;
        adj_list.resize(num_events + 1);
        pred_list.resize(num_events + 1);
        
        // Добавляем работы
        for (const auto& [pred, vertex, weight] : temp_works) {
            if (pred > 0) {
                addWork(pred, vertex, weight);
            } else {
                // Если предшественник 0, значит это начальное событие
                // Создаем фиктивную работу от начального события (1)
                // или добавляем работу с предшественником 1, если vertex не 1
                if (vertex != 1) {
                    // Можно добавить работу от 1 к vertex, если это первая работа
                    // Но лучше обработать это отдельно
                }
                addWork(1, vertex, weight);
            }
        }
        
        cout << "Граф успешно загружен из файла " << filename << endl;
        cout << "Количество событий: " << num_events << endl;
        cout << "Количество работ: " << works.size() << endl;
        
        return true;
    }
    
    // Загрузка с возможностью задания имен событий
    bool loadFromFileWithNames(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Ошибка: Не удалось открыть файл " << filename << endl;
            return false;
        }
        
        works.clear();
        adj_list.clear();
        pred_list.clear();
        event_names.clear();
        
        string line;
        int max_event = 0;
        vector<tuple<int, int, int>> temp_works;
        
        while (getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            
            istringstream iss(line);
            string vertex_str, pred_str, weight_str;
            
            if (iss >> vertex_str >> pred_str >> weight_str) {
                int vertex = stoi(vertex_str);
                int predecessor = stoi(pred_str);
                int weight = stoi(weight_str);
                
                max_event = max(max_event, vertex);
                if (predecessor > 0) {
                    max_event = max(max_event, predecessor);
                }
                
                temp_works.push_back(make_tuple(predecessor, vertex, weight));
                
                // Сохраняем имена событий (можно добавить отдельный формат)
                event_names[vertex] = to_string(vertex);
                if (predecessor > 0) {
                    event_names[predecessor] = to_string(predecessor);
                }
            }
        }
        
        file.close();
        
        if (temp_works.empty()) {
            cerr << "Ошибка: Файл не содержит данных" << endl;
            return false;
        }
        
        num_events = max_event;
        adj_list.resize(num_events + 1);
        pred_list.resize(num_events + 1);
        
        for (const auto& [pred, vertex, weight] : temp_works) {
            if (pred > 0) {
                addWork(pred, vertex, weight);
            } else {
                addWork(1, vertex, weight);
            }
        }
        
        return true;
    }
    
    // Добавление работы
    void addWork(int i, int j, int duration) {
        works.push_back(Work(i, j, duration));
        int index = works.size() - 1;
        adj_list[i].push_back(index);
        pred_list[j].push_back(index);
    }
    
    // Установка имен событий
    void setEventName(int event, const string& name) {
        event_names[event] = name;
    }
    
    // Расчет всех параметров
    void calculateAll() {
        if (works.empty()) {
            cerr << "Ошибка: Нет данных для расчета" << endl;
            return;
        }
        calculateEarlyTimes();
        calculateLateTimes();
        calculateFloats();
    }
    
    // Вывод таблицы сетевого графика
    void printTable() {
        if (works.empty()) {
            cout << "Нет данных для отображения" << endl;
            return;
        }
        
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
            
            // Формируем шифр работы с учетом имен событий
            string work_code;
            if (event_names.count(w.start) && event_names.count(w.end)) {
                work_code = event_names[w.start] + "-" + event_names[w.end];
            } else {
                work_code = to_string(w.start) + "-" + to_string(w.end);
            }
            
            cout << left
                 << setw(10) << work_code
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
        
        for (size_t i = 0; i < path.size(); i++) {
            if (event_names.count(path[i])) {
                cout << event_names[path[i]];
            } else {
                cout << path[i];
            }
            if (i < path.size() - 1) cout << " -> ";
        }
    }
    
    // Вывод информации о графе
    void printGraphInfo() {
        cout << "\nИнформация о графе:" << endl;
        cout << "Количество событий: " << num_events << endl;
        cout << "Количество работ: " << works.size() << endl;
        cout << "\nСписок работ:" << endl;
        for (const Work& w : works) {
            cout << w.start << " -> " << w.end << " : " << w.duration << endl;
        }
    }
};

int main() {
    setlocale(LC_ALL, "Russian");
    
    NetworkGraph graph;
    string filename;
    
    cout << "Программа расчета параметров сетевого графика" << endl;
    cout << "Формат файла: вершина предшественник вес" << endl;
    cout << "Пример: 2 1 5 (вершина 2, предшественник 1, вес 5)" << endl;
    cout << "Для начальных вершин предшественник = 0" << endl;
    cout << string(50, '-') << endl;
    
    cout << "Введите имя файла с данными: ";
    cin >> filename;
    
    if (graph.loadFromFile(filename)) {
        graph.calculateAll();
        graph.printTable();
    } else {
        cout << "Хотите использовать тестовые данные? (y/n): ";
        char choice;
        cin >> choice;
        
        if (choice == 'y' || choice == 'Y') {
            // Создаем тестовый граф
            cout << "\nИспользуем тестовые данные..." << endl;
            
            // Пример графа из 7 событий
            NetworkGraph test_graph;
            
            // Создаем временный файл с тестовыми данными
            ofstream test_file("test_graph.txt");
            test_file << "# Формат: вершина предшественник вес\n";
            test_file << "2 1 4\n";
            test_file << "3 1 6\n";
            test_file << "4 2 3\n";
            test_file << "5 3 5\n";
            test_file << "6 4 4\n";
            test_file << "6 5 4\n";
            test_file << "7 6 3\n";
            test_file.close();
            
            cout << "Создан тестовый файл test_graph.txt" << endl;
            
            if (test_graph.loadFromFile("test_graph.txt")) {
                test_graph.calculateAll();
                test_graph.printTable();
            }
        }
    }
    
    return 0;
}