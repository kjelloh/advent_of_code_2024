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
  int count{};
  while (std::getline(in,line)) {
    std::cout << "\nLine[" << ++count << "]:" << line.size() << " " << std::quoted(line);
    result.push_back(line);
  }
  std::cout << "\n<END parse>";
  return result;
}

class Vector {
public:
  Vector() = default;
  Vector(int row,int col) : m_row{row},m_col{col} {};
  Vector operator+(Vector const& other) {
    return Vector{m_row+other.m_row,m_col+other.m_col};
  }
  auto row() const {return m_row;}
  auto col() const {return m_col;}
  auto& row() {return m_row;}
  auto& col() {return m_col;}
  bool operator<(Vector const& other) const {
    return (m_row==other.m_row)?(m_col<other.m_col):(m_row<other.m_row);
  }
private:
  int m_row{};
  int m_col{};
};
void print(Vector const& v) {
  std::cout << "[row:" << v.row() << ",col:" << v.col() << "]";
}

void print(std::string s) {std::cout << s;}

class Simulation {
private:
  using Pos = Vector;
  using Dir = Vector;
  Model m_grid;
  Pos m_start;
  Pos m_pos{};
  Dir m_dir{};
  Integer m_count{};
  std::set<Vector> m_visited{};
  Vector const UP{-1,0};
  Vector const RIGHT{0,1};
  Vector const DOWN{1,0};
  Vector const LEFT{0,-1};
public:
  
  Simulation& restart() {
//    print("restart");
    m_pos = m_start;
    m_dir = UP;
    return *this;
  }
  
  std::pair<Vector,Vector> state() const {
    return {m_pos,m_dir};
  }

  char at(Vector const& pos) const {
    if (on_map(pos)) return m_grid[pos.row()][pos.col()];
    return '?';
  }

  void set(Vector const& pos,char ch) {
    if (not on_map(pos)) {
      std::ostringstream os{};
      os << "at(pos) called with pos:" << pos.row() << "," << pos.col() << " not on map";
      throw std::runtime_error{os.str()};
    }
    m_grid[pos.row()][pos.col()] = ch;
  }

  auto width() const {
    return m_grid[0].size();
  }
    
  auto height() const {
    return m_grid.size();
  }

  Simulation(Model const& grid) : m_grid{grid} {
    for (int row=0;row<height();++row) {
      for (int col=0;col<width();++col) {
        Vector pos{row,col};
        auto ch = at(pos);
        if (ch=='^') {
          m_start = pos;
          m_pos = m_start;
          m_dir = UP;
          print(NL);
          print("m_pos");print(m_pos);
          print(" ");
          print("m_dir");print(m_dir);
          m_grid[pos.row()][pos.col()] = '.';
          m_visited.insert(pos);
        }
      }
    }
  }
    
  bool on_map(Vector const& pos) const {
    return (pos.row()>=0 and pos.row() < height() and pos.col() >= 0 and pos.col() < width());
  }
  
  bool on_map() const {
    return on_map(m_pos);
  }

  bool operator++() {
    ++m_count;
    while (true) {
      auto next_pos = m_pos + m_dir;
  //    print(NL);
  //    std::cout << at(next_pos) << " at ";
  //    print("next_pos");print(next_pos);
      if (at(next_pos) != '#') break; // next step ok
      // turn right and try again
//      print(NL);print("TURN RIGHT");
      auto old_dir = m_dir;
      m_dir.row() = old_dir.col();
      m_dir.col() = -old_dir.row();
    }
    m_pos = m_pos + m_dir;
    if (on_map()) {
      m_visited.insert(m_pos);
//      print(" m_pos:");print(m_pos);
//      std::cout << " m_count:" << m_count;
//      std::cout << " m_visited:" << m_visited.size();
    }
    return on_map();
  }
  
  auto const& visited() const {return m_visited;}
};

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    Integer acc{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      Simulation sim{model};
      while (++sim) {}
      result = std::to_string(sim.visited().size());
    }
    return result;
  }
}

namespace part2 {
  class LoopFinder {
  private:
    Simulation m_sim;
  public:
    LoopFinder(Simulation const& sim) : m_sim{sim} {
      while (++m_sim) {}
    }
    std::vector<Vector> positions() {
      std::vector<Vector> result{};
      // Place and obstacle somewhere along the original path
      // to divert into a loop
      for (auto const& pos : m_sim.visited()) {

// Try a l l grid positions for obstacle to see if we missed something?
//      for (int row=0;row<m_sim.height();++row)
//        for (int col=0;col<m_sim.width();++col) {
//          Vector pos{row,col};

        std::set<std::pair<Vector,Vector>> visited{};
//        print(NL);
//        print("try obstacle at pos:");
//        print(pos);
        Simulation sim_candidate{m_sim};
        sim_candidate.set(pos,'#');
        sim_candidate.restart();
//        print(NL);print(T);
        while (++sim_candidate) {
          if (visited.contains(sim_candidate.state())) {
            print(NL);
            print(" FOUND:");
            print(pos);
            result.push_back(pos);
            print(" count:");
            std::cout << result.size();
            break; // is loop
          }
          else {
//            print(" ");
//            print(sim_candidate.state().first);
            visited.insert(sim_candidate.state());
          }
        }
      }
      return result; // 1995 too low
    }
  };
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part2";
    if (in) {
      auto model = parse(in);
      std::cout << "\nheight:" << model.size() << " width:" << model[0].size();
      Simulation sim{model};
      LoopFinder loop_finder{sim};
      auto positions = loop_finder.positions();
//      for (auto const& pos : positions) {
//        print(NL);
//        print("will loop with obstacle at:");
//        print(pos);
//      }
      result = std::to_string(positions.size());

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

   >day_6 -all
   
   For my input:
               
   ANSWERS
   duration:1ms answer[part 1 in:example.txt] 41
   duration:11ms answer[part 1 in:puzzle.txt] 5516
   duration:3ms answer[part 2 in:example.txt] 6
   duration:24358ms answer[part 2 in:puzzle.txt] 2008

   
   */
  return 0;

}
