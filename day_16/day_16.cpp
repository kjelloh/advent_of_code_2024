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

template <>
struct std::formatter<Model> : std::formatter<std::string> {
  template<class FmtContext>
  FmtContext::iterator format(Model const& model, FmtContext& ctx) const {
    std::ostringstream oss{};
    using aoc::grid::operator<<;
    oss << model;
    std::format_to(ctx.out(),"\n{}",oss.str());
    return ctx.out();
  }
};

Model parse(auto& in) {
  using namespace aoc::parsing;
  auto input = Splitter{in};
  auto lines = input.lines();
  Model result{to_raw(lines)};
  return result;
}

using aoc::Args;

namespace test {

  struct Expected {
    Grid grid;
    Integer score{};
    std::optional<Integer> steps{};
    std::optional<Integer> turns{};
    bool operator==(Expected const& other) const {
      bool result{true};
      result = result and (grid == other.grid);
      result = result and (score == other.score);
      result = result and (steps and other.steps)?steps.value() == other.steps.value():true;
      result = result and (turns and other.turns)?turns.value() == other.turns.value():true;
      return result;
    }
  };

  std::ostream& operator<<(std::ostream& os,Expected const& entry) {
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
  using Expecteds = std::vector<Expected>;
  std::ostream& operator<<(std::ostream& os,Expecteds const& log) {
    for (auto const& entry : log) {
      os << NL << entry;
    }
    return os;
  }

  struct Outcome {
    Expected expected;
    Expected deduced;
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
    os << NL << NL << T << "Expected " << T << "Deduced";
    for (int i=0;i<max_lines;++i) {
      os << NL;
      if (i==0) {
        os << NL << T << "Expected:" << expected_lines[i];
        os << NL << T << "Deduced:" << deduced_lines[i];
      }
      else {
        if (i<expected_lines.size()) {
          os << expected_lines[i];
          last_width = expected_lines[i].size();
        }
        else {
          os << std::string(last_width,' ');
        }
        os << T;
        if (i < deduced_lines.size()) {
          os << deduced_lines[i];
        }
      }
    }
    if (outcome.deduced == outcome.expected) {
      os << NL << "SAME OK!";
    }
    else {
      os << NL << "DIFFERS!";
    }
    return os;
  }

  std::vector<aoc::raw::Lines> to_examples(aoc::parsing::Sections const& sections) {
    std::vector<aoc::raw::Lines> result{};
    result.push_back(aoc::parsing::to_raw(sections[11]));
    result.push_back(aoc::parsing::to_raw(sections[15]));
    return result;
  }
  
  Expecteds to_expecteds(aoc::parsing::Sections const& sections,auto config_ix,Args const& args) {
    Expecteds result{};
        
    if (config_ix == 0) {
      // config_ix 0
      //---------- section 12 ----------
      //    line[0]:69 "   There are many paths through this maze, but taking any of the best"
      //    line[1]:73 "   paths would incur a score of only 7036. This can be achieved by taking"
      //    line[2]:73 "   a total of 36 steps forward and turning 90 degrees a total of 7 times:"
      auto lines = sections[12];
      aoc::parsing::Line line{""};
      for (auto const& s : lines) line.str() += s;
      std::cout << NL << NL << std::quoted(line.str());
      auto groups = line.groups("(\\d+)\\D+?(\\d+)\\D+?(\\d+)\\D+?(\\d+)");
      if (groups.size()==4) {
        auto score = std::stoi(groups[0]);
        std::cout << NL << T << " --> score:" << score;
        auto steps = std::stoi(groups[1]);
        std::cout << NL << T <<  " --> steps:" << steps;
        auto turns = std::stoi(groups[3]);
        std::cout << NL << T << " --> turns:" << turns;
        result.push_back({Grid{to_raw(sections[13])},score,steps,turns});
      }
    }
    else if (config_ix == 1) {
      // config_ix 1
      //---------- section 16 ----------
      //    line[0]:74 "   In this maze, the best paths cost 11048 points; following one such path"
      //    line[1]:24 "   would look like this:"
      // ...
      auto lines = sections[16];
      aoc::parsing::Line line{""};
      for (auto const& s : lines) line.str() += s;
      std::cout << NL << NL << std::quoted(line.str());
      auto groups = line.groups("(\\d+)");
      if (groups.size()==1) {
        auto score = std::stoi(groups[0]);
        std::cout << NL << T << " --> score:" << score;
        result.push_back({to_raw(sections[17]),score});
      }
    }
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
      os << NL << T << pos << " " << ch;
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
    Integer result{};
    auto turn_count = turns.size();
    auto step_count = (best_path.size()-1); // steps one less than count
    result = turn_count*1000 + step_count;
    return result;
  }

  aoc::application::ExpectedTeBool test1(std::optional<aoc::parsing::Sections> const& opt_sections,Args args) {
    std::cout << NL << "test0";
    if (not opt_sections) return std::unexpected("doc.txt required");
    auto const& sections = *opt_sections;
    std::cout << NL << T << "sections ok";
    auto examples = to_examples(sections);
    if (examples.size()==0) return std::unexpected("example from doc.txt required");
    std::cout << NL << T << "examples ok";
    auto example_in = aoc::test::to_example_in(examples[0]);
    auto example_model = parse(example_in);
    std::cout << NL << std::format("\n{}",example_model);

    // return test result here
    auto start = example_model.find('S');
    auto end = example_model.find('E');
    auto expecteds = to_expecteds(sections,0,args);
    if (expecteds.size()==0) return std::unexpected("No expecteds");
    auto const&  expected = expecteds[0];
    
    auto deduced_path = aoc::doc::to_marked_path(start, expected.grid, [](char ch){
      std::set<char> const PATH_MARK{'S','<','^','>','v','E'};
      return PATH_MARK.contains(ch);
    });
    std::cout << NL << "Deduced path:" << deduced_path;
    auto deduced_turns = to_turns(deduced_path);
    std::cout << NL << "Deduced turns:" << deduced_turns;
    auto deduced_score = to_score(deduced_path,deduced_turns);
    std::cout << NL << "deduced score:" << deduced_score;
    
    auto best_path = to_best_path(start, end, example_model);
    std::cout << NL << "best_path:" << best_path;
    auto best_turns = to_turns(best_path);
    using test::operator<<;
    std::cout << NL << "best_turns:" << best_turns;
    auto best_score = to_score(best_path,best_turns);
    std::cout << NL << "best_score:" << best_score;

    Grid grid = example_model;
    aoc::grid::to_dir_traced(grid, best_path);
    Expected best{grid,best_score,best_path.size()-1
      ,best_turns.size()};
    Outcome outcome{expected,best};
    std::cout << NL << outcome;
    return (outcome.expected == outcome.deduced);
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
      std::cout << NL << NL << "best_turns:" << best_turns;
      
      Grid grid = model;
      aoc::grid::to_dir_traced(grid, best_path);
      std::cout << NL << grid;
      
      auto best_score = test::to_score(best_path,best_turns);
      std::cout << NL << NL << "best_score:" << best_score;

      response << best_score;
    }
    if (response.str().size()>0) return response.str();
    return std::nullopt;
  }

}
namespace part2 {
  using Paths = std::vector<Path>;
  using Position = std::tuple<int,int>; // row,col

  std::set<aoc::grid::Position> to_visited(Position const& start, Position const& end,Grid const& grid) {
    using Direction = std::tuple<int,int>; // dr,dc
    using Pose = std::tuple<Position,Direction>;
    using Poses = std::vector<Pose>;
    using Cost = int;
    using Node = std::tuple<Cost,Pose>; // Natural ordering Cost then Pose will work for our priority_queue
    
    // Priority queue sorted on lowest cost (first in tuple 'Node')
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
    std::map<Pose, Integer> lowest_cost_of;
    std::map<Pose, std::set<Pose>> previous_of;
    auto lowest_cost_to_end_so_far = std::numeric_limits<Integer>::max();
    std::set<Pose> processed{};
    
    Direction const EAST{0,1};
    Pose begin{start,EAST};
    pq.push({0,begin});
    lowest_cost_of[begin] = 0;
    processed.insert(begin);
    
    int loop_count{};
    while (!pq.empty()) {
      ++loop_count;
      auto curr = pq.top();pq.pop();
      // std::print("\nprocess {}",curr);
      auto [cost,pose] = curr;
      auto [pos,dir] = pose;
      // Skip if we 'already' explore a cost largest than one found
      // NOTE: We can't break as the queue may still not contain the best one
      if (cost > lowest_cost_to_end_so_far) {
        // Reduces iterations 1521 -> 1194 for small example
        // Still, example grid is 15*15 = 225 positions.
        // And around half(?) are walls anyhow.
        // Assume each free position corresponds to 5 states (four rotaions and one step)
        // 5*225 = 1225.
        // How can we get more than 1225 iterations?
        continue;
      }
      if (pos==end) {
        // We may end up here taking a path more costly than the overall best one.
        // That is, a better one is not yet on queue but may be added later
        // as we explore more paths.
        std::print("\n*END* at {}",curr);
        if (cost < lowest_cost_to_end_so_far) {
          // Reduce best so faar.
          lowest_cost_to_end_so_far = cost;
        }
        continue;
      }
      processed.insert(pose); // Now properly processed for best path to end
      // Note: We have to pick a candidate for next to be able to
      //       update the 'back track' mapping pos <-- previous_of(next)
      auto [r,c] = pos;
      auto [dr,dc] = dir;

      // Expand into candidates.
      // From all my studies of prefssional solvers I have gotten
      // influenced to use this short hand design.
      // Loop over known 'steps' and ascociated cost in one go.
      // next cost,pos{nr,nc}, dir{ndr,ndc}
      for (auto [step_cost,nr,nc,ndr,ndc] : std::vector<std::tuple<Cost,int,int,int,int>>{
         {1000,r,c,-dc,dr} // rotate e.g., +,0 to 0,+ = left cost 1000
        ,{1000,r,c,dc,-dr} // rotate e.g., +,0 to 0,-1 = right cost 1000
        ,{1,r+dr,c+dc,dr,dc} // step 'forward' cost 1
      }) {
        // Conveniance re-pack
        Position next_pos{nr,nc};
        Direction next_dir{ndr,ndc};
        Pose next_pose{next_pos,next_dir};
        auto next_cost = cost + step_cost;
        
        // filter candidate
        if (grid.at({r,c}) == '#') continue;
        if (processed.contains(next_pose)) continue; // skip already processed 'next'

        // Update cost map and backtrack linking
        auto lowest_cost_of_next
          =   lowest_cost_of.contains(next_pose)
            ? lowest_cost_of.at(next_pose)
            : std::numeric_limits<Integer>::max();
        if (next_cost<lowest_cost_of_next) {
          // New best
          lowest_cost_of[next_pose] = next_cost;
          previous_of[next_pose].clear(); // Throw away any previous record of 'best'
          previous_of[next_pose].insert(pose); // add to link options
        }
        else if (next_cost == lowest_cost_of_next) {
          // same as previous best
          previous_of[next_pose].insert(pose); // add another link option
        }
        pq.push({next_cost,next_pose}); // Accept/Explore candidate
      }
    }
    std::print("\nSearch consumed {} iterations",loop_count);

    
    // Back-track previous_of to find all members of the best paths found
    std::set<aoc::grid::Position> result{};

    std::queue<std::vector<Pose>> q{};
    auto [er,ec] = end;
    Position e{er,ec};
    for (auto pose : std::vector<Pose>{{e,{0,1}},{e,{0,-1}},{e,{1,0}},{e,{-1,0}}}) {
      q.push({pose}); // all possibel end poses to track from
    }
    while (not q.empty()) {
      auto path = q.front(); q.pop();
      auto [pos,dir] = path.back();
      if (pos == start) {
        for (auto [pos,dir] : path) {
          auto [r,c] = pos;
          result.insert({r,c});
        }
      }
      for (auto prev : previous_of[path.back()]) {
        auto next_path = path;
        next_path.push_back(prev);
        q.push(next_path);
      }
    }
          
    return result;
  }

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part2";
    if (in) {
      auto model = parse(in);
      // A bit convoluted from trying out 'raw' C++ types for part_2 search (not my aoc-types)
      auto [sr,sc] = model.find('S');
      auto [er,ec] = model.find('E');
      auto visited = part2::to_visited({sr,sc},{er,ec}, model); // returns set of aoc::grid::Position
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

int main(int argc, char *argv[]) {
  aoc::application app{};
  app.add_to_examples(test::to_examples);
  app.add_test("test1",test::test1);
  app.add_solve_for("1",part1::solve_for,"example.txt");
  app.add_solve_for("1",part1::solve_for,"example1.txt");
  app.add_solve_for("1",part1::solve_for,"puzzle.txt");
  app.add_solve_for("2",part2::solve_for,"example.txt");
  app.add_solve_for("2",part2::solve_for,"example1.txt");
  app.add_solve_for("2",part2::solve_for,"puzzle.txt");
  app.run(argc, argv);
  app.print_result();

  /*

   Xcode Debug -O2

   >day_16 -all
         
   ANSWERS
   duration:4ms answer[part:"test1"] PASSED
   duration:2ms answer[part 1 in:example.txt] 7036
   duration:2ms answer[part 1 in:example1.txt] 11048
   duration:110ms answer[part 1 in:puzzle.txt] 95476
   duration:7ms answer[part 2 in:example.txt] 45
   duration:6ms answer[part 2 in:example1.txt] 64
   duration:3578ms answer[part 2 in:puzzle.txt] 511
   
   */
  return 0;

}
