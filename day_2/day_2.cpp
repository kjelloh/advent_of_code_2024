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

char const* example = R"()";

auto const NL = "\n";
auto const T = "\t";
auto const NT = "\n\t";

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = Integer;
using Answer = std::pair<std::string, Result>;
using Answers = std::vector<Answer>;
using Solution = std::map<int, Answers>; // Puzzle part -> Answers

using Model = std::vector<std::vector<Result>>;

Model parse(auto& in) {
  std::cout << "\n<BEGIN parse>";
  Model result{};
  std::string line{};
  while (std::getline(in,line)) {
    result.push_back({});
    std::cout << "\nLine:" << std::quoted(line);
    std::istringstream ls{line};
    Result level{};
    std::cout << " --> ";
    while (ls >> level) {
      std::cout << " level:" << level;
      result.back().push_back(level);
    }
  }
  std::cout << "\n<END parse>";
  return result;
}

namespace part1 {
  Result solve_for(Model& model,auto args) {
    Result result{};
    std::cout << NL << NL << "part1";
    for (auto const& report : model) {
      std::cout << "\nreport";
      // Report is SAFE if all increasing or decreasing
      // AND the change is at leasts 1 and at most 3
      bool is_safe{true};
      bool is_increasing{report[1] > report[0]};
      for (int i=0;i<report.size()-1;++i) {
        auto diff = report[i+1] - report[i];
        is_safe = is_safe and (is_increasing == (diff > 0)) and (std::abs(diff) >= 1) and (std::abs(diff) <= 3);
        std::cout << "\n\tdiff:" << diff << " is_safe:" << is_safe << " " << report[i] << " " << report[i+1];
        if (not is_safe) break;
      }
      if (is_safe) {
        ++result;
        std::cout << "\tSAFE";
      }
      else {
        std::cout << "\tUNSAFE";
      }
    }
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
  // day_n x y
  std::tuple<int,std::string> args{1,"example.txt"};
  auto& [part,file] = args;
  if (argc > 1 ) {
    part = std::stoi(argv[1]);
    if (argc > 2) {
      file = argv[2];
    }
  }
  constexpr std::string input_folder{"../../"};
  file = input_folder + file;
  std::cout << NL << "Part=" << part << " file=" << std::quoted(file);
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
