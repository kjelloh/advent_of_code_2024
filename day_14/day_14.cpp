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

using namespace aoc::raw;

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = Integer;

using aoc::xy::Vector;
using Robot = std::pair<Vector,Vector>;
using Robots = std::vector<Robot>;
std::ostream& operator<<(std::ostream& os,Robot const& robot) {
  std::cout << "{p:" << robot.first << ",v:" << robot.second << "}";
  return os;
}
std::ostream& operator<<(std::ostream& os,Robots const& robots) {
  std::cout << "[";
  std::string sep{};
  for (auto const& robot : robots) {
    std::cout << sep << robot;
    if (sep.size()==0) sep = "\n\t,";
  }
  return os;
  std::cout << "]";
}

using Model = Robots;

Model parse(auto& in) {
  using namespace aoc::parsing;
  Model result{};
  auto input = Splitter{in};
  auto lines = input.lines();
  std::cout << NL << T << lines.size() << " lines";
  for (int i=0;i<lines.size();++i) {
    auto line = lines[i];
    std::cout << NL << T << T << "line[" << i << "]:" << line.size() << " " << std::quoted(line.str());
    // "p=20,54 v=57,17"
    auto groups = line.groups(R"(p=([+-]?\d+),([+-]?\d+) v=([+-]?\d+),([+-]?\d+))");
    if (groups.size()==4) {
      auto px = std::stoi(groups[0].str());
      auto py = std::stoi(groups[1].str());
      auto vx = std::stoi(groups[2].str());
      auto vy = std::stoi(groups[3].str());
      result.push_back({{px,py},{vx,vy}});
    }
    else {
      std::cerr << NL << "PARSE ERROR: WRong numbers count: " << groups.size() << " in " << std::quoted(line.str());
    }
  }
  return result;
}

using Args = std::vector<std::string>;

Robots to_stepped(Robots const& robots,auto width,auto height,int const STEPS) {
  Robots result{robots};
  std::for_each(result.begin(), result.end(), [width,height,STEPS](Robot& robot){
    
    auto [p,v] = robot;
    auto [x,y] = p;
    auto [vx,vy] = v;
    
    auto end_x = (x + STEPS*vx) % width; // Wrap around
    if (end_x<0) end_x += width;
    auto end_y = (y + STEPS*vy) % height;
    if (end_y<0) end_y += height;
    
    std::cout << NL << T << "robot:" << robot << " --> ";
            
    robot.first.x = end_x;
    robot.first.y = end_y;
    
    std::cout << robot;
  });
  return result;
}

using aoc::grid::Grid;
Grid to_grid(Robots const& robots,auto width,auto height) {
  aoc::grid::Grid result{std::vector<std::string>(height,std::string(width,'.'))};
  for (auto const& [p,v] : robots) {
    aoc::grid::Position pos{static_cast<int>(p.y),static_cast<int>(p.x)};
    auto& ch = result.at(pos);
    if (ch >= '0' and ch <= '9') ch += 1;
    else ch = '1';
  }
  return result;
}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      std::cout << NL << "robots:" << model;
      // Allow to solve for both example and puzzle
      // In my input and example there is always a robot at left most and bottom most.
      auto [p_max,_] = std::accumulate(model.begin(), model.end(), Robot{},[](auto acc,auto const& robot){
        auto [p,v] = robot;
        Vector max_p = {std::max(acc.first.x,p.x),std::max(acc.first.y,p.y)};
        acc.first = max_p;
        return acc;
      });
      auto width = p_max.x+1;
      auto height = p_max.y+1;
      std::cout << NL << "width:" << width << " height:" << height;
      std::cout << NL << to_grid(model, width, height);

      if (false) {
        // Test
        int const STEPS{1};
        auto transformed = to_stepped(model, width, height, STEPS);
//        std::cout << NL << "transformed:" << transformed;
        std::cout << NL << to_grid(transformed,width,height);
        return 0;
      }
      
      int const STEPS{100};
      auto transformed = to_stepped(model, width, height, STEPS);
//      std::cout << NL << "transformed:" << transformed;
      std::cout << NL << to_grid(transformed,width,height);

      
      std::array<Result,4> acc{};
      auto in_quadrant = [width,height](Robot const& robot,int ix){
        // assume odd width and height
        auto ghosted_col = width / 2; // 11 -> 6, 101 -> 51
        auto ghosted_row = height / 2; // 7 -> 4, 103 -> 52
        auto [p,v] = robot;
        auto [col,row] = p;
        if (col < ghosted_col and row < ghosted_row and ix ==1) return true;
        if (col > ghosted_col and row < ghosted_row and ix ==2) return true;
        if (col < ghosted_col and row > ghosted_row and ix ==3) return true;
        if (col > ghosted_col and row > ghosted_row and ix ==4) return true;
        return false;
      };

      for (auto const& robot : transformed) {
        std::cout << NL << "robot:" << robot;
        for (int i=1;i<=4;++i) {
          if (in_quadrant(robot, i)) {
            std::cout << " In quadrant " << i;
            ++acc[i-1];
            break;
          }
        }
      }
      
      std::cout << NL << "quadrant counts";
      for (int i=1;i<=4;++i) {
        std::cout << NL << T << "acc[" << i << "]:" << acc[i-1];
      }

      result = acc[0]*acc[1]*acc[2]*acc[3];
        
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

  std::filesystem::path working_dir{"../.."};
  if (auto dir = get_working_dir()) {
    working_dir = *dir;
  }
  else {
    std::cout << NL << "No working directory path configured";
  }
  std::cout << NL << "Using working_dir " << working_dir;

  Answers answers{};
  std::vector<std::chrono::time_point<std::chrono::system_clock>> exec_times{};
  exec_times.push_back(std::chrono::system_clock::now());
//  std::vector<int> states = {11};
  std::vector<int> states = {10};
  for (auto state : states) {
    switch (state) {
      case 11: {
        std::filesystem::path file{working_dir / "example.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1 Example",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 10: {
        std::filesystem::path file{working_dir / "puzzle.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1     ",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 21: {
        std::filesystem::path file{working_dir / "example.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2 Example",part2::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 20: {
        std::filesystem::path file{working_dir / "puzzle.txt"};
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
   ...
  */
  return 0;
}