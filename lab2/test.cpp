#include <algorithm>
#include <climits>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <vector>

using namespace std;

struct Work {
    int start;
    int end;
    int duration;

    int t_early_start;
    int t_early_finish;
    int t_late_start;
    int t_late_finish;
    int full_reserve;
    int reserve;

    Work(int i, int j, int d)
        : start(i),
          end(j),
          duration(d),
          t_early_start(0),
          t_early_finish(0),
          t_late_start(0),
          t_late_finish(0),
          full_reserve(0),
          reserve(0)
    {
    }
};

class Graph {
private:
    int num_events;
    vector<Work> works;
    vector<vector<int>> adj_list;
    vector<vector<int>> pred_list;
    map<int, string> event_names;
    bool use_letters;

    string getEventName(int event_num) {
        if (use_letters && event_num >= 1 && event_num <= 26) {
            return string(1, 'A' + event_num - 1);
        }
        return to_string(event_num);
    }

    void calculateEarlyTimes()
    {
        vector<int> early_time(num_events + 1, 0);

        for (int i = 1; i <= num_events; i++) {
            for (int work_idx : adj_list[i]) {
                Work& w = works[work_idx];
                int new_time = early_time[w.start] + w.duration;
                if (new_time > early_time[w.end]) {
                    early_time[w.end] = new_time;
                }
            }
        }

        for (Work& w : works) {
            w.t_early_start = early_time[w.start];
            w.t_early_finish = w.t_early_start + w.duration;
        }
    }

    void calculateLateTimes()
    {
        int max_event_time = 0;
        int last_event = 0;
        
        for (const Work& w : works) {
            if (w.t_early_finish > max_event_time) {
                max_event_time = w.t_early_finish;
                last_event = w.end;
            }
        }
        
        vector<int> late_time(num_events + 1, INT_MAX);
        
        late_time[last_event] = max_event_time;
        
        for (int event = num_events; event >= 1; event--) {
            for (int work_idx : pred_list[event]) {
                Work& w = works[work_idx];
                
                if (late_time[w.end] != INT_MAX) {
                    w.t_late_finish = late_time[w.end];
                    
                    w.t_late_start = w.t_late_finish - w.duration;
                    
                    if (late_time[w.start] > w.t_late_start) {
                        late_time[w.start] = w.t_late_start;
                    }
                }
            }
        }
        
        for (Work& w : works) {
            if (w.t_late_finish == 0) {
                w.t_late_finish = late_time[w.end];
                w.t_late_start = w.t_late_finish - w.duration;
            }
        }
    }

    void calculateFloats()
    {
        for (Work& w : works) {
            w.full_reserve = w.t_late_start - w.t_early_start;

            int min_early_start_next = INT_MAX;
            for (int next_idx : adj_list[w.end]) {
                const Work& next_work = works[next_idx];
                min_early_start_next
                        = min(min_early_start_next, next_work.t_early_start);
            }

            if (min_early_start_next != INT_MAX) {
                w.reserve = min_early_start_next - w.t_early_finish;
            } else {
                w.reserve = 0;
            }
        }
    }

public:
    Graph() : num_events(0), use_letters(false)
    {
    }

    void setUseLetters(bool use) {
        use_letters = use;
    }

    bool loadFromFile(const string& filename)
    {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "ERROR: Не удалось открыть файл " << filename << endl;
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
            if (line.empty())
                continue;

            istringstream iss(line);
            int ver, predecessor, weight;

            if (iss >> ver >> predecessor >> weight) {
                max_event = max(max_event, ver);
                if (predecessor > 0) {
                    max_event = max(max_event, predecessor);
                }
                temp_works.push_back(make_tuple(predecessor, ver, weight));
            }
        }

        file.close();

        if (temp_works.empty()) {
            cerr << "ERROR: Файл не содержит данных" << endl;
            return false;
        }

        num_events = max_event;
        adj_list.resize(num_events + 1);
        pred_list.resize(num_events + 1);

        for (const auto& [pred, ver, weight] : temp_works) {
            if (pred > 0) {
                addWork(pred, ver, weight);
            } else {
                addWork(1, ver, weight);
            }
        }

        return true;
    }

    void addWork(int i, int j, int duration)
    {
        works.push_back(Work(i, j, duration));
        int index = works.size() - 1;
        adj_list[i].push_back(index);
        pred_list[j].push_back(index);
    }

    void calculateAll()
    {
        if (works.empty()) {
            cerr << "ERROR: Нет данных для расчета" << endl;
            return;
        }
        calculateEarlyTimes();
        calculateLateTimes();
        calculateFloats();
    }

    void printTable()
    {
        if (works.empty()) {
            cout << "Нет данных для отображения" << endl;
            return;
        }
        
        cout << left << setw(12) << "Шифр" << setw(12) << "t(i,j)" << setw(15)
             << "t^РН_ij" << setw(15) << "t^РО_ij" << setw(15) << "t^ПН_ij"
             << setw(15) << "t^ПО_ij" << setw(12) << "R_ij" << setw(12)
             << "r_ij"
             << "Кр." << endl;

        int critical_path_length = 0;
        for (const Work& w : works) {
            critical_path_length = max(critical_path_length, w.t_early_finish);
        }

        for (size_t k = 0; k < works.size(); k++) {
            const Work& w = works[k];

            bool is_critical = (w.full_reserve == 0);

            string code = getEventName(w.start) + '-' + getEventName(w.end);

            cout << left << setw(12) << code << setw(12) << w.duration
                 << setw(15) << w.t_early_start << setw(15) << w.t_early_finish
                 << setw(15) << w.t_late_start << setw(15) << w.t_late_finish
                 << setw(12) << w.full_reserve << setw(12) << w.reserve;

            if (is_critical)
                cout << " Критическая";

            cout << endl;
        }

        cout << "Длина критического пути: " << critical_path_length << endl;

        cout << "\nКритический путь: ";
        findAndPrintCriticalPath();
        cout << endl;
    }

    void findAndPrintCriticalPath()
    {
        vector<int> path;
        vector<bool> visited(num_events + 1, false);

        int current = 1;
        path.push_back(current);

        while (current != num_events) {
            bool found = false;
            for (int work_idx : adj_list[current]) {
                Work& w = works[work_idx];
                if (w.full_reserve == 0 && w.start == current) {
                    if (!visited[w.end]) {
                        current = w.end;
                        path.push_back(current);
                        visited[current] = true;
                        found = true;
                        break;
                    }
                }
            }
            if (!found)
                break;
        }

        for (size_t i = 0; i < path.size(); i++) {
            cout << getEventName(path[i]);
            if (i < path.size() - 1)
                cout << " -> ";
        }
    }
};

int main()
{
    Graph graph;
    char choice;
    char letter_choice;

    cout << "Использовать данные с моей карточки? (Вариант 20) (y/n): ";
    cin >> choice;

    cout << "Использовать буквы английского алфавита для вершин? (y/n): ";
    cin >> letter_choice;
    
    graph.setUseLetters(letter_choice == 'y' || letter_choice == 'Y');

    if (choice == 'y' || choice == 'Y') {
        Graph test_graph;
        test_graph.setUseLetters(letter_choice == 'y' || letter_choice == 'Y');

        ofstream test_file("test_graph.txt");
        test_file << "2 1 4\n";
        test_file << "3 1 6\n";
        test_file << "4 2 3\n";
        test_file << "5 3 5\n";
        test_file << "6 4 4\n";
        test_file << "6 5 4\n";
        test_file << "7 6 3\n";
        test_file.close();

        if (test_graph.loadFromFile("test_graph.txt")) {
            test_graph.calculateAll();
            test_graph.printTable();
        }
    } else {
        string filename;

        cout << "Введите имя файла с данными: ";
        cin >> filename;

        if (graph.loadFromFile(filename)) {
            graph.calculateAll();
            graph.printTable();
        }
    }

    return 0;
}