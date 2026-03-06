//
// Created by Antonie Gabriel Belu on 05.03.2026.
//

#include "finite_automaton.h"

#include <iostream>

finite_automaton::finite_automaton(const std::string& input_file_name) {
    std::ifstream read(input_file_name);
    std::istringstream ss;
    std::string line;

    // Read finite automaton model
    std::getline(read, line);
    ss = std::istringstream(line);
    ss >> model_of_finite_automaton;

    // Read states
    std::getline(read, line);
    ss = std::istringstream(line);
    std::string state;
    while (ss >> state)
        states.insert(state);

    // Read alphabet
    std::getline(read, line);
    ss = std::istringstream(line);
    std::string symbol;
    while (ss >> symbol)
        alphabet.insert(symbol);

    // Read initial state
    std::getline(read, line);
    ss = std::istringstream(line);
    ss >> initial_state;

    // Read transition function pairs
    std::getline(read, line);
    std::string current_state, new_state;
    while (line != "end_transition_function") {
        ss = std::istringstream(line);
        ss >> current_state >> symbol >> new_state;
        if (transition_function.contains({current_state, symbol})) {
            if (model_of_finite_automaton == "dfa")
                throw std::invalid_argument("finite_automaton::finite_automaton(" + input_file_name + "): dfa model cannot have duplicate transitions.");

            transition_function[{current_state, symbol}].insert(new_state);
        } else {
            std::set<std::string> new_state_set;
            new_state_set.insert(new_state);
            transition_function[{current_state, symbol}] = new_state_set;
        }

        std::getline(read, line);
    }

    // Read final states
    std::getline(read, line);
    ss = std::istringstream(line);
    std::string final_state;
    while (ss >> final_state)
        final_states.insert(final_state);
}

void finite_automaton::check_words_from(const std::string &input_file_name) {
    std::ifstream read(input_file_name);
    std::string line;
    std::string word;

    // Used for testing. If the FA has another value than this, the user will be informed.
    int known_validity = static_cast<int>(valid_word::UNKNOWN);

    // Increases each time the FA has the same result as known_validty at the end
    // of the word check.
    size_t words_passed = 0;
    size_t num_of_words = 0;
    while (std::getline(read, line)) {
        std::istringstream ss(line);
        ss >> word;
        if (ss.fail())
            throw std::invalid_argument("finite_automaton::check_words_from(" + input_file_name + "): cannot read word.");

        ss >> known_validity;
        if (ss.fail())
            known_validity = static_cast<int>(valid_word::UNKNOWN);

        this->states_trail.clear();

        std::cout << "[Info] Checking word: " << word << '\n';
        if (check_word(word, this->initial_state, 0))
            valid_word_message(known_validity, words_passed);
        else
            invalid_word_message(known_validity, words_passed);

        ++num_of_words;
    }

    std::cout << "Number of words: " << num_of_words << '\n';
    std::cout << "Words passed: " << words_passed << '\n';
}

void finite_automaton::explore_symbols(std::string& current_state, int &is_word_valid, const std::string& word, const size_t left) {
    if (this->model_of_finite_automaton == "dfa") {
        explore_symbols_dfa(current_state, is_word_valid, word, left);
        return;
    }

    explore_symbols_nfa(current_state, is_word_valid, word, left);
}

bool finite_automaton::symbol_exists_in_word(const std::string& word, const size_t left, const std::string& symbol) {
    if (symbol.size() + left > word.size())
        return false;

    // Does this symbol exist in the word?
    size_t symbol_ia_index = 0;
    for (const auto& ch : symbol) {
        if (word[left + symbol_ia_index] != ch)
           return false;

        ++symbol_ia_index;
    }

    return true;
}

void finite_automaton::explore_symbols_dfa(std::string& current_state, int &is_word_valid, const std::string& word, const size_t left) {
    std::string longest_symbol;
    bool invalid_symbol = true;
    for (const auto& symbol_in_alphabet : this->alphabet) {
        if (!symbol_exists_in_word(word, left, symbol_in_alphabet))
            continue;

        invalid_symbol = false;
        if (longest_symbol.size() < symbol_in_alphabet.size())
            longest_symbol = symbol_in_alphabet;
    }

    if (invalid_symbol || !this->transition_function.contains({current_state, longest_symbol})) {
        is_word_valid = static_cast<int>(valid_word::INVALID);
        return;
    }

    for (const auto& next_state : this->transition_function[{current_state, longest_symbol}]) {
        if (check_word(word, next_state, left + longest_symbol.size())) {
            is_word_valid = static_cast<int>(valid_word::VALID);
            return;
        }
    }

    // Dead end.
    is_word_valid = static_cast<int>(valid_word::INVALID);
}

void finite_automaton::explore_symbols_nfa(std::string& current_state, int &is_word_valid, const std::string& word, const size_t left) {
    for (const auto& symbol_in_alphabet : this->alphabet) {
        if (!symbol_exists_in_word(word, left, symbol_in_alphabet))
            continue;

        if (!this->transition_function.contains({current_state, symbol_in_alphabet}))
            continue;

        // Explore all possible 'next' states and try to find the optimal one
        for (const auto& next_state : this->transition_function[{current_state, symbol_in_alphabet}]) {
            if (check_word(word, next_state, left + symbol_in_alphabet.size())) {
                is_word_valid = static_cast<int>(valid_word::VALID);
                return;
            }
        }
    }

    is_word_valid = static_cast<int>(valid_word::INVALID);
}

bool finite_automaton::check_word(const std::string& word, const std::string& state, const size_t left) {
    states_trail.push_back(state);

    // Base cases
    if (left == word.size() && this->final_states.contains(state))
        return true;

    if (left == word.size()) {
        states_trail.pop_back();
        return false;
    }

    if (this->LAMBDA == word[left] && this->final_states.contains(state))
        return true;

    if (this->LAMBDA == word[left]) {
        states_trail.clear();
        return false;
    }

    std::string current_state = state;
    int is_word_valid = static_cast<int>(valid_word::UNKNOWN);
    explore_symbols(current_state, is_word_valid, word, left);

    if (is_word_valid == static_cast<int>(valid_word::VALID))
        return true;

    states_trail.pop_back();
    return false;
}

void finite_automaton::invalid_word_message(const int known_validity, size_t& words_passed) const {
    if (known_validity == static_cast<int>(valid_word::UNKNOWN)) {
        std::cout << "[?] FA: Invalid | Test Case: Missing." << '\n';
    } else if (known_validity == static_cast<int>(valid_word::VALID)) {
        std::cout << "[x] FA: Invalid | Test Case: Valid." << '\n';
    } else {
        ++words_passed;
        std::cout << "[OK] FA: Invalid | Test Case: Invalid." << '\n';
    }

    std::cout << "[Trail of states] ";
    if (states_trail.empty()) {
        std::cout << "No states visited." << "\n\n";
        return;
    }

    std::cout << states_trail.front();
    for (size_t i = 1; i < states_trail.size(); ++i) {
        std::cout << " -> " << states_trail[i];
    }
    std::cout << "\n\n";
}

void finite_automaton::valid_word_message(const int known_validity, size_t& words_passed) const {
    if (known_validity == static_cast<int>(valid_word::UNKNOWN)) {
        std::cout << "[?] FA: Valid | Test Case: Missing." << '\n';
    } else if (known_validity == static_cast<int>(valid_word::INVALID)) {
        std::cout << "[x] FA: Valid | Test Case: Invalid." << '\n';
    } else {
        ++words_passed;
        std::cout << "[OK] FA: Valid | Test Case: Valid." << '\n';
    }

    std::cout << "[Trail of states] ";
    if (states_trail.empty()) {
        std::cout << "No states visited." << "\n\n";
        return;
    }

    std::cout << states_trail.front();
    for (size_t i = 1; i < states_trail.size(); ++i) {
        std::cout << " -> " << states_trail[i];
    }
    std::cout << "\n\n";
}