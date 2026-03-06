#include <iostream>

#include "finite_automaton.h"

int main() {
    finite_automaton x("input.txt");
    x.check_words_from("cuvinte.txt");
    std::cout << x;
    return 0;
}