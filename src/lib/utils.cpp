#include "utils.h"

#include <cctype>


using namespace std;

string_view strip(string_view line) {
    while (!line.empty() && isspace(line.front())) {
        line.remove_prefix(1);
    }
    while (!line.empty() && isspace(line.back())) {
        line.remove_suffix(1);
    }
    return line;
}
