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
using Result = Integer;
using Model = aoc::grid::Grid;

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

using Args = std::vector<std::string>;

using aoc::grid::Position;
using aoc::grid::Positions;
using aoc::grid::width;
using aoc::grid::height;

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

Result to_scores_sum(TrailHeads const& ths) {
  Result result{};
  std::cout << NL << "to_scores_sum";
  for (auto const& [start,ends] : ths) {
    auto score = ends.size();
    std::cout << NL << T << "score start:" << start << " -> " << ends << " = " << score;
    result += score;
  }
  return result;
}

Positions to_start_candidates(Model const& model) {
  Positions result;
  std::cout << NL << "to_start_candidates";
  for (int row=0;row<height(model);++row) {
//    std::cout << NL << T;
    for (int col=0;col<width(model);++col) {
      Position pos{row,col};
      if (auto och = at(pos,model)) {
//        std::cout << "{" << *och << "}";
        if (*och == '0') result.push_back(pos);
      }
    }
  }
  return result;
}

using Path = Positions;
//using Visited = std::set<Position>;
using Visited = std::map<Position, std::vector<Path>>;

Positions find_ends(Position const& start,Model const& model,bool for_part_2) {
  std::cout << NL << NL << "find_ends(start:" << start << ")";
  Positions result{};
  Visited visited{};
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
    char cell_value = *at(current.back(),model);
    
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
      if (auto och = at(neighbor,model)) {
        Path next{current};
        next.push_back(neighbor);
        auto nch = *och;
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
      result = to_scores_sum(trail_heads);
    }
    return result;
  }
}

Result to_ratings_sum(TrailHeads const& ths) {
  Result result{};
  std::cout << NL << "to_ratings_sum";
  std::cout << " size:" << ths.size();
  std::map<std::pair<Position,Position>,Result> end_counts{};
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
      result = to_ratings_sum(trail_heads);
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
//  std::vector<int> states = {11,10,21,20};
  std::vector<int> states = {12,10,22,20};
  for (auto state : states) {
    switch (state) {
      case 11: {
        std::filesystem::path file{"../../example.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1 Example",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 12: {
        std::filesystem::path file{"../../example2.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1 Larger Example",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 10: {
        std::filesystem::path file{"../../puzzle.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1     ",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 21: {
        std::filesystem::path file{"../../example.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2 Example",part2::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 22: {
        std::filesystem::path file{"../../example2.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2 Larger Example",part2::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 23: {
        std::filesystem::path file{"../../example3.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2 Example",part2::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 20: {
        std::filesystem::path file{"../../puzzle.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2     ",part2::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      default:{std::cerr << "\nSORRY, no action for state " << state;} break;
    }
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
   duration:13ms answer[Part 1 Larger Example] 36
   duration:158ms answer[Part 1     ] 733
   duration:8ms answer[Part 2 Larger Example] 81
   duration:242ms answer[Part 2     ] 1514
   
   */
  return 0;
}
