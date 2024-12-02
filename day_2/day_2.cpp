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

std::ostream& operator<<(std::ostream& out,std::vector<Result> report) {
  for (auto const& level : report) {
    out << " " << level;
  }
  return out;
}

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

bool is_safe(std::vector<Result> report) {
  std::cout << "\nis_safe(" << report << ")";
  bool result;
  // Report is SAFE if all increasing or decreasing
  // AND the change is at leasts 1 and at most 3
  std::vector<Result> diff{};
  for (int i=0;i<report.size()-1;++i) {
    diff.push_back(report[i+1]-report[i]);
  }
  auto diffs_ok = std::all_of(diff.begin(), diff.end(), [](Result d){return std::abs(d) >= 1 and std::abs(d) <= 3;});
  std::cout << "\n\tdiff:" << diff;
  if (diffs_ok) std::cout << " safe"; else std::cout << " unsafe";
  std::vector<Result> trend{}; // increase +1, decrease -1 or 'flat' 0
  for (int i=0;i<diff.size();++i) {
    if (diff[i]>0) trend.push_back(1);
    else if (diff[i]<0) trend.push_back(-1);
    else trend.push_back(0);
  }
  auto trend_ok = std::abs(std::accumulate(trend.begin(),trend.end(),Result{})) == trend.size();
  std::cout << "\n\ttrend:" << trend;
  if (trend_ok) std::cout << " safe"; else std::cout << " unsafe";

  result = diffs_ok and trend_ok;
  if (result) std::cout << "\n\tSAFE";
  else std::cout << "\n\tUNSAFE";
  return result;
}

namespace part1 {
  Result solve_for(Model& model,auto args) {
    Result result{};
    std::cout << NL << NL << "part1";
    for (auto const& report : model) {
      std::cout << "\nreport" << report;
      if (is_safe(report)) {
        ++result;
        std::cout << " #" << result;
      }
    }
    return result;
  }
}

namespace part2 {

  bool can_be_made_safe(std::vector<Result> report) {
    std::cout << "\ncan_be_made_safe(" << report << ")";
    bool result{};
    // TRAP! If we detect a level that causes the report to indicate UNSAFE
    //       we may fix the report by removing either of the levels in the pair
    //       that causes the inconsistency (diff size or level trend)
    
    // Check if we can make the report indicate SAFE by removing any one level?
    for (int i=0;i<report.size();++i) {
      auto candidate = report;
      candidate.erase(candidate.begin() + i);
      std::cout << "\n\ttry " << candidate;
      if (is_safe(candidate)) {
        std::cout << " MADE SAFE!";
        result = true;
        break;
      }
    }
    if (not result) {
      std::cout << " FAILED TO MAKE SAFE";
    }
    else {
      std::cout << " CAN BE MADE SAFE!";
    }

    return result;
  }

  Result solve_for(Model& model,auto args) {
    Result result{};
    std::cout << NL << NL << "part2";
    int count{};
    for (auto const& report : model) {
      if (can_be_made_safe(report)) {
        ++result;
        std::cout << " #" << result;
      }
    }
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
  std::tuple<char,std::vector<std::string>> args{{},{}};
  auto& [part,files] = args;
  if (argc > 1 and strlen(argv[1])==1) {
    part = argv[1][0];
    for (int i=2;i<argc;++i) {
      files.push_back(argv[i]);
    }
  }
  else {
    part = '*';
  }
  if (files.size()==0) {
    files.push_back("example.txt");
    files.push_back("puzzle.txt");
  }

  constexpr std::string input_folder{"../../"};

  switch (part) {
  case '1': {
    for (auto file : files) {
      file = input_folder + file;
      std::cout << NL << "Part=" << part << " file=" << std::quoted(file);
      std::ifstream in{ file };
      auto model = parse(in);
      auto answer = part1::solve_for(model,args);
      solution[part].push_back({ file,answer });
    }
  } break;
  case '2': {
    for (auto file : files) {
      file = input_folder + file;
      std::cout << NL << "Part=" << part << " file=" << std::quoted(file);
      std::ifstream in{ file };
      auto model = parse(in);
      auto answer = part2::solve_for(model,args);
      solution[part].push_back({ file,answer });
    }
  } break;
  case '*': {
    for (auto file : files) {
      file = input_folder + file;
      std::cout << NL << "Part=" << part << " file=" << std::quoted(file);
      std::ifstream in{ file };
      auto model = parse(in);
      auto answer = part1::solve_for(model,args);
      solution['1'].push_back({ file,answer });
    }
    for (auto file : files) {
      file = input_folder + file;
      std::cout << NL << "Part=" << part << " file=" << std::quoted(file);
      std::ifstream in{ file };
      auto model = parse(in);
      auto answer = part2::solve_for(model,args);
      solution['2'].push_back({ file,answer });
    }
  } break;
  default:
    std::cout << NL << "No part " << part << " only part 1 and 2";
  }

  std::cout << NL << NL << "------------ REPORT----------------";
  for (auto const& [part, answers] : solution) {
    std::cout << NL << "Part " << char(part) << " answers";
    for (auto const& [heading, answer] : answers) {
      if (answer != 0) std::cout << NT << "answer[" << heading << "] " << answer;
      else std::cout << NT << "answer[" << heading << "] " << " NO OPERATION ";
    }
  }
  std::cout << NL << NL;
  return 0;
}
