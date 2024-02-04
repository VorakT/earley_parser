#pragma once
#include <vector>
#include <unordered_set>
#include <string>
#include <array>
#include <map>


const uint32_t kEmpty = 0;
const uint32_t kStartId = 1;
const uint32_t kNotTerminal = 0;
const uint32_t kTerminal = 1;

struct Symbol {
  uint32_t id;
  bool is_terminal;

  Symbol() = default;

  Symbol(uint32_t id, bool is_terminal);
};


struct Grammar {
  Symbol left;
  std::vector<Symbol> right;
};


struct Situation {
  uint32_t grammar_id;
  uint32_t left;
  uint32_t point_ind;

  Situation() = default;

  Situation(uint32_t grammar_id, uint32_t left, uint32_t point_ind);

  struct HashFunction {
    inline std::size_t operator()(const Situation& a) const;
  };

  bool operator==(const Situation& other) const;
};


class EarleyParser {
private:
  std::vector<Grammar> grammar;
  std::vector<uint32_t> word;
  std::vector<std::array<std::vector<std::unordered_set<Situation, Situation::HashFunction>>, 2>> layers;  //layer[0] - not terminal, layer[1] - terminal
  uint32_t layer_ind;
  uint32_t start_id;
  std::map<char, uint32_t> terminal_id, not_terminal_id;
  uint32_t terminal_size, not_terminal_size;


  void scan();

  void predict(std::unordered_set<Situation, Situation::HashFunction>& new_elements);

  void complete(std::unordered_set<Situation, Situation::HashFunction>& new_elements, 
                                std::vector<std::vector<uint32_t>>& checked);

  void complete_with_new_elements(std::unordered_set<Situation, Situation::HashFunction>& new_elements, 
                                                std::vector<std::vector<uint32_t>>& checked);

  void debug();

public:
  void fit(const std::string& not_terminal_alphabet, const std::string& terminal_alphabet, 
            const std::vector<std::string>& given_grammar, char start_symbol);

  bool predict(std::string word_to_predict);
};