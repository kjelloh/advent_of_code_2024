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

using aoc::raw::NL;
using aoc::raw::T;
using aoc::raw::NT;

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = aoc::raw::Line;
using Model = aoc::raw::Lines;

Model parse(auto& in) {
  using namespace aoc::parsing;
  Model result{};
  auto lines = Splitter{in}.lines();
  for (auto const& [lx,line] : aoc::views::enumerate(lines)) {
    std::cout << NL << T << T << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
    result.push_back(line);
  }
  return result;
}

using Args = std::vector<std::string>;

namespace test {

  // Adapt to expected for day puzzle
  struct LogEntry {
    aoc::raw::Line code;
    aoc::raw::Line expected_presses;
    bool operator==(LogEntry const& other) const {
      bool result{true};
      return result;
    }
  };

  std::ostream& operator<<(std::ostream& os,LogEntry const& entry) {
    std::cout << entry.code << " -- ? --> " << entry.expected_presses;
    return os;
  }

  using LogEntries = aoc::test::LogEntries<LogEntry>;

  LogEntries parse(auto& doc_in) {
    std::cout << NL << T << "test::parse";
    LogEntries result{};
    using namespace aoc::parsing;
    auto sections = Splitter{doc_in}.same_indent_sections();
    int target_sx{-1};
    for (auto const& [sx,section] : aoc::views::enumerate(sections)) {
      std::cout << NL << "---------- section " << sx << " ----------";
      for (auto const& [lx,line] : aoc::views::enumerate(section)) {
        std::cout << NL << T << T << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
        if (line.str().find("here is a shortest sequence of button presses") != std::string::npos) {
          target_sx = static_cast<int>(sx)+1;
        }
        if (sx == target_sx) {
          auto const& [left,right] = line.split(':');
          LogEntry entry{left,right.trim()};
          result.push_back(entry);
        }
      }
    }
    return result;
  }

  using aoc::grid::Grid;
  using aoc::grid::Position;
  class Robot {
  public:
    Robot(Grid grid) : m_grid(std::move(grid)) {
      m_current = m_grid.find_all('A')[0];
    }
    std::string press(std::string path) {
      std::cout << NL << "Robot::press:" << path;
      std::string result{};
      for (char ch : path) {
        auto end = m_grid.find_all(ch)[0];
        auto best = find(end);
//        std::cout << NL << "best:" << best;
        result += best;
        result.push_back('A');
      }
      return result;
    }
  private:
    std::string find(Position const& end) {
//      std::cout << NL << "find(" << end << ")";
      std::string result{};
      using CostToPos = std::pair<int,Position>;
//      using PQ = std::priority_queue<CostToPos>;
      using PQ = std::priority_queue<CostToPos, std::vector<CostToPos>, std::greater<CostToPos>>;
      using CostMap = std::map<Position,int>;
      using PrevMap = std::map<Position, Position>;
      PQ pq{};
      auto start = m_current;
      pq.push({0,start});
      CostMap cost_map{};
      cost_map[start] = 0;
      PrevMap prev_map{};
      while (not pq.empty()) {
        auto [cost,current] = pq.top();
        pq.pop();
//        std::cout << NL << T << pq.size() << " " << current << " " << end;
        if (current == end) {
          this->m_current = end;
//          std::cout << NL << "best:" << cost_map[end];
          break;
        }
        using namespace aoc::grid;
        for (auto& dp : {UP,LEFT,DOWN,RIGHT}) {
          auto next = current + dp;
          if (not m_grid.on_map(next)) continue;
          if (m_grid.at(next) == ' ') continue;
          auto new_cost = cost+1;
          if (not cost_map.contains(next) or new_cost < cost_map[next]) {
            cost_map[next] = new_cost;
            pq.push({new_cost,next});
            prev_map[next] = current;
          }
        }
      }
      // Reconstruct best path
      auto current = end;
      auto prev = prev_map[current];
//      std::cout << NL << T << "path:";
      while (current != start) {
//        std::cout << NL << T << current << " <-- " << prev;
        result.push_back(aoc::grid::to_dir_char(prev,current));
//        std::cout << " result:" << std::quoted(result);
        current = prev;
        prev = prev_map[current];
      }
      std::reverse(result.begin(), result.end());
      return result;
    }
    Position m_current;
    aoc::grid::Grid m_grid;
  };

  std::optional<Result> test0(Args args) {
    
    aoc::raw::Lines answers{};
    std::cout << NL << NL << "test0";
    //    For example, to make the robot type 029A on the numeric keypad, one sequence of inputs on the directional keypad you could use is:
    //
    //    < to move the arm from A (its initial position) to 0.
    //    A to push the 0 button.
    //    ^A to move the arm to the 2 button and push it.
    //    >^^A to move the arm to the 9 button and push it.
    //    vvvA to move the arm to the A button and push it.
    aoc::grid::Grid keypad({
       "789"
      ,"456"
      ,"123"
      ," 0A"
    });
    Robot r(keypad);
    auto moves = r.press("029A");
    std::cout << NL << "moves:" <<  moves;
    // moves:<A^A>^^AvvvA
    if (moves == "<A^A>^^AvvvA") {
      answers.push_back(moves + " PASSED");
    }
    if (answers.size()>0) {
      std::string acc{};
      for (auto const& answer : answers) {
        acc += NL + answer;
      }
      return acc;
    }
    return std::nullopt;
  }

  std::optional<Result> test1(auto& in, auto& doc_in,Args args) {
    std::optional<Result> result{};
    aoc::raw::Lines answers{};
    std::cout << NL << NL << "test1";
    if (in) {
      auto model = ::parse(in);
      if (doc_in) {
        auto log = test::parse(doc_in);
        using aoc::test::operator<<;
        std::cout << NL << log;
        aoc::grid::Grid keypad({
           "789"
          ,"456"
          ,"123"
          ," 0A"
        });
        aoc::grid::Grid robot_remote({
           " ^A"
          ,"<v>"
        });

        
//        Robot::press:029A
//                           Robot::press:<A^A^^>AvvvA
        // <A^A>^^AvvvA, <A^A^>^AvvvA, or <A^A^^>AvvvA
//        Robot::press:<v<A>^>A<A>A<AA>vA^A<vAAA^>A
                    // v<<A>>^A<A>AvA<^AA>A<vAAA>^A

//        my press:<v<A>A<A>^>AvA^<A>vA^A<v<A>^>AvA^A<v<A>^>AAvA<A^>A<A>A<v<A>A^>AAA<A>vA^A
          //       ^v< v <
          //          < v <
//        expected:<vA<AA>>^AvAA<^A>A<v<A>>^AvA^A<vA>^A<v<A>^A>AAvA^A<v<A>A>^AAAvA<^A>A
          //       ^v

//        Robot::press:029A
//        Robot::press:<A^A>^^AvvvA
//        Robot::press:v<<A>>^A<A>AvA<^AA>Av<AAA>^A
//        my press:v<A<AA>>^AvAA<^A>Av<<A>>^AvA^Av<A>^Av<<A>^A>AAvA^Av<A<A>>^AAAvA<^A>A
          //       <vA<AA>>^AvAA<^A>A<v<A>>^AvA^A<vA>^A<v<A>^A>AAvA^A<v<A>A>^AAAvA<^A>A
//        expected:<vA<AA>>^AvAA<^A>A<v<A>>^AvA^A<vA>^A<v<A>^A>AAvA^A<v<A>A>^AAAvA<^A>A

//        Robot::press:980A
//        Robot::press:<^<^^>>A<AvvvA>A
        //             0214789 8
//        Robot::press:v<<A>^Av<A>^AAv>AA^Av<<A>>^Av<AAA>^AvA^A
//        my press:v<A<AA>>^AvA<^A>Av<A<A>>^AvA<^A>AAv<A>A^AA<A>Av<A<AA>>^AvAA<^A>Av<A<A>>^AAAvA<^A>Av<A>^A<A>A
//        expected:<v<A>>^AAAvA^A<vA<AA>>^AvAA<^A>A<v<A>A>^AAAvA<^A>A<vA>^A<A>A
        
        // Shoot! We need to examine all possible moves from robot to see what path the next robot can take
        // that is the shortest one!
        for (LogEntry const& entry : log) {
          Robot robot0{keypad};
          Robot robot1{robot_remote};
          Robot robot2{robot_remote};
          auto moves0 = robot0.press(entry.code);
          auto moves1 = robot1.press(moves0);
          auto moves2 = robot2.press(moves1);
          std::cout << NL << "my press:" << moves2;
          std::cout << NL << "expected:" << entry.expected_presses;
          if (moves2 == entry.expected_presses) {
            answers.push_back("my press:" + moves2 + " OK. PASSED");
          }
          else {
            answers.push_back("my press:" + moves2 + " not expected " + entry.expected_presses + " - FAILED");
          }
        }
        if (answers.size()>0) {
          std::string acc{};
          for (auto const& answer : answers) {
            acc += NL + answer;
          }
          return acc;
        }
      }
    }
    return result;
  }

}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
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
  for (int i=1;i<argc;++i) {
    args.push_back(argv[i]);
  }

  Answers answers{};
  std::vector<std::chrono::time_point<std::chrono::system_clock>> exec_times{};
  exec_times.push_back(std::chrono::system_clock::now());
  std::vector<int> states = {111};
//  std::vector<int> states = {0,111};
  for (auto state : states) {
    switch (state) {
      case 0: {
        answers.push_back({"test0",test::test0(args)});
      } break;
      case 111: {
        auto doc_file = aoc::to_working_dir_path("doc.txt");
        std::ifstream doc_in{doc_file};
        auto file = aoc::to_working_dir_path("example.txt");
        std::ifstream in{file};
        if (in and doc_in) answers.push_back({"Part 1 Test Example vs Log",test::test1(in,doc_in,args)});
        else std::cerr << "\nSORRY, no file " << file << " or doc_file " << doc_file;
      } break;
      case 11: {
        auto file = aoc::to_working_dir_path("example.txt");
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1 Example",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
      } break;
      case 10: {
        auto file = aoc::to_working_dir_path("puzzle.txt");
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1     ",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
      } break;
      case 211: {
        auto doc_file = aoc::to_working_dir_path("doc.txt");
        std::ifstream doc_in{doc_file};
        auto file = aoc::to_working_dir_path("example.txt");
        std::ifstream in{file};
        if (in and doc_in) answers.push_back({"Part 2 Test Example vs Log",test::test1(in,doc_in,args)});
        else std::cerr << "\nSORRY, no file " << file << " or doc_file " << doc_file;
      } break;
      case 21: {
        auto file = aoc::to_working_dir_path("example.txt");
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2 Example",part2::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
      } break;
      case 20: {
        auto file = aoc::to_working_dir_path("puzzle.txt");
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2     ",part2::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
      } break;
      default:{std::cerr << "\nSORRY, no action for state " << state;} break;
    }
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
   For my input:

   ANSWERS
   ...
      
  */
  return 0;
}
