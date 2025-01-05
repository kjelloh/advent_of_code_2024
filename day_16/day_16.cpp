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
using aoc::grid::Position;
using aoc::grid::Positions;
using aoc::grid::Direction;
using aoc::grid::Path;

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = std::string;
using Model = Grid;

Model parse(auto& in) {
  using namespace aoc::parsing;
  auto input = Splitter{in};
  auto lines = input.lines();
  Model result{to_raw(lines)};
  return result;
}

using aoc::Args;

std::set<Position> to_junctions(std::set<Position> const& reachable,auto to_neighbours) {
  std::set<Position> result{};
  for (auto const& pos : reachable) {
    int count{};
    for (auto const& adj : to_neighbours(pos)) {
      if (not reachable.contains(adj)) continue;
      ++count;
    }
    if (count > 2) result.insert(pos);
  }
  return result;
}

namespace test {

  struct LogEntry {
    Grid grid;
    Integer score{};
    std::optional<Integer> steps{};
    std::optional<Integer> turns{};
    bool operator==(LogEntry const& other) const {
      bool result{true};
      result = result and grid == other.grid;
      result = result and score == other.score;
      result = result and (steps and other.steps)?steps.value() == other.steps.value():true;
      result = result and (turns and other.turns)?turns.value() == other.turns.value():true;
      return result;
    }
  };

  std::ostream& operator<<(std::ostream& os,LogEntry const& entry) {
    os << " score:" << entry.score;
    os << " steps:";
    if (entry.steps) os << *entry.steps;
    else
      os << "?";
    os << " turns:";
    if (entry.turns) os << *entry.turns;
    else os << "?";
    os << NL << "Grid:" << NL << entry.grid;
    return os;
  }
  using LogEntries = std::vector<LogEntry>;
  std::ostream& operator<<(std::ostream& os,LogEntries const& log) {
    for (auto const& entry : log) {
      os << NL << entry;
    }
    return os;
  }

  struct Outcome {
    LogEntry expected;
    LogEntry deduced;
  };

  std::ostream& operator<<(std::ostream& os,Outcome const& outcome) {
    std::ostringstream expected_os{};
    expected_os << outcome.expected;
    std::ostringstream deduced_os{};
    deduced_os << outcome.deduced;
    aoc::raw::Lines expected_lines{};
    aoc::raw::Lines deduced_lines{};
    std::istringstream expected_is{expected_os.str()};
    std::istringstream deduced_is{deduced_os.str()};
    aoc::raw::Line line{};
    while (std::getline(expected_is,line)) expected_lines.push_back(line);
    while (std::getline(deduced_is,line)) deduced_lines.push_back(line);
    auto max_lines = std::max(expected_lines.size(),deduced_lines.size());
    std::size_t
    last_width{};
    std::cout << NL << "Expected " << T << "Deduced";
    for (int i=0;i<max_lines;++i) {
      std::cout << NL;
      if (i==0) {
        std::cout << NL << T << "Expected:" << expected_lines[i];
        std::cout << NL << T << " Deduced:" << deduced_lines[i];
      }
      else {
        if (i<expected_lines.size()) {
          std::cout << expected_lines[i];
          last_width = expected_lines[i].size();
        }
        else {
          std::cout << std::string(last_width,' ');
        }
        std::cout << T;
        if (i < deduced_lines.size()) {
          std::cout << deduced_lines[i];
        }
      }
    }
    if (outcome.deduced == outcome.expected) {
      std::cout << NL << "SAME OK!";
    }
    else {
      
    }
    return os;
  }

  aoc::parsing::Sections parse_doc(Args const& args) {
    std::cout << NL << T << "parse puzzle doc text";
    aoc::parsing::Sections result{};
    using namespace aoc::parsing;
    std::ifstream doc_in{aoc::to_working_dir_path("doc.txt")};
    auto sections = Splitter{doc_in}.same_indent_sections();
    for (auto const& [sx,section] : aoc::views::enumerate(sections)) {
      std::cout << NL << "---------- section " << sx << " ----------";
      result.push_back(section);
      for (auto const& [lx,line] : aoc::views::enumerate(section)) {
        std::cout << NL << T << T << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
      }
    }

  }

  aoc::raw::Lines to_example(aoc::parsing::Sections const& sections) {
    return {};
  }

  
  LogEntries to_log_entries(aoc::parsing::Sections const& sections,Args const& args) {
    LogEntries result{};
    using namespace aoc::parsing;
    for (int i=0;i<sections.size();++i) {
      auto const& section = sections[i];
      using aoc::raw::operator<<;
      std::cout << NL << std::format("sections[{}]:{}",i,section.size()) << to_raw(section);
    }
    
    if (sections.size()>=2) {
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
          result.push_back({Grid{to_raw(sections[1])},score,steps,turns});
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
    }
    std::cout << NL << result;
    return result;
  }

  using Reachable = std::set<Position>;
  using Segments = std::vector<aoc::grid::Path>;
  Segments to_segments(Position const& start,Reachable const& reachable,Reachable const& junctions) {
    std::cout << NL << "to_segments";
    Segments result{};
    aoc::graph::Graph<Position> graph{reachable};
    std::cout << NL << graph;
    return result;
  }

  struct Node {
      Position pos;
      Integer cost;
      int direction;
      bool operator>(Node const& other) const { return cost > other.cost; }
  };

  Path to_best_path(Position const& start, Position const& end,Grid const& grid) {
    Path result{};
      
      std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
      std::map<Position, Integer> cost_map;
      std::map<Position, Position> previous;
      std::set<Position> visited;
      auto step_cost = [](int prev_dir, int curr_dir) -> Integer {
          return 1 + (prev_dir != -1 && prev_dir != curr_dir ? 1000 : 0);
      };

      pq.push({start, 0, -1});
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

              auto curr_dir = to_direction_index(current.pos, next);
              auto new_cost = current.cost + step_cost(current.direction, curr_dir);

              // Update if this path is better.
              if (!cost_map.count(next) || new_cost < cost_map[next]) {
                  cost_map[next] = new_cost;
                  previous[next] = current.pos;
                  pq.push({next, new_cost, curr_dir});
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

  using Turns = std::vector<std::pair<Position,char>>;
  std::ostream& operator<<(std::ostream& os,Turns const& turns) {
    for (auto const& [pos,ch] : turns) {
      std::cout << NL << T << pos << " " << ch;
    }
    return os;
  }
  Turns to_turns(Path const& path) {
    Turns result{};
    auto start = path.front();
    Position before_start{start.row,start.col-1};
    Direction after = path[1] - path[0];
    Direction before = path[0] - before_start;
    for (int i=1;i<path.size();++i) {
      if (i>1) {
        after = path[i] - path[i-1];
        before = path[i-1] - path[i-2];
      }
      switch (aoc::raw::sign(before.cross(after))) {
        case -1: result.push_back({path[i-1],'R'}); break; // negative (clockwise) 'sweep' as in turn before -> after
        case  0: break; // No turn
        case  1: result.push_back({path[i-1],'L'}); break; // positive (counter clockwise) 'sweep' as in turn before -> after
      }
    }
    return result;
  }

  Integer to_score(Path const& best_path,Turns const& turns) {
    std::cout << NL << "to_score";
    Integer result{};
    auto turn_count = turns.size();
    auto step_count = (best_path.size()-1); // steps one less than count
    result = turn_count*1000 + step_count;
    return result;
  }


  std::optional<Result> test1(Model const& model,LogEntries const& log, Args args) {
    std::cout << NL << NL << "test1";
    
    auto start = model.find('S');
    auto end = model.find('E');
    
    if (log.size() == 1) {
      // Extract the exposed solution path from log
      auto expected = log[0];
//      auto deduced_path = to_marked_path(start, expected.grid, [](char ch){
//        std::set<char> const PATH_MARK{'S','<','^','>','v','E'};
//        return PATH_MARK.contains(ch);
//      });
//      std::cout << NL << "Deduced path:" << deduced_path;
//      auto deduced_turns = to_turns(deduced_path);
//      std::cout << NL << "Deduced turns:" << deduced_turns;
//      auto deduced_score = to_score(deduced_path,deduced_turns);
//      std::cout << NL << "deduced score:" << deduced_score;
      
      auto best_path = to_best_path(start, end, model);
      std::cout << NL << "best_path:" << best_path;
      auto best_turns = to_turns(best_path);
      using test::operator<<;
      std::cout << NL << "best_turns:" << best_turns;
      auto best_score = to_score(best_path,best_turns);
      std::cout << NL << "best_score:" << best_score;

      Grid grid = model;
      aoc::grid::to_dir_traced(grid, best_path);
//      for (int i=1;i<best_path.size()-1;++i) {
//        auto to = best_path[i+1];
//        auto from = best_path[i];
//        switch (to_direction_index(from, to)) {
//          case 0: grid.at(from) = '>'; break;
//          case 1: grid.at(from) = 'v'; break;
//          case 2: grid.at(from) = '<'; break;
//          case 3: grid.at(from) = '^'; break;
//          case -1: break;
//        }
//      }
      LogEntry best{grid,best_score,best_path.size()-1
        ,best_turns.size()};
      Outcome outcome{expected,best};
      std::cout << NL << outcome;
    }
    else {
      std::cout << NL << "UNEXPECTED: Read Log has more that one entry, entires:" << log.size();
    }
    return std::nullopt;
  }


  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::ostringstream response{};
    std::cout << NL << NL << "test";
    if (in) {
      auto model = parse(in);
      auto doc = parse_doc(args);
      auto example_lines = to_example(doc);
      if (example_lines.size()>0) {
        if (args.options.contains("-to_example")) {
          auto example_file = aoc::to_working_dir_path("example.txt");
          if (aoc::raw::write_to_file(example_file, example_lines)) {
            response << "Created " << example_file;
          }
          else {
            response << "Sorry, failed to create file " << example_file;
          }
          return response.str();
        }
        else {
          std::ostringstream oss{};
          aoc::raw::write_to(oss, example_lines);
          std::istringstream example_in{oss.str()};
          auto example_model = ::parse(example_in);
          std::cout << NL << "example_model:" << example_model;
          auto model = ::parse(in);
          auto log = test::to_log_entries(doc, args);
          return test1(model,log,args);

        }
      }
    }
    if (response.str().size()>0) return response.str();
    else return std::nullopt;
  }

}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::ostringstream response{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      auto start = model.find('S');
      auto end = model.find('E');
      auto best_path = test::to_best_path(start, end, model);
      std::cout << NL << "best_path:" << best_path;
      auto best_turns = test::to_turns(best_path);
      using test::operator<<;
      std::cout << NL << "best_turns:" << best_turns;
      auto best_score = test::to_score(best_path,best_turns);
      std::cout << NL << "best_score:" << best_score;

      Grid grid = model;
      for (int i=1;i<best_path.size()-1;++i) {
        auto to = best_path[i+1];
        auto from = best_path[i];
        switch (to_direction_index(from, to)) {
          case 0: grid.at(from) = '>'; break;
          case 1: grid.at(from) = 'v'; break;
          case 2: grid.at(from) = '<'; break;
          case 3: grid.at(from) = '^'; break;
          case -1: break;
        }
      }
      std::cout << NL << grid;
      
      response << best_score;
    }
    if (response.str().size()>0) return response.str();
    return std::nullopt;
  }

}
namespace part2 {

  using Paths = std::vector<Path>;
  using Pose = std::pair<Position,Direction>;

  struct Node {
    Pose pose;
    Integer cost;
    bool operator>(Node const& other) const { return cost > other.cost; }
  };

  Paths to_best_paths(Position const& start, Position const& end,Grid const& grid) {
    Paths result{};
    
    // Priority queue sorted on lowest cost
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
    std::map<Pose, Integer> lowest_cost_of;
    std::map<Pose, std::set<Pose>> previous_of;
    auto lowest_cost_to_end_so_far = std::numeric_limits<Integer>::max();

    auto step_cost = [](Pose prev, Pose curr) -> Integer {
      return (prev.first != curr.first)?1:0 + (prev.second != curr.second ? 1000 : 0);
    };
    
    Direction const EAST{0,1};
    Pose begin{start,EAST};
    pq.push({begin,0});
    lowest_cost_of[begin] = 0;
    
    while (!pq.empty()) {
      
    }
    return result;
  }

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part2";
    if (in) {
      auto model = parse(in);
      auto start = model.find('S');
      auto end = model.find('E');
      auto best_paths = part2::to_best_paths(start, end, model);
      std::set<Position> visited{};
      for (auto const& path : best_paths) {
        std::ranges::copy(path,std::inserter(visited, visited.begin()));
      }
      {
        auto marked = model;
        aoc::grid::to_filled(marked, visited);
        std::cout << NL << marked;
      }
      result = std::to_string(visited.size());

    }
    return result;
  }
}

using Answers = std::vector<std::pair<std::string,std::optional<Result>>>;

std::vector<Args> to_requests(Args const& args) {
  std::vector<Args> result{};
  result.push_back(args); // No fancy for now
  return result;
}

int main(int argc, char *argv[]) {
  Args user_args{};
  
  // Override by any user input
  for (int i=1;i<argc;++i) {
    user_args.arg["file"] = "example.txt";
    std::string token{argv[i]};
    if (token.starts_with("-")) user_args.options.insert(token);
    else {
      // assume options before <part> and <file>
      auto non_option_index = i - user_args.options.size(); // <part> <file>
      switch (non_option_index) {
        case 1: user_args.arg["part"] = token; break;
        case 2: user_args.arg["file"] = token; break;
        default: std::cerr << NL << "Unknown argument " << std::quoted(token);
      }
    }
  }
  
  auto requests = to_requests(user_args);
  
  if (not user_args or user_args.options.contains("-all")) {
    requests.clear();

    std::vector<std::string> parts = {"test", "1", "2"};
    std::vector<std::string> files = {"example.txt", "puzzle.txt"};
    
    for (const auto& [part, file] : aoc::algo::cartesian_product(parts, files)) {
      Args args;
      args.arg["part"] = part;
      args.arg["file"] = file;
      requests.push_back(args);
    }
  }

  Answers answers{};
  std::vector<std::chrono::time_point<std::chrono::system_clock>> exec_times{};
  exec_times.push_back(std::chrono::system_clock::now());
  for (auto request : requests) {
    auto part = request.arg["part"];
    auto file = aoc::to_working_dir_path(request.arg["file"]);
    std::cout << NL << "Using part:" << part << " file:" << file;
    std::ifstream in{file};
    if (in) {
      if (part=="1") {
        answers.push_back({std::format("part{} {}",part,file.filename().string()),part1::solve_for(in,request)});
      }
      else if (part=="2") {
        answers.push_back({std::format("part{} {}",part,file.filename().string()),part2::solve_for(in,request)});
      }
      else if (part.starts_with("test")) {
        answers.push_back({std::format("{} {}",part,file.filename().string()),test::solve_for(in,request)});
      }
    }
    else std::cerr << "\nSORRY, no file " << file;
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

   Xcode Debug -O2

   >day_16 -all

   ANSWERS
   duration:4ms answer[Part 1 Test Example vs Log] NO OPERATION
   duration:4ms answer[Part 1 Test Example vs Log] NO OPERATION
   duration:1ms answer[Part 1 Example] 7036
   duration:111ms answer[Part 1     ] 95476

   */
  return 0;
}
