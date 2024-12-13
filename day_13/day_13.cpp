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
using Result = Integer;
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

using Args = std::vector<std::string>;

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      std::cout << NL << "model:" << model;
      
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
  std::vector<int> states = {11};
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
