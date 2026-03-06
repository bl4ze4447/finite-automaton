#include <iostream>

#include "finite_automaton.h"

int main() {
    finite_automaton x("finite_automaton.input");
    x.check_words_from("word_list.input");
    std::cout << x;
    return 0;
}