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

void  finite_automaton::check_words_from(const std::string &input_file_name) {
    std::ifstream read(input_file_name);
    std::string line;
    std::string word;

    // Used for testing. If the FA has another value than this, the user will be informed.
    int known_validity = static_cast<int>(valid_word::UNKNOWN);

    // Increases each time the FA has the same value as is_valid at the end
    // of the word check.
    size_t words_passed = 0;
    size_t num_of_words = 0;
    while (std::getline(read, line)) {
        ++num_of_words;
        std::istringstream iss(line);
        iss >> word;
        if (iss.fail())
            throw std::invalid_argument("finite_automaton::check_words_from(" + input_file_name + "): cannot read word.");

        iss >> known_validity;
        if (iss.fail()) known_validity = static_cast<int>(valid_word::UNKNOWN);

        std::cout << "[Info] Checking word: " << word << '\n';

        std::string current_state = this->initial_state;
        this->states_trail.clear();
        states_trail.push_back(current_state);

        size_t nth_symbol = 0;
        int is_word_valid = static_cast<int>(valid_word::UNKNOWN);
        for (const auto& symbol : word) {
            // Check if our FA accepts lambda
            if (this->LAMBDA == symbol && this->final_states.contains(current_state)) {
                valid_word_message(known_validity, words_passed);
                break;
            }
            if (this->LAMBDA == symbol) {
                states_trail.clear();
                invalid_word_message(known_validity, words_passed);
                break;
            }

            // Is this symbol missing from our alphabet?
            bool invalid_symbol = true;
            for (const auto& symbol_in_alphabet : this->alphabet) {
                if (symbol_in_alphabet.size() + nth_symbol > word.size())
                    continue;

                // Check if we can use this symbol
                // Also treats cases where:
                // alphabet = [a, aa, aaa]
                // symbol = a
                size_t symbol_ia_index = 0;
                bool symbol_exists_in_word = true;
                for (const auto& ch : symbol_in_alphabet) {
                    if (word[nth_symbol + symbol_ia_index] != ch)
                        symbol_exists_in_word = false;

                    ++symbol_ia_index;
                }

                if (symbol_exists_in_word == false)
                    continue;

                invalid_symbol = false;

                // Does this one lead us to a valid choice?
                if (!this->transition_function.contains({current_state, symbol_in_alphabet})) {
                    is_word_valid = static_cast<int>(valid_word::INVALID);
                    break;
                }

                // Explore all possible 'next' states and try to find the optimal one
                for (const auto& next_state : this->transition_function[{current_state, symbol_in_alphabet}]) {
                    if (check_word(word.substr(nth_symbol + symbol_in_alphabet.size()), next_state)) {
                        is_word_valid = static_cast<int>(valid_word::VALID);
                        break;
                    }
                }

                if (is_word_valid == static_cast<int>(valid_word::VALID))
                    break;

                // Dead end, reverse our road.
                states_trail.pop_back();
                is_word_valid = 0;
                break;
            }

            if (is_word_valid == static_cast<int>(valid_word::VALID)) {
                valid_word_message(known_validity, words_passed);
                break;
            }

            if (is_word_valid == static_cast<int>(valid_word::INVALID) || invalid_symbol) {
                this->states_trail.clear();
                invalid_word_message(known_validity, words_passed);
                break;
            }

            ++nth_symbol;
        }

        if (!this->final_states.contains(current_state) && is_word_valid == static_cast<int>(valid_word::UNKNOWN)) {
            valid_word_message(known_validity, words_passed);
        }
    }

    std::cout << "Number of words: " << num_of_words << '\n';
    std::cout << "Words passed: " << words_passed << '\n';
}

bool  finite_automaton::check_word(const std::string& word, const std::string& state) {
    states_trail.push_back(state);
    if (word.empty() && this->final_states.contains(state))
        return true;

    size_t nth_symbol = 0;
    std::string current_state = state;
    for (const auto& symbol : word) {
        for (const auto& symbol_in_alphabet : this->alphabet) {
            if (symbol_in_alphabet.size() + nth_symbol > word.size())
                continue;

            if (!symbol_in_alphabet.starts_with(symbol))
                continue;

            size_t symbol_ia_index = 0;
            bool symbol_exists_in_word = true;
            for (const auto& ch : symbol_in_alphabet) {
                if (word[nth_symbol + symbol_ia_index] != ch)
                    symbol_exists_in_word = false;

                ++symbol_ia_index;
            }

            if (symbol_exists_in_word == false)
                continue;

            // Does this one lead us to a valid choice?
            if (!this->transition_function.contains({current_state, symbol_in_alphabet}))
                continue;

            for (const auto& next_state : this->transition_function[{current_state, symbol_in_alphabet}]) {
                if (check_word(word.substr(nth_symbol + symbol_in_alphabet.size()), next_state)) {
                    return true;
                }
            }
        }

        if (!this->transition_function.contains({current_state, std::to_string(symbol)})) {
            states_trail.pop_back();
            return false;
        }
        ++nth_symbol;
    }


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
        std::cout << "[OK] FA: Invalid | Test Case: Valid." << '\n';
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