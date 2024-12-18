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

using aoc::grid::Position;
using aoc::grid::Positions;
using aoc::grid::Grid;

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = Integer;
using Model = Positions;

Model parse(auto& in) {
  using namespace aoc::parsing;
  Model result{};
  auto input = Splitter{in};
  auto lines = input.lines();
  std::cout << NL << T << lines.size() << " lines";
  for (int i=0;i<lines.size();++i) {
    auto line = lines[i];
    std::cout << NL << T << T << "line[" << i << "]:" << line.size() << " " << std::quoted(line.str());
    auto const& [x,y] = line.split(',');
    result.push_back({std::stoi(y),std::stoi(x)}); // row = y,col=x
  }
  return result;
}

using Args = std::vector<std::string>;

struct Node {
    Position pos;
    Result cost;
    bool operator>(Node const& other) const { return cost > other.cost; }
};

using aoc::grid::Path;

Path to_best_path(Position const& start, Position const& end,Grid const& grid) {
  Path result{};
  
  std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
  std::map<Position, Result> cost_map;
  std::map<Position, Position> previous;
  std::set<Position> visited;
  auto step_cost = []() {
    return 1;
  };
  
  pq.push({start, 0});
  cost_map[start] = 0;
  
  while (!pq.empty()) {
    Node current = pq.top();
    pq.pop();
    
    if (current.pos == end) {
      break;
    }
    
    if (visited.count(current.pos)) {
      continue;
    }
    visited.insert(current.pos);
    
    for (auto const& next : default_neighbors(current.pos)) {
      if (not grid.on_map(next)) continue;
      if (visited.contains(next)) continue;
      if (grid.at(next) == '#') continue;
      
      auto new_cost = current.cost + step_cost();
      
      // Update if this path is better.
      if (!cost_map.count(next) || new_cost < cost_map[next]) {
        cost_map[next] = new_cost;
        previous[next] = current.pos;
        pq.push({next, new_cost});
      }
    }
  }
  
  // Reconstruct the path.
  for (Position at = end; at != start; at = previous[at]) {
    result.push_back(at);
  }
  result.push_back(start);
  std::reverse(result.begin(), result.end());
  
  return result;
}

namespace test {

  // Adapt to expected for day puzzle
  struct LogEntry {
    Grid unwalked{};
    Grid walked{};
    int best_step_count{-1};
    bool operator==(LogEntry const& other) const {
      bool result{true};
//      result = result and (unwalked == other.unwalked);
//      result = result and (walked == other.walked);
      result = result and (best_step_count == other.best_step_count);
      return result;
    }
  };

  std::ostream& operator<<(std::ostream& os,LogEntry const& entry) {
    std::cout << NL << "log:";
    std::cout << NL << "unwalked:" << entry.unwalked;
    std::cout << NL << "walked:" << entry.walked;
    std::cout << NL << "best_step_count:" << entry.best_step_count;

    return os;
  }

  using LogEntries = aoc::test::LogEntries<LogEntry>;

  LogEntry parse(auto& in) {
    std::cout << NL << T << "test::parse";
    LogEntry result{};
    using namespace aoc::parsing;
    auto sections = Splitter{in}.sections();
    int byte_count{-1};
    Grid unwalked_grid{{}};
    Grid walked_grid{{}};
    int corrupted_locations{-1};
    int best_step_count{-1};
    for (int i=0;i<sections.size();++i) {
      std::cout << NL << "sections[" << i << "]" << std::flush;
      for (auto const& line : sections[i]) {
        std::cout << NL << line.str() << std::flush;
        switch (i) {
          case 0: {
            std::regex pattern{R"(the first (\d+) bytes)"};
            std::smatch match{};
            if (std::regex_search(line.str(),match,pattern)) {
              byte_count = std::stoi(match[1]);
              std::cout << NL << T << "--> byte_count:" << byte_count;
            }
          } break;
          case 1: {
            unwalked_grid.push_back(line.str());
          } break;
          case 2: {
            std::regex cl_pattern1{R"(After just (\d+) bytes)"};
            std::regex sc_pattern{R"(would take (\d+) steps)"};
            std::smatch match{};
            if (std::regex_search(line.str(),match,cl_pattern1)) {
              corrupted_locations = std::stoi(match[1]);
              std::cout << NL << T << "--> corrupted_locations:" << corrupted_locations;
            }
            if (std::regex_search(line.str(),match,sc_pattern)) {
              best_step_count = std::stoi(match[1]);
              std::cout << NL << T << "--> best_step_count:" << best_step_count;
            }
          } break;
          case 3: {
            walked_grid.push_back(line.str());
          } break;
          default: {std::cerr << NL << "Sorry, parsed unexpected sections count " << sections.size();} break;
        }
      }
    }
    result.unwalked = unwalked_grid;
    result.walked = walked_grid;
    result.best_step_count = best_step_count;
    return result;
  }

  std::optional<Result> test0(Args args) {
    std::cout << NL << NL << "test0";
    return std::nullopt;
  }

  std::optional<Result> test1(auto& in, auto& log_in,Args args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "test1";
    if (in) {
      auto model = ::parse(in);
      if (log_in) {
        auto log = test::parse(log_in);
        std::cout << NL << log;
        auto const& unwalked = log.unwalked;
        auto best_path = to_best_path(unwalked.top_left(),unwalked.bottom_right(), unwalked);
        std::cout << NL << "best_path:" << best_path;
        auto best_step_count = best_path.size()-1;
        std::cout << NL << "best_step_count:" << best_step_count;
        auto computed = log.unwalked;
        computed = aoc::grid::to_traced(computed, best_path);
        std::cout << NL << "computed:" << computed;
        if (best_step_count == log.best_step_count) {
          result = best_path.size()-1;
        }
        else {
          std::cout << NL << "FAILED";
        }
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
  for (int i=0;i<argc;++i) {
    args.push_back(argv[i]);
  }

  Answers answers{};
  std::vector<std::chrono::time_point<std::chrono::system_clock>> exec_times{};
  exec_times.push_back(std::chrono::system_clock::now());
  std::vector<int> states = {111};
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
