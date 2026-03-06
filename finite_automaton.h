//
// Created by Antonie Gabriel Belu on 05.03.2026.
//

#ifndef FINITE_AUTOMATA_H
#define FINITE_AUTOMATA_H
#include <list>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <set>
#include <sstream>
// Tema 1 DFA
// Starile
// Alfabet
// Starea initiala
// Functia (pe randuri diferite)
// Stari acceptoare

// Cuvinte care spui daca sunt acceptate sau nu

// Facut si Tema 2 NFA
struct hash_pair {
    template <class T1, class T2>
    size_t operator()(const std::pair<T1, T2>& p) const
    {
        // Hash the first element
        size_t hash1 = std::hash<T1>{}(p.first);
        // Hash the second element
        size_t hash2 = std::hash<T2>{}(p.second);
        // Combine the two hash values
        return hash1
               ^ (hash2 + 0x9e3779b9 + (hash1 << 6)
                  + (hash1 >> 2));
    }
};

class finite_automaton {
    /// The 'void' character in a deterministic finite automaton
    const char LAMBDA = '?';

    /// Internal nodes for our deterministic finite automaton graph
    std::set<std::string> states;

    /// Possible values for links between states
    std::set<std::string> alphabet;

    /// Transforms the (current_state, symbol) pair into a new state
    std::unordered_map<std::pair<std::string, std::string>, std::set<std::string>, hash_pair> transition_function;

    /// Starting point for our deterministic finite automaton
    std::string initial_state;

    /// Internal nodes where we accept the input as being valid
    ///
    /// final_states <= states
    std::set<std::string> final_states;

    /// The trail of states that the FA visited
    ///
    /// If the FA rejects a word, the trail will be empty
    std::vector<std::string> states_trail;

    /// The current model of the FA
    std::string model_of_finite_automaton;

    enum class valid_word {
        UNKNOWN = -1,
        INVALID,
        VALID
    };

    /// Explores all possible symbols starting from the current_state and symbol
    void explore_symbols_nfa(std::string& current_state, int &is_word_valid, const std::string& word, size_t left);

    /// Explores only the longest possible symbol starting from the current_state and symbol
    void explore_symbols_dfa(std::string& current_state, int &is_word_valid, const std::string& word, size_t left);

    /// Switches between exploring all possible symbols and only the longest possible symbol
    /// based on model_of_finite_automaton
    void explore_symbols(std::string& current_state, int &is_word_valid, const std::string& word, size_t left);

    /// Checks if the given symbol exists in the word at the given position
    static bool symbol_exists_in_word(const std::string& word, size_t left, const std::string& symbol);
public:
    /// Initialize the FA with values from the given input_file_name
    ///
    /// Structure of file must be exactly as below (K <= N <= M):
    ///
    /// model_of_finite_automaton
    /// state1 state2 ... stateN
    /// symbol1 symbol2 ... symbolM
    /// initial_state
    /// current_state symbol new_state
    /// ....
    /// end_transition_function
    /// final_state1 final_state2 ... final_stateK
    ///
    /// model_of_finite_automaton can be either dfa or nfa
    finite_automaton(const std::string& input_file_name);

    /// Print a message about the validity of a word and increment words_passed
    /// if it had the expected result.
    void invalid_word_message(int known_validity, size_t& words_passed) const;

    /// Print a message about the validity of a word and increment words_passed
    /// if it had the expected result.
    void valid_word_message(int known_validity, size_t& words_passed) const;

    /// Check words from the given input file and print results
    ///
    /// Structure of a file should be exactly as below (* means optional)
    ///
    /// word1 expected_result1*
    /// word2 expected_result2*
    /// ...
    /// wordN expected_resultN*
    ///
    /// Values for should be 0, if the word is invalid, or 1 if the word is valid
    void check_words_from(const std::string& input_file_name);

    /// Recursively checks a word, by going multiple paths to check for cases
    /// such as having a symbol that starts with another symbol ('a', 'aa', 'ab'),
    /// or for cases where we have two or more trails that can be taken ((q0, a), (q0, b))
    bool check_word(const std::string& word, const std::string& state, size_t left = 0);

    friend std::ostream& operator<<(std::ostream& os, const finite_automaton& d) {
        os << '\n';
        os << "States: ";
        for (const auto& s : d.states) {
            os << s << " ";
        }
        os << '\n';

        os << "Alphabet: ";
        for (const auto& a : d.alphabet) {
            os << a << " ";
        }
        os << '\n';

        os << "Initial state: " << d.initial_state << '\n';

        os << "Transition functions (F : (States, Alphabet) -> States), " << '\n';
        for (const auto& f : d.transition_function) {
            for (const auto& fn : f.second) {
                os << "\tF(" << f.first.first << ", " << f.first.second << ") = " << fn << '\n';
            }
        }

        os << "Final states: ";
        for (const auto& s : d.final_states) {
            os << s << " ";
        }
        os << '\n';

        return os;
    }
};


#endif //FINITE_AUTOMATA_H