#include <cctype>
#include <iostream>
#include <iomanip> // E.g., std::quoted
#include <string>
#include <string_view>
#include <sstream> // E.g., std::istringstream, std::ostringstream
#include <vector>
#include <set>
#include <map>
#include <stack>
#include <queue>
#include <deque>
#include <array>
#include <ranges> // E.g., std::subrange, std::view
#include <utility> // E.g., std::pair,..
#include <algorithm> // E.g., std::find, std::all_of,...
#include <numeric> // E.g., std::accumulate
#include <limits> // E.g., std::numeric_limits
#include <fstream>
// #include <experimental/generator> // supported by visual studio 2022 17.8.2 with project setting/compiler switch /std:c++latest
#include <format>
#include <optional>
#include <regex>

std::pair<std::string_view, std::vector<std::string_view>> split_string_view(std::string_view input, char separator) {
    std::vector<std::string_view> parts;
    size_t start = 0;
    size_t end;

    // Find each substring separated by the separator
    while ((end = input.find(separator, start)) != std::string_view::npos) {
        parts.emplace_back(input.substr(start, end - start));
        start = end + 1;
    }

    // Add the last substring
    parts.emplace_back(input.substr(start));

    // Return the original string view and the vector of parts
    return {input, parts};
}

constexpr int NonMatching = -1; // Constant for non-matching tokens

std::pair<std::string, std::vector<std::string_view>> split_string_regex(const std::string& input, const std::string& regex_pattern) {
    std::vector<std::string_view> parts;
    std::regex regex(regex_pattern);

    // Using the constant NonMatching for clarity
    auto begin = std::sregex_token_iterator(input.begin(), input.end(), regex, NonMatching);
    auto end = std::sregex_token_iterator();

    for (auto it = begin; it != end; ++it) {
        // Create a string_view on each segment based on the original string
        std::string_view part(input.data() + (it->first - input.begin()), it->length());
        parts.emplace_back(part); // Add the string_view to parts
    }

    // Return the original string and the vector of parts
    return {input, parts};
}

std::pair<std::string_view, std::vector<std::string_view>> find_overlapping_matches(std::string_view input, std::string const& regex_pattern) {
    std::vector<std::string_view> matches; // Vector to hold the matches
    auto to_windowed_regex = [](std::string const& regex) -> std::string {
      if (regex.size()>0 and regex[0] != '^') return std::string{"^"} + regex;
      return regex;
    };
    std::regex regex{to_windowed_regex(regex_pattern)};
    std::match_results<std::string_view::const_iterator> match; // Match results holder

    for (int position = 0;position < input.size();++position) {
      if (std::regex_search(input.begin() + position, input.end(), match, regex)) {
        std::println("matched '{}' at position:{}",match.str(),position);
        matches.emplace_back(input.substr(position,match[0].length())); // Store matched substring as string_view
      }
    }
  
    // Return the original input and the vector of matches
    return {input, matches}; // Return original string and matches as string_views
}


char const* example = R"()";

auto const NL = "\n";
auto const T = "\t";
auto const NT = "\n\t";

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = Integer;
using Answer = std::pair<std::string, Result>;
using Answers = std::vector<Answer>;
using Solution = std::map<int, Answers>; // Puzzle part -> Answers

using Model = std::vector<std::string>;

Model parse(auto& in) {
  Model result{};
  std::string line{};
  while (std::getline(in,line)) {
    result.push_back(line);
  }
  return result;
}

namespace part1 {
  Result solve_for(Model& model,auto args) {
    Result result{};
    std::cout << NL << NL << "part1";
    return result;
  }
}

namespace part2 {
  Result solve_for(Model& model,auto args) {
    Result result{};
    std::cout << NL << NL << "part2";
    return result;
  }
}

int main(int argc, char *argv[])
{
  Solution solution{};
  std::cout << NL << "argc : " << argc;
  for (int i = 0; i < argc; ++i) {
    std::cout << NL << "argv[" << i << "] : " << std::quoted(argv[i]);
  }
  // day00 x y
  std::tuple<int,std::string> args{1,"example.txt"};
  auto& [part,file] = args;
  if (argc > 1 ) {
    part = std::stoi(argv[1]);
    if (argc > 2) {
      file = argv[2];
    }
  }
  std::cout << NL << "Part : " << part << " file : " << file;
  std::ifstream in{ file };
  auto model = parse(in);

  switch (part) {
  case 1: {
    auto answer = part1::solve_for(model,args);
    solution[part].push_back({ file,answer });
  } break;
  case 2: {
    auto answer = part2::solve_for(model,args);
    solution[part].push_back({ file,answer });
  } break;
  default:
    std::cout << NL << "No part " << part << " only part 1 and 2";
  }

  std::cout << NL << NL << "------------ REPORT----------------";
  for (auto const& [part, answers] : solution) {
    std::cout << NL << "Part " << part << " answers";
    for (auto const& [heading, answer] : answers) {
      if (answer != 0) std::cout << NT << "answer[" << heading << "] " << answer;
      else std::cout << NT << "answer[" << heading << "] " << " NO OPERATION ";
    }
  }
  std::cout << NL << NL;
  return 0;
}
