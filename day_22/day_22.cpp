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
#include <format>
#include <optional>
#include <regex>
#include <filesystem>

using aoc::raw::NL;
using aoc::raw::T;
using aoc::raw::NT;

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = aoc::raw::Line;
using Model = aoc::raw::Lines;

Model parse(auto& in) {
  using namespace aoc::parsing;
  Model result{};
  auto sections = Splitter{in}.sections();
  for (auto const& [sx,section] : aoc::views::enumerate(sections)) {
    std::cout << NL << "---------- section " << sx << " ----------";
    for (auto const& [lx,line] : aoc::views::enumerate(section)) {
      std::cout << NL << T << T << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
      result.push_back(line);
    }
  }
  return result;
}

using Args = std::vector<std::string>;

namespace test {

  // Adapt to expected for day puzzle
  struct LogEntry {
    bool operator==(LogEntry const& other) const {
      bool result{true};
      return result;
    }
  };

  std::ostream& operator<<(std::ostream& os,LogEntry const& entry) {
    return os;
  }

  using LogEntries = aoc::test::LogEntries<LogEntry>;

  LogEntries parse(auto& doc_in) {
    std::cout << NL << T << "test::parse";
    LogEntries result{};
    using namespace aoc::parsing;
    auto sections = Splitter{doc_in}.same_indent_sections();
    for (auto const& [sx,section] : aoc::views::enumerate(sections)) {
      std::cout << NL << "---------- section " << sx << " ----------";
      for (auto const& [lx,line] : aoc::views::enumerate(section)) {
        std::cout << NL << T << T << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
      }
    }
    return result;
  }

  std::optional<Result> test0(Args args) {
    std::cout << NL << NL << "test0";
    return std::nullopt;
  }

  std::optional<Result> test1(auto& in, auto& doc_in,Args args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "test1";
    if (in) {
      auto model = ::parse(in);
      if (doc_in) {
        auto log = test::parse(doc_in);
      }
    }
    return result;
  }

}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
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
  for (int i=1;i<argc;++i) {
    args.push_back(argv[i]);
  }

  Answers answers{};
  std::vector<std::chrono::time_point<std::chrono::system_clock>> exec_times{};
  exec_times.push_back(std::chrono::system_clock::now());
  std::vector<int> states = {0,111,11,10,211,21,20};
//  std::vector<int> states = {0,111};
  for (auto state : states) {
    switch (state) {
      case 0: {
        answers.push_back({"test0",test::test0(args)});
      } break;
      case 111: {
        auto doc_file = aoc::to_working_dir_path("doc.txt");
        std::ifstream doc_in{doc_file};
        auto file = aoc::to_working_dir_path("example.txt");
        std::ifstream in{file};
        if (in and doc_in) answers.push_back({"Part 1 Test Example vs Log",test::test1(in,doc_in,args)});
        else std::cerr << "\nSORRY, no file " << file << " or doc_file " << doc_file;
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
      case 211: {
        auto doc_file = aoc::to_working_dir_path("doc.txt");
        std::ifstream doc_in{doc_file};
        auto file = aoc::to_working_dir_path("example.txt");
        std::ifstream in{file};
        if (in and doc_in) answers.push_back({"Part 2 Test Example vs Log",test::test1(in,doc_in,args)});
        else std::cerr << "\nSORRY, no file " << file << " or doc_file " << doc_file;
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
