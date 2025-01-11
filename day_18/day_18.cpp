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
using Result = std::string;
using Model = Positions;

template <>
struct std::formatter<Model> : std::formatter<std::string> {
  template<class FmtContext>
  FmtContext::iterator format(Model const& model, FmtContext& ctx) const {
    std::ostringstream oss{};
    oss << model;
    std::format_to(ctx.out(),"\n{}",oss.str());
    return ctx.out();
  }
};


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

struct Node {
    Position pos;
    Integer cost;
    bool operator>(Node const& other) const { return cost > other.cost; }
};

using aoc::grid::Path;

Path to_best_path(Position const& start, Position const& end,Grid const& grid) {
  Path result{};
  
  std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
  std::map<Position, Integer> cost_map;
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
    
    for (auto const& next : aoc::grid::to_ortho_neighbours(current.pos)) {
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

Grid to_unwalked(Model const& model,int byte_count) {
  int width{7};
  int height{7};
  if (byte_count == 1024) {
    width = 71;
    height = 71;
  }
  std::cout << NL << "to_unwalked() width:" << width << " heigh:" << height << " byte_count:" << byte_count;
  Grid result{aoc::raw::Lines(height,aoc::raw::Line(width,'.'))};
  for (int i=0;i<byte_count;++i) {
    auto pos = model[i];
    result.at(pos) = '#';
  }
  return result;
}

namespace test {

  // Adapt to expected for day puzzle
  struct Expected {
    int byte_count{-1};
    Grid unwalked{};
    Grid walked{};
    int best_step_count{-1};
  };

  std::ostream& operator<<(std::ostream& os,Expected const& entry) {
    std::cout << NL << "expected:";
    std::cout << NL << "unwalked:" << entry.unwalked;
    std::cout << NL << "byte_count:" << entry.byte_count;
    std::cout << NL << "walked:" << entry.walked;
    std::cout << NL << "best_step_count:" << entry.best_step_count;
    return os;
  }

//  using Expecteds = aoc::test::Expecteds<Expected>;

  std::vector<aoc::raw::Lines> to_examples(aoc::parsing::Sections const& sections) {
    std::vector<aoc::raw::Lines> result{};
    result.push_back({});
    result.back().append_range(aoc::parsing::to_raw(sections[11]));
    return result;
  }

  aoc::application::ExpectedTeBool test1(std::optional<aoc::parsing::Sections> const& opt_sections,Args args) {
    if (not opt_sections) return std::unexpected("doc.txt required");
    auto const& sections = *opt_sections;
    std::cout << NL << T << "sections ok";
    auto examples = to_examples(sections);
    if (examples.size()==0) return std::unexpected("example from doc.txt required");
    std::cout << NL << T << "examples ok";
    auto example_in = aoc::test::to_example_in(examples[0]);
    auto example_model = parse(example_in);
    std::cout << NL << std::format("\n{}",example_model);
        
    Expected expected{};
    expected.byte_count = -1;
    expected.best_step_count = -1;
    {
      //---------- section 15 ----------
      //    line[0]:71 "   In the above example, if you were to draw the memory space after the"
      //    line[1]:72 "   first 12 bytes have fallen (using . for safe and # for corrupted), it"
      //    line[2]:24 "   would look like this:"
      std::string entry = aoc::parsing::to_line(sections[15]).str();
      std::cout << NL << T << std::quoted(entry);
      std::regex pattern{R"(the first (\d+) bytes)"};
      std::smatch match{};
      if (std::regex_search(entry,match,pattern)) {
        expected.byte_count = std::stoi(match[1]);
        std::cout << " --> byte_count:" << expected.byte_count;
      }
    }
    {
      expected.unwalked.base() = aoc::parsing::to_raw(sections[16]);
    }
    {
      //---------- section 17 ----------
      //    line[0]:72 "   You can take steps up, down, left, or right. After just 12 bytes have"
      //    line[1]:71 "   corrupted locations in your memory space, the shortest path from the"
      //    line[2]:72 "   top left corner to the exit would take 22 steps. Here (marked with O)"
      //    line[3]:20 "   is one such path:"
      std::string entry = aoc::parsing::to_line(sections[17
                                                         ]).str();
      std::cout << NL << T << std::quoted(entry);
      std::regex sc_pattern{R"(would take (\d+) steps)"};
      std::smatch match{};
      if (std::regex_search(entry,match,sc_pattern)) {
        expected.best_step_count = std::stoi(match[1]);
        std::cout << " --> best_step_count:" << expected.best_step_count;
      }
    }
    {
      expected.walked.base() = aoc::parsing::to_raw(sections[18]);
    }

    
    std::cout << NL << expected;
    auto const& unwalked = expected.unwalked;
    auto best_path = to_best_path(unwalked.top_left(),unwalked.bottom_right(), unwalked);
    std::cout << NL << "best_path:" << best_path;
    auto best_step_count = best_path.size()-1;
    std::cout << NL << "best_step_count:" << best_step_count;
    auto computed = to_unwalked(example_model,expected.byte_count);
    computed = aoc::grid::to_filled(computed, best_path);
    std::cout << NL << "computed:" << computed;
    return (best_step_count == expected.best_step_count);
  }

} // namespace test

namespace common {
  struct Solution {
    int byte_count;
    Grid unwalked;
    Path best_path;
    Integer best_step_count;
  };
  std::optional<Solution> solve_for(Model const& model,Args args) {
    std::optional<Solution> result{};
    std::cout << NL << NL << "common::solve_for";
    int byte_count{12};
    if (model.size()>25) byte_count = 1024;
    if (args.options.size()==1) byte_count = std::stoi(args.options.begin()->substr(1));
    auto unwalked = to_unwalked(model,byte_count);
    auto best_path = to_best_path(unwalked.top_left(),unwalked.bottom_right(), unwalked);
    std::cout << NL << "best_path:" << best_path;
    auto best_step_count = best_path.size()-1;
    std::cout << NL << "best_step_count:" << best_step_count;
    auto computed = unwalked;
    computed = aoc::grid::to_filled(computed, best_path);
    std::cout << NL << "computed:" << computed;
    result = Solution(byte_count,unwalked,best_path,best_step_count);
    return result;
  }

}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      auto solution = common::solve_for(model, args);
      if (solution) {
        result = std::to_string(solution->best_step_count);
      }
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
      auto solution = common::solve_for(model, args);
      if (solution) {
        auto unwalked = solution->unwalked;
        auto start = unwalked.top_left();
        auto end = unwalked.bottom_right();
        std::cout << NL;
        for (int i=solution->byte_count;i<model.size();++i) {
          if (i % 32 == 0) std::cout << '>' << i << ".." << std::flush;
          auto pos = model[i];
          unwalked.at(pos) = '#';
          auto seen = aoc::grid::to_flood_fill(unwalked, start);
          if (not seen.contains(end)) {
            std::cout << NL << "FOUND: byte[" << i << "] at " << pos << " blocks reach of end";
            std::cout << NL << aoc::grid::to_filled(unwalked,seen);
            std::cout << NL << "Finally blcoked at " << pos;
            result = std::format("{},{}",pos.col,pos.row); // col/x, row/y
            break;
          }
          
        }
      }
    }
    return result;
  }
}

int main(int argc, char *argv[]) {
  aoc::application app{};
  app.add_to_examples(test::to_examples);
  app.add_test("test1", test::test1);
  app.add_solve_for("1",part1::solve_for,"example.txt");
  app.add_solve_for("1",part1::solve_for,"puzzle.txt");
  app.add_solve_for("2",part2::solve_for,"example.txt");
  app.add_solve_for("2",part2::solve_for,"puzzle.txt");
  app.run(argc, argv);
  app.print_result();
  /*
   
   Xcode Debug -O2

   >day_18 -all
      
   ANSWERS
   duration:2ms answer[part:"test1"] PASSED
   duration:0ms answer[part 1 in:example.txt] 22
   duration:73ms answer[part 1 in:puzzle.txt] 252
   duration:1ms answer[part 2 in:example.txt] 6,1
   duration:21554ms answer[part 2 in:puzzle.txt] 5,60
   
   */

}
