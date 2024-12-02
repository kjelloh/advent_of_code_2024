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
  int count{};
  std::string line{};
  while (std::getline(in,line)) {
    result.push_back({});
    std::cout << "\nLine[" << count++ << "]:" << std::quoted(line);
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

  std::ostream& operator<<(std::ostream& out,std::vector<Result> report) {
    for (auto const& level : report) {
      out << " " << level;
    }
    return out;
  }

  std::optional<int> unsafe_index_in(std::vector<Result> report) {
    std::cout << "\nunsafe_index_in(" << report << ")";
    std::optional<int> result{};
    // Report is SAFE if all increasing or decreasing
    // AND the change is at leasts 1 and at most 3
    bool is_safe{true};
    bool is_increasing{report[1] > report[0]};
    for (int i=0;i<report.size()-1;++i) {
      auto diff = report[i+1] - report[i];
      is_safe = is_safe and (is_increasing == (diff > 0)) and (std::abs(diff) >= 1) and (std::abs(diff) <= 3);
      std::cout << "\n\tdiff:" << diff << " is_safe:" << is_safe << " " << report[i] << " " << report[i+1];
      if (not is_safe) {
        std::cout << " unsafe_index:" << i+1;
        return i+1;
      }
    }
    return result;
  }

  bool is_safe(std::vector<Result> report) {
    bool result;
    std::vector<Result> diff{};
    for (int i=0;i<report.size()-1;++i) {
      diff.push_back(report[i+1]-report[i]);
    }
    std::cout << "\n\tdiff:" << diff;
    std::vector<Result> trend{};
    for (int i=0;i<diff.size();++i) {
      if (diff[i]>0) trend.push_back(1);
      else if (diff[i]<0) trend.push_back(-1);
      else trend.push_back(0);
    }
    std::cout << "\n\ttrend:" << trend;

    result =     std::all_of(diff.begin(), diff.end(), [](Result d){
                   return std::abs(d) >= 1 and std::abs(d) <= 3;
                 })
             and std::abs(std::accumulate(trend.begin(),trend.end(),Result{})) == trend.size();
    return result;
  }

  bool can_be_made_safe(std::vector<Result> report) {
    bool result;
    // TRAP! If we detect a level that causes the report to indicate UNSAFE
    //       we may fix the report by removing either of the levels in the pair
    //       that causes the inconsistency (diff size or level trend)
    
    // Check if we can make the report indicate SAFE by removing any one level?
    for (int i=0;i<report.size();++i) {
      auto candidate = report;
      candidate.erase(candidate.begin() + i);
      if (is_safe(candidate)) {
        result = true;
        break;
      }
    }
    return result;
  }

  Result solve_for(Model& model,auto args) {
    Result result{};
    std::cout << NL << NL << "part2";
    int count{};
    for (auto const& candidate : model) {
      auto report = candidate;
      std::cout << "\nreport[" << count++ << "]" << report;
      if (auto unsafe_index = unsafe_index_in(report)) {
        std::cout << " REMOVED report[" << *unsafe_index << "]:" << report[*unsafe_index];
        report.erase(report.begin() + *unsafe_index);
      }
      
      if (auto unsafe_index = unsafe_index_in(report); not unsafe_index) {
        ++result;
        std::cout << "\n\tSAFE no:" << result;
      }
      else {
        std::cout << "\n\tUNSAFE";
        if (can_be_made_safe(candidate)) {
          ++ result;
          std::cout << " ACTUALLY SAFE no:" << result;
        }
      }

      
    }
    return result; // 515 is too low
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