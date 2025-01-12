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
  using  namespace aoc::parsing;
  std::cout << "\n<BEGIN parse>";
  Model result{};
  std::string line{};
  int count{};
  auto lines = Splitter{in}.lines();
  for (auto const& line : lines) {
    std::cout << "\nLine[" << count++ << "]:" << " " << std::quoted(line.str()) << ":" << line.size();
    result.push_back(line);
  }
  std::cout << "\n<END parse>";
  return result;
}

using Position = std::pair<int,int>;
using Delta = Position;
using PositionPair = std::pair<Position,Position>;

std::ostream& operator<<(std::ostream& os,Position pos) {
  os << "[r:" << pos.first << ",c:" << pos.second << "]";
  return os;
}

Delta to_delta(PositionPair const& pp) {
  // pp.first + delta = pp.second
  return {{pp.second.first-pp.first.first},{pp.second.second-pp.first.second}};
}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      auto const WIDTH = model[0].size();
      auto const HEIGHT = model.size();
      std::map<char,std::vector<Position>> c2p{};
      for (int r=0;r<model.size();++r) {
        for (int c=0;c<model[0].size();++c) {
          auto ch = model[r][c];
          if (ch != '.') c2p[ch].push_back({r,c});
        }
      }
      if (true) {
        for (auto const& entry : c2p) {
          std::cout << NL << entry.first;
          for (auto const& pos : entry.second) {
            std::cout << " [r:" << pos.first << ",c:" << pos.second << "]";
          }
        }
      }
      
      std::set<Position> candidates{};
      for (auto const& entry : c2p) {
        auto ch = entry.first;
        auto v = entry.second;
        for (int i=0;i<v.size();++i) {
          for (int j=0;j<v.size();++j) {
            if (i!=j) {
              auto vi = v[i];
              auto vj = v[j];
              PositionPair pp{vi,vj};
              auto delta = to_delta(pp);
              
              // vi+delta=vj --> vj+delta = vi+2*delta
              Position c{vj.first+delta.first,vj.second+delta.second};
              std::cout << NL << vi << " and " << vj;
              std::cout << " candidate " << c;
              if (c.first>=0 and c.first<WIDTH and c.second>=0 and c.second < HEIGHT) {
                std::cout << " OK (on grid)";
                candidates.insert(c);
              }

              if (delta.first % 2 == 0 and delta.second % 2 == 0) {
                std::cout << " between candidates ";
                // halfway between is valid c1=vi+0.5*delta, c2=vj-0.5*delta
                // NOTE: For my input this never happens?
                Position c1{vi.first+delta.first/2,vi.second+delta.second/2};
                Position c2{vj.first-delta.first/2,vj.second-delta.second/2};
                std::cout << c1 << " " << c2;
                candidates.insert(c1);
                candidates.insert(c2);
              }
            }
          }
        }
      }
      result = std::to_string(candidates.size());
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
      auto const WIDTH = model[0].size();
      auto const HEIGHT = model.size();
      std::map<char,std::vector<Position>> c2p{};
      for (int r=0;r<model.size();++r) {
        for (int c=0;c<model[0].size();++c) {
          auto ch = model[r][c];
          if (ch != '.') c2p[ch].push_back({r,c});
        }
      }
      if (true) {
        for (auto const& entry : c2p) {
          std::cout << NL << entry.first;
          for (auto const& pos : entry.second) {
            std::cout << " [r:" << pos.first << ",c:" << pos.second << "]";
          }
        }
      }
      
      std::set<Position> candidates{};
      for (auto const& entry : c2p) {
        auto ch = entry.first;
        auto v = entry.second;
        for (int i=0;i<v.size();++i) {
          for (int j=0;j<v.size();++j) {
            if (i!=j) {
              auto vi = v[i];
              auto vj = v[j];
              std::cout << NL << vi << " and " << vj;
              PositionPair pp{vi,vj};
              auto delta = to_delta(pp);
              for (int factor=0;factor<=WIDTH+1;++factor) {
                Position c{vi.first+factor*delta.first,vi.second+factor*delta.second};
                std::cout << NL << T << " candidate " << c;
                if (c.first>=0 and c.first<WIDTH and c.second>=0 and c.second < HEIGHT) {
                  std::cout << " OK (on grid)";
                  candidates.insert(c);
                }
                else {
                  std::cout << " BREAK";
                  break;
                }
              }
            }
          }
        }
      }
      result = std::to_string(candidates.size());
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

   >day_8 -all
   
   For my input:
                  
   ANSWERS
   duration:0ms answer[part 1 in:example.txt] 14
   duration:4ms answer[part 1 in:puzzle.txt] 244
   duration:0ms answer[part 2 in:example.txt] 34
   duration:13ms answer[part 2 in:puzzle.txt] 912

   */
  return 0;

}
