#include "aoc.hpp"

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

auto const NL = "\n";
auto const T = "\t";
auto const NT = "\n\t";

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = std::string;

using Model = std::vector<std::vector<Integer>>;

std::ostream& operator<<(std::ostream& out,std::vector<Integer> report) {
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
    Integer level{};
    std::cout << " --> ";
    while (ls >> level) {
      std::cout << " level:" << level;
      result.back().push_back(level);
    }
  }
  std::cout << "\n<END parse>";
  return result;
}

bool is_safe(std::vector<Integer> report) {
  std::cout << "\nis_safe(" << report << ")";
  bool result;
  // Report is SAFE if all increasing or decreasing
  // AND the change is at leasts 1 and at most 3
  std::vector<Integer> diff{};
  for (int i=0;i<report.size()-1;++i) {
    diff.push_back(report[i+1]-report[i]);
  }
  auto diffs_ok = std::all_of(diff.begin(), diff.end(), [](Integer d){return std::abs(d) >= 1 and std::abs(d) <= 3;});
  std::cout << "\n\tdiff:" << diff;
  if (diffs_ok) std::cout << " safe"; else std::cout << " unsafe";
  std::vector<Integer> trend{}; // increase +1, decrease -1 or 'flat' 0
  for (int i=0;i<diff.size();++i) {
    if (diff[i]>0) trend.push_back(1);
    else if (diff[i]<0) trend.push_back(-1);
    else trend.push_back(0);
  }
  auto trend_ok = std::abs(std::accumulate(trend.begin(),trend.end(),Integer{})) == trend.size();
  std::cout << "\n\ttrend:" << trend;
  if (trend_ok) std::cout << " safe"; else std::cout << " unsafe";

  result = diffs_ok and trend_ok;
  if (result) std::cout << "\n\tSAFE";
  else std::cout << "\n\tUNSAFE";
  return result;
}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::cout << NL << NL << "part1";
    auto model = parse(in);
    Integer acc{};
    for (auto const& report : model) {
      std::cout << "\nreport" << report;
      if (is_safe(report)) {
        ++acc;
        std::cout << " #" << acc;
      }
    }
    return std::to_string(acc);
  }
}

namespace part2 {

  bool can_be_made_safe(std::vector<Integer> report) {
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

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::cout << NL << NL << "part2";
    auto model = parse(in);
    Integer acc{};
      for (auto const& report : model) {
        if (can_be_made_safe(report)) {
          ++acc;
          std::cout << " #" << acc;
        }
      }
      return std::to_string(acc);
    }
}

int main(int argc, char *argv[]) {
  aoc::application app{};
  app.add_solve_for("1",part1::solve_for,"example.txt");
  app.add_solve_for("1",part1::solve_for,"puzzle.txt");
  app.add_solve_for("2",part2::solve_for,"example.txt");
  app.add_solve_for("2",part2::solve_for,"puzzle.txt");
  app.run(argc, argv);
  app.print_result();
  /*

   Xcode Debug -O2

   >day_2 -all
   
   For my input:
            
   ANSWERS
   duration:1ms answer[part 1 in:example.txt] 2
   duration:35ms answer[part 1 in:puzzle.txt] 483
   duration:0ms answer[part 2 in:example.txt] 4
   duration:60ms answer[part 2 in:puzzle.txt] 528

   */
  return 0;

}
