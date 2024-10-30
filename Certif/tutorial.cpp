#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath> // pour std::abs

using namespace std;

int main() {
    int n;
    cin >> n;
    
    if (n == 0) {
        cout << "0" << endl;
        return 0;
    }

    vector<int> temperatures(n);
    for (int& t : temperatures) {
        cin >> t;
    }

    // Trouver l'élément avec la valeur absolue la plus proche de 0
    auto closest = *min_element(temperatures.begin(), temperatures.end(), [](int a, int b) {
        // Compare les valeurs absolues, et si égales, favorise les positifs
        return (abs(a) < abs(b)) || (abs(a) == abs(b) && a > b);
    });

    cout << closest << endl;
    return 0;
}