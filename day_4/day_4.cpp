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

auto const NL = "\n";
auto const T = "\t";
auto const NT = "\n\t";

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = Integer;
using Model = std::vector<std::string>;

Model parse(auto& in) {
  std::cout << "\n<BEGIN parse>";
  Model result{};
  std::string line{};
  while (std::getline(in,line)) {
    std::cout << "\nLine:" << std::quoted(line);
    result.push_back(line);
  }
  std::cout << "\n<END parse>";
  return result;
}

using Args = std::vector<std::string>;

struct Vector {
  int row;
  int col;
  Vector operator+(Vector const& other) const {return {row+other.row,col+other.col};}
  bool operator<(Vector const& other) const {
    return (row < other.row) or (row == other.row and col < other.col);
  }
};

std::vector<Vector> const DIRS = {
   {0,1}
  ,{1,1}
  ,{1,0}
  ,{1,-1}
  ,{0,-1}
  ,{-1,-1}
  ,{-1,0}
  ,{-1,1}
};
using Vectors = std::set<Vector>;

/*
 ....XXMAS.
 .SAMXMS...
 ...S..A...
 ..A.A.MS.X
 XMASAMX.MM
 X.....XA.A
 S.S.S.S.SS
 .A.A.A.A.A
 ..M.M.M.MM
 .X.X.XMASX
 
 0,4 1,1
 0,5 0,1
 1,4 0,-1
 3,9 1,0 1,-1
 4,0 0,1
 4,6 0,-1 -1,0
 5,6 -1,-1
 9,1 -1,1
 9,3 -1,-1 -1,1
 9,5 -1,-1 1,-1 0,1
 9,9 -1,-1 -1,0
 
 */

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    Result acc{0};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      std::string const XMAS{"XMAS"};
      for (int row=0;row < model.size();++row) {
        for (int col=0;col<model[0].size();++col) {
          // Try walking in all possible directions
          Vector start{row,col};
          for (auto const& dv : DIRS) {
            auto pos = start;
            bool found_xmas{false};
            for (int index=0;index<XMAS.size();++index) {
              if (pos.row < 0 or pos.row >= model.size() or pos.col < 0 or pos.col > model[0].size()) {
                break;
              }
              if (model[pos.row][pos.col] == XMAS[index]) {
                found_xmas = (index == XMAS.size()-1);
                pos = pos + dv;
                continue;
              }
              break;
            }
            if (found_xmas) {
              ++acc;
              std::cout << NL << "Found XMAS at [row:" << start.row << ",col:" << start.col << "]";
              std::cout << " in DIR [dr:" << dv.row << ",dc:" << dv.col << "]";
            }
            else {
              std::cout << NL << "SKIPPED [row:" << start.row << ",col:" << start.col << "]";
              std::cout << " in DIR [dr:" << dv.row << ",dc:" << dv.col << "]";
            }
          } // dirs
        }
      }
      result = acc;
    }
    return result;
  }
}

namespace part2 {
  char at_pos(Vector const& pos,Model const& model) {
    return model[pos.row][pos.col];
  }
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    Result acc{0};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      Vector const UP_LEFT{-1,-1};
      Vector const DOWN_LEFT{1,-1};
      Vector const UP_RIGHT{-1,1};
      Vector const DOWN_RIGHT{1,1};
      for (int row=1;row < model.size()-1;++row) {
        for (int col=1;col<model[0].size()-1;++col) {
          Vector pos{row,col};
          std::cout << NL << "Check [row:" << pos.row << ",col:" << pos.col << "]" << " --> acc:" << acc;
          if (at_pos(pos,model) == 'A') {
            bool mas_1 =
              (    at_pos(pos+UP_LEFT,model)=='M'
               and at_pos(pos+DOWN_RIGHT,model)=='S')
              or
              (    at_pos(pos+UP_LEFT,model)=='S'
               and at_pos(pos+DOWN_RIGHT,model)=='M');
            bool mas_2 =
              (    at_pos(pos+DOWN_LEFT,model)=='M'
               and at_pos(pos+UP_RIGHT,model)=='S')
              or
              (    at_pos(pos+DOWN_LEFT,model)=='S'
               and at_pos(pos+UP_RIGHT,model)=='M');

            if (mas_1 and mas_2) {
              ++acc;
              std::cout << NL << T << "Found X-MAS!";
            }
            else {
              std::cout << NL << T << "NON X-MAS 'A'";
            }
          }
        }
      }
      result = acc;
    }
    return result;
  }
}

using Answers = std::vector<std::pair<std::string,std::optional<Result>>>;
int main(int argc, char *argv[])
{
  Args args{};
  for (int i=0;i<argc;++i) {
    args.push_back(argv[i]);
  }
  Answers answers{};
  std::vector<std::chrono::time_point<std::chrono::system_clock>> exec_times{};
  exec_times.push_back(std::chrono::system_clock::now());
  std::vector<int> states = {3};
  for (auto state : states) {
    switch (state) {
      case 0: {
        std::filesystem::path file{"../../example.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1 Example",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 1: {
        std::filesystem::path file{"../../puzzle.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1     ",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 2: {
        std::filesystem::path file{"../../example.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2 Example",part2::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 3: {
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
   ...
  */
  return 0;
}
