#include "EarleyParser.h"
#include <iostream>
#include <sstream>

Symbol::Symbol(uint32_t id, bool is_terminal) : id(id), is_terminal(is_terminal) {}

Situation::Situation(uint32_t grammar_id, uint32_t left, uint32_t point_ind) : grammar_id(grammar_id), left(left), point_ind(point_ind) {}

inline std::size_t Situation::HashFunction::operator()(const Situation& a) const {
        return std::hash<uint32_t>()(a.grammar_id) ^ std::hash<uint32_t>()(a.left) ^ std::hash<uint32_t>()(a.point_ind);
    }


bool Situation::operator==(const Situation& other) const {
    return other.grammar_id == grammar_id && other.left == left && other.point_ind == point_ind;
}


void EarleyParser::fit(const std::string& not_terminal_alphabet, const std::string& terminal_alphabet, 
                        const std::vector<std::string>& given_grammar, char start_symbol) {
    not_terminal_id.clear();
    terminal_id.clear();
    uint32_t id = 2;
    for (char c : not_terminal_alphabet)
        not_terminal_id[c] = id++;
    not_terminal_size = id;
    id = 0;
    for (char c : terminal_alphabet)
        terminal_id[c] = id++;
    terminal_size = id;
    grammar.assign(given_grammar.size(), {});
    for (uint32_t i = 0; i < given_grammar.size(); ++i) {
        const auto& str = given_grammar[i];
        std::size_t pos = str.find("->");
        if (pos == std::string::npos)
            throw std::invalid_argument("No -> in grammar rule");
        grammar[i].left.id = 1;
        grammar[i].left.is_terminal = false;
        for (uint32_t j = 0; j < pos; ++j) {
            if (str[j] == ' ')
                continue;
            if (grammar[i].left.id != 1)
                throw std::invalid_argument("Must be only one symbol in the left side");
            if (not_terminal_id.find(str[j]) == not_terminal_id.end())
                throw std::invalid_argument("Symbol is not in not terminal alphabet");
            grammar[i].left.id = not_terminal_id[str[j]];
        }
        for (uint32_t j = pos + 2; j < str.size(); ++j) {
            if (str[j] == ' ')
                continue;
            Symbol s;
            if (not_terminal_id.find(str[j]) != not_terminal_id.end()) {
                s.is_terminal = false;
                s.id = not_terminal_id[str[j]];
            }
            else if (terminal_id.find(str[j]) != terminal_id.end()) {
                s.is_terminal = true;
                s.id = terminal_id[str[j]];
            }
            else
                throw std::invalid_argument("Symbol is not in alphabet");
            grammar[i].right.push_back(s);
        }
    }
    if (not_terminal_id.find(start_symbol) == not_terminal_id.end())
        throw std::invalid_argument("Start symbol is not in not terminal alphabet");
    start_id = not_terminal_id[start_symbol];
    Grammar start;
    start.left = Symbol(kStartId, false);
    start.right = { Symbol(start_id, false) };
    grammar.push_back(start);

}


void EarleyParser::scan() {
    auto& layer = layers[layer_ind];
    auto& next_layer = layers[layer_ind + 1];
    for (auto situation : layer[kTerminal][word[layer_ind]]) {
        situation.point_ind++;
        auto& gr = grammar[situation.grammar_id];
        if (situation.point_ind == gr.right.size())
            next_layer[kNotTerminal][kEmpty].insert(std::move(situation));
        else
            next_layer[gr.right[situation.point_ind].is_terminal][gr.right[situation.point_ind].id].insert(std::move(situation));
    }
}


void EarleyParser::predict(std::unordered_set<Situation, Situation::HashFunction>& new_elements) {
    auto& layer = layers[layer_ind];
    std::vector<Situation> new_situations;
    new_situations.reserve(grammar.size());
    for (uint32_t i = 0; i < grammar.size(); ++i) {
        if (!layer[kNotTerminal][grammar[i].left.id].empty()) {
            Situation situation;
            situation.grammar_id = i;
            situation.left = layer_ind;
            situation.point_ind = 0;
            new_situations.push_back(std::move(situation));
        }
    }
    for (const auto& situation : new_situations) {
        auto& gr = grammar[situation.grammar_id];
        if (grammar[situation.grammar_id].right.size() == 0 && layer[kNotTerminal][kEmpty].find(situation) == layer[kNotTerminal][kEmpty].end()) {
            new_elements.insert(situation);
        }
        else if (grammar[situation.grammar_id].right.size() != 0) {
            auto& layer_to_check = layer[gr.right[situation.point_ind].is_terminal][gr.right[situation.point_ind].id];
            if (layer_to_check.find(situation) == layer_to_check.end()) {
                new_elements.insert(situation);
            }
        }
    }
}


void EarleyParser::complete(std::unordered_set<Situation, Situation::HashFunction>& new_elements, 
                                std::vector<std::vector<uint32_t>>& checked) {
    auto& layer = layers[layer_ind];
    std::vector<Situation> new_situations;
    for (const auto& situation : layer[kNotTerminal][kEmpty]) {
        auto& prev_layer = layers[situation.left];
        uint32_t left_symbol = grammar[situation.grammar_id].left.id;
        checked[situation.left][left_symbol] = 1;
        for (auto situation2 : prev_layer[kNotTerminal][left_symbol]) {
            situation2.point_ind++;
            new_situations.push_back(std::move(situation2));
        }
    }
    for (const auto& situation : new_situations) {
        auto& gr = grammar[situation.grammar_id];
        if (gr.right.size() == situation.point_ind && layer[kNotTerminal][kEmpty].find(situation) == layer[kNotTerminal][kEmpty].end()) {
            new_elements.insert(situation);
        }
        else if (gr.right.size() != situation.point_ind) {
            auto& layer_to_check = layer[gr.right[situation.point_ind].is_terminal][gr.right[situation.point_ind].id];
            if (layer_to_check.find(situation) == layer_to_check.end()) {
                new_elements.insert(situation);
            }
        }
    }
}

void EarleyParser::complete_with_new_elements(std::unordered_set<Situation, Situation::HashFunction>& new_elements, 
                                                std::vector<std::vector<uint32_t>>& checked) {
    auto& layer = layers[layer_ind];
    std::vector<Situation> new_situations;
    for (const auto& situation : new_elements) {
        if (situation.point_ind != grammar[situation.grammar_id].right.size()) {
            continue;
        }
        uint32_t left_symbol = grammar[situation.grammar_id].left.id;
        if (checked[situation.left][left_symbol])
            continue;
        auto& prev_layer = layers[situation.left];
        checked[situation.left][left_symbol] = 1;
        for (auto situation2 : prev_layer[kNotTerminal][left_symbol]) {
            situation2.point_ind++;
            new_situations.push_back(std::move(situation2));
        }
    }
    for (const auto& situation : new_elements) {
        if (situation.point_ind == grammar[situation.grammar_id].right.size()) {
            continue;
        }
        auto& symbol = grammar[situation.grammar_id].right[situation.point_ind];
        if (!symbol.is_terminal && checked[layer_ind][symbol.id]) {
            auto situation2 = situation;
            situation2.point_ind++;
            new_situations.push_back(std::move(situation2));
        }
    }
    for (const auto& situation : new_elements) {
        auto& gr = grammar[situation.grammar_id];
        if (gr.right.size() == situation.point_ind) {
            layer[kNotTerminal][kEmpty].insert(situation);
        }
        else {
            layer[gr.right[situation.point_ind].is_terminal][gr.right[situation.point_ind].id].insert(situation);
        }
    }
    new_elements.clear();
    for (const auto& situation : new_situations) {
        auto& gr = grammar[situation.grammar_id];
        if (gr.right.size() == situation.point_ind && layer[kNotTerminal][kEmpty].find(situation) == layer[kNotTerminal][kEmpty].end()) {
            new_elements.insert(situation);
        }
        else if (gr.right.size() != situation.point_ind) {
            auto& layer_to_check = layer[gr.right[situation.point_ind].is_terminal][gr.right[situation.point_ind].id];
            if (layer_to_check.find(situation) == layer_to_check.end()) {
                new_elements.insert(situation);
            }
        }
    }
}


bool EarleyParser::predict(std::string word_to_predict) {
    word.assign(word_to_predict.size(), {});
    for (uint32_t i = 0; i < word_to_predict.size(); ++i) {
        if (terminal_id.find(word_to_predict[i]) == terminal_id.end())
            throw std::invalid_argument("Symbol not in terminal alphabet");
        word[i] = terminal_id[word_to_predict[i]];
    }
    layers.assign(word.size() + 1, {});
    for (uint32_t i = 0; i < word.size() + 1; ++i) {
        layers[i][kNotTerminal].resize(not_terminal_size);
        layers[i][kTerminal].resize(terminal_size);
    }
    layers[0][kNotTerminal][start_id].emplace(grammar.size() - 1, 0, 0);
    layer_ind = 0;
    std::unordered_set<Situation, Situation::HashFunction> new_elements;
    std::vector<std::vector<uint32_t>> checked(word.size() + 1, std::vector<uint32_t>(not_terminal_size, 0));
    complete(new_elements, checked);
    predict(new_elements);
    while (!new_elements.empty()) {
        complete_with_new_elements(new_elements, checked);
        predict(new_elements);
    }
    while (layer_ind < word.size()) {
        scan();
        layer_ind++;
        checked.assign(word.size() + 1, std::vector<uint32_t>(not_terminal_size, 0));
        complete(new_elements, checked);
        predict(new_elements);
        while (!new_elements.empty()) {
            complete_with_new_elements(new_elements, checked);
            predict(new_elements);
        }
    }
    return layers[word.size()][kNotTerminal][kEmpty].find(Situation(grammar.size() - 1, 0, 1)) != layers[word.size()][kNotTerminal][kEmpty].end();
}


/*void EarleyParser::debug() {
    std::cout << "\n\nDebuging\n";
    for (int i = 0; i <= word.size(); ++i) {
        std::cout << "Layer number: " << i << '\n';
        std::cout << "Not terminal stuff\n";
        for (int j = 0; j < not_terminal_size; ++j) {
            std::cout << "Letter index: " << j << '\n';
            for (auto s : layers[i][kNotTerminal][j]) {
                std::cout << "Grammar: " << s.grammar_id << '\n';
                std::cout << "Left: " << s.left << '\n';
                std::cout << "Point index: " << s.point_ind << '\n';
            }
        }
        std::cout << "Terminal stuff\n";
        for (int j = 0; j < terminal_size; ++j) {
            std::cout << "Letter index: " << j << '\n';
            for (auto s : layers[i][kTerminal][j]) {
                std::cout << "Grammar: " << s.grammar_id << '\n';
                std::cout << "Left: " << s.left << '\n';
                std::cout << "Point index: " << s.point_ind << '\n';
            }
        }
        std::cout << '\n';
    }
    std::cout << std::endl;
}*/
