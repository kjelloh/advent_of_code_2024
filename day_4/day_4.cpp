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

auto const NL = "\n";
auto const T = "\t";
auto const NT = "\n\t";

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = std::string;
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

char at_pos(Vector const& pos,Model const& model) {
  return model[pos.row][pos.col];
}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    Integer acc{0};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      std::string const XMAS{"XMAS"};
      for (int row=0;row < model.size();++row) {
        for (int col=0;col<model[0].size();++col) {
          Vector start{row,col};
          if (at_pos(start,model)=='X') for (auto const& dv : DIRS) {
            auto pos = start;
            bool found_xmas{false};
            for (int index=0;index<XMAS.size();++index) {
              if (pos.row < 0 or pos.row >= model.size() or pos.col < 0 or pos.col > model[0].size()) {
                break;
              }
              if (at_pos(pos,model) == XMAS[index]) {
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
      result = std::to_string(acc);
    }
    return result;
  }
}

namespace part2 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    Integer acc{0};
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
      result = std::to_string(acc);
    }
    return result;
  }
}

int main(int argc, char *argv[]) {
  aoc::application app{};
  app.add_solve_for("1",part1::solve_for,"example.txt");
  app.add_solve_for("1",part1::solve_for,"puzzle.txt");
  app.add_solve_for("2",part2::solve_for,"example.txt");
  app.add_solve_for("2",part2::solve_for,"puzzle.txt");
  app.run(argc, argv);
  app.print_result();
  /*

   Xcode Debug -O2

   >day_4 -all
   
   For my input:
               
   ANSWERS
   duration:2ms answer[part 1 in:example.txt] 18
   duration:79ms answer[part 1 in:puzzle.txt] 2401
   duration:0ms answer[part 2 in:example.txt] 9
   duration:33ms answer[part 2 in:puzzle.txt] 1822

   */
  return 0;

}
