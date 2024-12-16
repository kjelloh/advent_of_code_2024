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
#include <format>
#include <optional>
#include <regex>

using aoc::raw::NL;
using aoc::raw::T;
using aoc::raw::NT;
using aoc::grid::Grid;

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = Integer;
using Model = Grid;

Model parse(auto& in) {
  using namespace aoc::parsing;
  auto input = Splitter{in};
  auto lines = input.lines();
  Model result{to_raw(lines)};
  return result;
}

using Args = std::vector<std::string>;

namespace test {

  struct LogEntry {
    Grid grid;
    Result score{};
    std::optional<Result> steps{};
    std::optional<Result> turns{};
  };

  std::ostream& operator<<(std::ostream& os,LogEntry const& entry) {
    os << "Exptected:" << " score:" << entry.score;
    os << " steps:";
    if (entry.steps) os << *entry.steps;
    else os << "?";
    os << " turns:";
    if (entry.turns) os << *entry.turns;
    else os << "?";
    return os;
  }
  using LogEntries = std::vector<LogEntry>;
  std::ostream& operator<<(std::ostream& os,LogEntries const& log) {
    for (auto const& entry : log) {
      os << NL << entry;
    }
    return os;
  }

  LogEntries parse(auto& in) {
    std::cout << NL << T << "test::parse";
    LogEntries result{};
    using namespace aoc::parsing;
    auto input = Splitter{in};
    auto sections = input.sections();
    // example: "a score of only 7036"
    // example: "36 steps"
    // example: "turning 90 degrees a total of 7 times"
    
    // example2: "the best paths cost 11048 points"
    
    auto line = sections[0][0];
    std::cout << NL << T << line.str();
    {
      auto groups = line.groups("(\\d+)\\D+?(\\d+)\\D+?(\\d+)\\D+?(\\d+)");
      if (groups.size()==4) {
        auto score = std::stoi(groups[0]);
        std::cout << " --> score:" << score;
        auto steps = std::stoi(groups[1]);
        std::cout << " --> steps:" << steps;
        auto turns = std::stoi(groups[3]);
        std::cout << " --> turns:" << turns;
        result.push_back({to_raw(sections[1]),score,steps,turns});
      }
    }
    if (result.size()==0){
      auto groups = line.groups("(\\d+)");
      if (groups.size()==1) {
        auto score = std::stoi(groups[0]);
        std::cout << " --> score:" << score;
        result.push_back({to_raw(sections[1]),score});
      }

    }
    std::cout << NL << result;
    return result;
  }

  std::optional<Result> test0(Args args) {
    std::cout << NL << NL << "test0";
    return std::nullopt;
  }

  std::optional<Result> test1(auto& in, auto& log_in,Args args) {
    std::cout << NL << NL << "test1";
    auto model = ::parse(in);
    auto log = test::parse(log_in);
    return std::nullopt;
  }

}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      std::cout << NL << model;
    }
    return result;
  }
}

namespace part2 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part2";
    if (in) {
      auto model = parse(in);
    }
    return result;
  }
}

using Answers = std::vector<std::pair<std::string,std::optional<Result>>>;
int main(int argc, char *argv[]) {
  Args args{};
  for (int i=0;i<argc;++i) {
    args.push_back(argv[i]);
  }

  Answers answers{};
  std::vector<std::chrono::time_point<std::chrono::system_clock>> exec_times{};
  exec_times.push_back(std::chrono::system_clock::now());
  std::vector<int> states = {0,111,112,11};
  for (auto state : states) {
    switch (state) {
      case 0: {
        answers.push_back({"test0",test::test0(args)});
      } break;
      case 111: {
        auto log_file = aoc::to_working_dir_path("example.log");
        std::ifstream log_in{log_file};
        auto file = aoc::to_working_dir_path("example.txt");
        std::ifstream in{file};
        if (in and log_in) answers.push_back({"Part 1 Test Example vs Log",test::test1(in,log_in,args)});
        else std::cerr << "\nSORRY, no file " << file << " or log_file " << log_file;
      } break;
      case 112: {
        auto log_file = aoc::to_working_dir_path("example2.log");
        std::ifstream log_in{log_file};
        auto file = aoc::to_working_dir_path("example2.txt");
        std::ifstream in{file};
        if (in and log_in) answers.push_back({"Part 1 Test Example vs Log",test::test1(in,log_in,args)});
        else std::cerr << "\nSORRY, no file " << file << " or log_file " << log_file;
      } break;
      case 11: {
        auto file = aoc::to_working_dir_path("example.txt");
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1 Example",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
      } break;
      case 10: {
        auto file = aoc::to_working_dir_path("puzzle.txt");
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1     ",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
      } break;
      case 21: {
        auto file = aoc::to_working_dir_path("example.txt");
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2 Example",part2::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
      } break;
      case 20: {
        auto file = aoc::to_working_dir_path("puzzle.txt");
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2     ",part2::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
      } break;
      default:{std::cerr << "\nSORRY, no action for state " << state;} break;
    }
    exec_times.push_back(std::chrono::system_clock::now());
  }
  
  std::cout << "\n\nANSWERS";
  for (int i=0;i<answers.size();++i) {
    std::cout << "\nduration:" << std::chrono::duration_cast<std::chrono::milliseconds>(exec_times[i+1] - exec_times[i]).count() << "ms";
    std::cout << " answer[" << answers[i].first << "] ";
    if (answers[i].second) std::cout << *answers[i].second;
    else std::cout << "NO OPERATION";
  }
  std::cout << "\n";
  /*
   For my input:

   ANSWERS
   ...
      
  */
  return 0;
}
