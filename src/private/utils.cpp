#include "utils.h"

#include <cctype>
#include <cmath>

using namespace std;

string_view Strip(string_view line) {
    while (!line.empty() && isspace(line.front())) {
        line.remove_prefix(1);
    }
    while (!line.empty() && isspace(line.back())) {
        line.remove_suffix(1);
    }
    return line;
}

bool IsZero(double x) {
    return abs(x) < 1e-6;
}
