#include "aoc.hpp"

#include <cctype>
#include <iostream> // E.g., std::istream, std::ostream...
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

auto const NL = "\n";
auto const T = "\t";
auto const NT = "\n\t";

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = std::string;
using Model = aoc::raw::Lines;

Model parse(auto& in) {
  using namespace aoc::parsing;
  Model result{};
  auto input = Splitter{in};
  auto lines = input.lines();
  std::cout << NL << T << lines.size() << " lines";
  for (int i=0;i<lines.size();++i) {
    auto line = lines[i];
    std::cout << NL << T << T << "line[" << i << "]:" << line.size() << " " << std::quoted(line.str());
    result.push_back(line);
  }
  return result;
}

using aoc::grid::Position;
using aoc::grid::Positions;

using TrailHeads = std::map<Position,Positions>;
std::ostream& operator<<(std::ostream& os,TrailHeads const& ths) {
  for (auto const& [start,ends] : ths) {
    os << NL << start;
    for (auto const& end : ends) {
      os << ends;
    }
  }
  return os;
}

Integer to_scores_sum(TrailHeads const& ths) {
  Integer result{};
  std::cout << NL << "to_scores_sum";
  for (auto const& [start,ends] : ths) {
    auto score = ends.size();
    std::cout << NL << T << "score start:" << start << " -> " << ends << " = " << score;
    result += score;
  }
  return result;
}

using aoc::grid::Grid;

Positions to_start_candidates(Grid const& grid) {
  Positions result;
  std::cout << NL << "to_start_candidates";
  for (int row=0;row<grid.height();++row) {
//    std::cout << NL << T;
    for (int col=0;col<grid.width();++col) {
      Position pos{row,col};
      if (grid.on_map(pos)) {
//        std::cout << "{" << *och << "}";
        if (grid.at(pos) == '0') result.push_back(pos);
      }
    }
  }
  return result;
}

using aoc::grid::Path;

Positions find_ends(Position const& start,Grid const& grid,bool for_part_2) {
  std::cout << NL << NL << "find_ends(start:" << start << ")";
  Positions result{};
  std::map<Position,std::vector<Path>> visited{};
  std::stack<Path> to_visit;

  to_visit.push({start});

  while (!to_visit.empty()) {
    Path current = to_visit.top();
    to_visit.pop();
    std::cout << NL << T << "POPPED current:" << current << " left:" << to_visit.size();
    
    auto is_visited = [&visited,for_part_2](Path const& current) {
      bool result{};
      if (for_part_2) {
        auto const& candidates = visited[current.back()];
        auto iter = std::find(candidates.begin(),candidates.end(),current);
        result = (iter != candidates.end()); // This path already travelled
      }
      else {
        result = (visited[current.back()].size() > 0); // this path position already reached
      }
      return result;
    };

    // Skip if already visited
    if (is_visited(current)) {
      std::cout << " SKIP";
      continue;
    }

    visited[current.back()].push_back(current);
    char cell_value = grid.at(current.back());
    
    std::cout << " '" << cell_value << "'";

    if (cell_value == '9') {
      std::cout << " END";
      result.push_back(current.back());
    }
    
    // Explore neighbors (up, down, left, right)
    auto current_pos = current.back();
    std::vector<Position> neighbors = {
        {current_pos.row - 1, current_pos.col}, // Up
        {current_pos.row + 1, current_pos.col}, // Down
        {current_pos.row, current_pos.col - 1}, // Left
        {current_pos.row, current_pos.col + 1}  // Right
    };

    for (const auto& neighbor : neighbors) {
      if (grid.on_map(neighbor)) {
        Path next{current};
        next.push_back(neighbor);
        auto nch = grid.at(neighbor);
        std::cout << NL << T << T << "neighbour:" << next << " '" << nch << "'";
        if (nch != cell_value+1) continue;
        if (is_visited(next)) continue;
        std::cout << " PUSHED";
        to_visit.push(next);
      }
    }
  }
  return result;
}

TrailHeads to_trail_heads(Position const& start,Model const& model,bool for_part_2) {
  TrailHeads result{};
  auto ends = find_ends(start, model,for_part_2);
  result[start] = ends;
  return result;
}

TrailHeads to_trail_heads(Model const& model,bool for_part_2) {
  TrailHeads result{};
  std::cout << NL << "to_trail_heads";
  auto starts = to_start_candidates(model);
  for (auto const& start : starts) {
    std::cout << NL << "processing start:" << start;
    auto ths = to_trail_heads(start, model,for_part_2);
    result[start] = ths[start];
  }
  return result;
}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      auto trail_heads = to_trail_heads(model,false);
      result = std::to_string(to_scores_sum(trail_heads));
    }
    return result;
  }
}

Integer to_ratings_sum(TrailHeads const& ths) {
  Integer result{};
  std::cout << NL << "to_ratings_sum";
  std::cout << " size:" << ths.size();
  std::map<std::pair<Position,Position>,Integer> end_counts{};
  for (auto const& [start,ends] : ths) {
    std::cout << NL << T << "start:" << start;
    for (auto const& end : ends) {
      std::cout << NL << T << T << "end:" << end;
      end_counts[{start,end}]++;
      std::cout << " end_counts[end]:" << end_counts[{start,end}];
    }
  }
  for (auto const& [end,count] : end_counts) {
    result += count;
  }
  return result;
}

namespace part2 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part2";
    if (in) {
      auto model = parse(in);
      auto trail_heads = to_trail_heads(model,true);
      result = std::to_string(to_ratings_sum(trail_heads));
    }
    return result;
  }
}

using Answers = std::vector<std::pair<std::string,std::optional<Result>>>;
int main(int argc, char *argv[]) {
  
  aoc::application app{};
  app.add_solve_for("test0", part1::solve_for,"example.txt");
  app.add_solve_for("test1", part1::solve_for,"example2.txt");
  app.add_solve_for("1", part1::solve_for,"puzzle.txt");
  app.add_solve_for("test2", part2::solve_for,"example.txt");
  app.add_solve_for("test3", part2::solve_for,"example2.txt");
  app.add_solve_for("2", part2::solve_for,"puzzle.txt");
  app.run(argc, argv);
  app.print_result();
  return 0;
  
  /*
  For my input:
      
   ANSWERS
   duration:2ms answer[part test0 in:example.txt] 1
   duration:22ms answer[part test1 in:example2.txt] 36
   duration:353ms answer[part 1 in:puzzle.txt] 733
   duration:4ms answer[part test2 in:example.txt] 16
   duration:19ms answer[part test3 in:example2.txt] 81
   duration:578ms answer[part 2 in:puzzle.txt] 1514

      
   */
  return 0;
}
