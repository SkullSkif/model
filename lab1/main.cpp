#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <iomanip>

using namespace std;

const double a = 600.0 / 217.0;        
const double b = 1680.0 / 217.0;       
const double f_1= 147.0 / 217.0;    


void generate_samples(int n) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, 1.0);
    
    double x = 0;
    ofstream file("gen_data.txt");
    
    for (int i = 0; i < n; i++) {
        double y = dis(gen);
        
        if (y <= f_1) {
            // x = 0.3 + sqrt(2u/a)
            x = 0.3 + sqrt(2.0 * y / a);
            file << x << ' ' << y << '\n';
        } else {
            // x = 1.5 - (1/8 - 3(y-f_1)/b)^(1/3)
            double x_temp = 1.0/8.0 - 3.0 * (y - f_1) / b;
            x = 1.5 - cbrt(x_temp);
            file << x << ' ' << y << '\n';
        }
    }
    file.close();
}


int main() {
    const int N = 1000000;  
    
    generate_samples(N);
    
    return 0;
}