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

// Try to read the path to the actual working directory
// from a text file at the location where we execute
std::optional<std::filesystem::path> get_working_dir() {
  std::optional<std::filesystem::path> result{};
    
  std::ifstream workingDirFile("working_dir.txt");

  std::string workingDir;
  std::getline(workingDirFile, workingDir); // Read the directory path
  std::filesystem::path dirPath{workingDir};

  if (std::filesystem::exists(dirPath) and std::filesystem::is_directory(dirPath)) {
    // Return the directory path as a std::filesystem::path
    result = std::filesystem::path(workingDir);
  }
  return result;
}

auto const NL = "\n";
auto const T = "\t";
auto const NT = "\n\t";

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Answer = aoc::application::Answer;
using aoc::xy::Vector;
struct MachineConfig {
  /*
   Button A: X+94, Y+34
   Button B: X+22, Y+67
   Prize: X=8400, Y=5400
   */
  Vector da{};
  Vector db{};
  Vector target{};
};
using MachineConfigs = std::vector<MachineConfig>;
std::ostream& operator<<(std::ostream& os,MachineConfig const& mc) {
  std::cout << "da:" << mc.da << " db:" << mc.db << " target:" << mc.target;
  return os;
}
std::ostream& operator<<(std::ostream& os,MachineConfigs const& mcs) {
  std::cout << "machine configs:";
  for (auto const& mc : mcs) std::cout << NL << T << mc;
  return os;
}
using Model = MachineConfigs;


Model parse(auto& in) {
  using namespace aoc::parsing;
  Model result{};
  auto input = Splitter{in};
  auto sections = input.sections();
  int section_count{};
  for (auto const& lines : sections) {
    result.push_back({});
    std::cout << NL << T << "sections[" << section_count++ << "]";
    std::cout << NL << T << lines.size() << " lines";
    for (int i=0;i<lines.size();++i) {
      auto line = lines[i];
      std::cout << NL << T << T << "line[" << i << "]:" << line.size() << " " << std::quoted(line.str());
      
      switch (i) {
        case 0:;
        case 1: {
          auto groups = line.groups(R"(Button (\w): X\+(\d+), Y\+(\d+))");
          std::cout << NL << T << std::quoted(line.str()) << " --> groups:" << groups.size();
          if (groups.size()==3) {
            auto caption = groups[0];
            auto dx = groups[1];
            auto dy = groups[2];
            if (caption.str() == "A") {
              result.back().da = {std::stoi(dx),std::stoi(dy)};
            }
            else if (caption.str() == "B") {
              result.back().db = {std::stoi(dx),std::stoi(dy)};
            }
            else {
              std::cerr << NL << "PARSE ERROR: Unknown button caption:" << std::quoted(caption.str());
            }
          }
          else {
            std::cerr << NL << "PARSE ERROR: Not a button config entry:" << std::quoted(line.str());
          }
        } break;
        case 2: {
          auto groups = line.groups(R"(Prize: X=(\d+), Y=(\d+))");
          if (groups.size()==2) {
            auto dx = groups[0];
            auto dy = groups[1];
            result.back().target = {std::stoi(dx),std::stoi(dy)};
          }
          else {
            std::cerr << NL << "PARSE ERROR: Not a prize config entry:" << std::quoted(line.str());
          }
        } break;
        default: {std::cerr << "PARSE ERROR: Unexpected line index:" << i;} break;
      }
    }
  }
  return result;
}

struct State {
  Integer x, y;
  Integer cost;
  int a_push_count{};
  int b_push_count{};
  bool operator>(const State& other) const {
      return cost > other.cost;
  }
};
std::ostream& operator<<(std::ostream& os,State state) {
  std::cout << "state:" << "x:" << state.x << " y:" << state.y << " cost:" << state.cost;
  return os;
}

using Seen = std::map<Integer,std::vector<State>>;
Integer find_min_cost(const MachineConfig& config,Integer const PUSH_LIMIT,Seen& seen) {
//    std::cout << NL << "find_min_cost:" << config;
    int const COST_A = 3;
    int const COST_B = 1;
  
    std::priority_queue<State, std::vector<State>, std::greater<State>> pq;
    std::set<std::pair<int, int>> visited;
    pq.push({0, 0, 0, 0, 0});

    while (!pq.empty()) {
//        std::cout << NL << T << "pq:" << pq.size();
        State current = pq.top();
        pq.pop();
//        std::cout << NL << T << T << "current:" << current;

      if (current.a_push_count > PUSH_LIMIT or current.b_push_count > PUSH_LIMIT) {
//          std::cout << NL << T << T << "SKIP PUSH LIMIT:";
//          std::cout << " a_push_count:" << current.a_push_count << " b_push_count:" << current.b_push_count;
          continue;
        }

        if (current.x == config.target.x and current.y == config.target.y) {
            std::cout << NL << T << T << "FOUND:" << current;
            seen[current.cost].push_back(current);
            return current.cost;
        }

        if (visited.count({current.x, current.y}) > 0) {
            continue;
        }
      
        visited.insert({current.x, current.y});
      
        // Passed target
        if (current.x > config.target.x or current.y > config.target.y) {
          continue;
        }

        // Passed button press limit
        if (current.x > config.target.x or current.y > config.target.y) {
          continue;
        }

        // Press Button A
        auto new_x_a = current.x + config.da.x;
        auto new_y_a = current.y + config.da.y;
        auto new_cost_a = current.cost + COST_A;

        // Check if we need to visit this position
        if (visited.count({new_x_a, new_y_a}) == 0) {
          pq.push({new_x_a, new_y_a, new_cost_a,current.a_push_count+1,current.b_push_count});
        }

        // Press Button B
        auto new_x_b = current.x + config.db.x;
        auto new_y_b = current.y + config.db.y;
        auto new_cost_b = current.cost + COST_B;

        // Check if we need to visit this position
        if (visited.count({new_x_b, new_y_b}) == 0) {
            pq.push({new_x_b, new_y_b, new_cost_b,current.a_push_count,current.b_push_count + 1});
        }
    }
    return -1; // failed
}

Integer find_min_cost(const MachineConfig& config,Integer const PUSH_LIMIT,bool is_part_2) {
  Seen seen{};
  if (is_part_2) {
    // Shoot, this is actually an algebraic problem?
    // Each config has a soluton given by wether the two equation system has an ineteger solution?
    /*
     Button A: X+94, Y+34
     Button B: X+22, Y+67
     Prize: X=8400, Y=5400
     */
    // {8400,5400} = m*{94  ,34} +   n{22   ,67}
    // {x   ,y}    = m*{da_x,da_y} + n*{db_x,db_y}

    // xy = m*da + n*db
    // x = m*da_x + n*db_x
    // y = m*da_y + n*db_y
    
    // Eliminate m
    // x*da_y = m*da_x*da_y + n*db_x*da_y
    // y*da_x = m*da_x*da_y + n*db_y*da_x
    // So,...
    // x*da_y - y*da_x = m*da_x*da_y - m*da_x*da_y + n*db_x*da_y - n*db_y*da_x
    // n = (x*da_y - y*da_x) / (db_x*da_y - db_y*da_x)
    
    // Eliminate n
    // x*db_y = m*da_x*db_y + n*db_x*db_y
    // y*db_x = m*da_y*db_x + n*db_y*db_x
    // x*db_y - y*db_x = m*da_x*db_y - m*da_y*db_x  + n*db_x*db_y - n*db_y*db_x
    // m = (x*db_y - y*db_x) / (da_x*db_y - da_y*db_x)

    auto [da,db,xy] = config;
    aoc::xy::Solver solver{xy,da,db}; // solve for {m,n} in: xy = m*da + n*db;
    if (auto omn = solver.solve()) {
      auto const& [m,n] = *omn;
      return 3*m+n;
    }
    return -1;
  }
  else {
    return find_min_cost(config, PUSH_LIMIT, seen);
  }
}

namespace part1 {
  std::optional<Answer> solve_for(std::istream& in,Args const& args) {
    std::optional<Answer> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      Integer acc{};
      auto model = parse(in);
      std::cout << NL << "model:" << model;
      for (auto const& mc : model) {
        std::cout << NL << "processing:" << mc;
        if (auto cost = find_min_cost(mc,100,false);cost >= 0) {
          std::cout << " --> min cost:" << cost;
          acc += cost;
        }
        else {
          std::cout << " --> FAILED ";
        }
      }
      result = std::to_string(acc);
    }
    return result;
  }
}

namespace part2 {
  std::optional<Answer> solve_for(std::istream& in,Args const& args) {
    Integer K{10000000000000LL};
    std::optional<Answer> result{};
    std::cout << NL << NL << "part2";
    if (in) {
      Integer acc{};
      auto model = parse(in);
      for (auto const& mc : model) {
        // Assume there is a cycle in the search space.
        // That is from push 1 to push 'cycle' the cost is the same
        // as from cycle+1 to 2*cycle.
        // If this is true we can calculate the best cost fro the first cycle.
        // And then just add that cost for each cycle until
        // we have a remaning push count to try at target?
        // We are seaching for {m,n} so that target = m*da + n*db.
        // With the cost = 3*m + n we want the {m,n} that gives us the lowest cost possible.
        // How do we find a cycle?
        // For a push count cycle we get 2*best(cycle) == best(2*cycle).
        
        MachineConfig mc_p2 = {mc.da,mc.db,{mc.target.x+K,mc.target.y+K}};
        std::cout << NL << "processing:" << mc_p2;
        if (auto cost = find_min_cost(mc_p2,10000,true);cost >= 0) {
          std::cout << " --> min cost:" << cost;
          acc += cost;
        }
        else {
          std::cout << " --> FAILED ";
        }
      }
      result = std::to_string(acc);
    }
    return result;
  }
}

std::vector<Args> to_requests(Args const& args) {
  std::vector<Args> result{};
  result.push_back(args); // No fancy for now
  return result;
}

int main(int argc, char *argv[]) {
  aoc::application app(to_requests,part1::solve_for,part2::solve_for);
  app.run(argc, argv);
  app.print_result();
  /*

   Xcode Debug -O2

   >day_13 -all
   
   For my input:
         
   ANSWERS
   duration:90ms answer[part1 example.txt] 480
   duration:2950ms answer[part1 puzzle.txt] 39290
   duration:0ms answer[part2 example.txt] 875318608908
   duration:18ms answer[part2 puzzle.txt] 73458657399094

   */

}
