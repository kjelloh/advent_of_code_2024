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

std::filesystem::path to_working_dir_path(std::string const& file_name) {
  static std::optional<std::filesystem::path> cached{};
  if (not cached) {
    cached = "../..";
    if (auto dir = get_working_dir()) {
      cached = *dir;
    }
    else {
      std::cout << NL << "No working directory path configured";
    }
    std::cout << NL << "Using working_dir " << *cached;
  }
  return *cached / file_name;
}

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = Integer;

using aoc::grid::Grid;
using aoc::grid::Position;
using Move = char;
using Moves = aoc::raw::Line;
struct Model {
  Grid grid{{}};
  Moves moves{};
};

std::ostream& operator<<(std::ostream& os,Model const& model) {
  std::cout << model.grid << " moves:" << std::quoted(model.moves);
  return os;
}

Model parse(auto& in) {
  using namespace aoc::parsing;
  auto input = Splitter{in};
  auto sections = input.sections();
  Moves moves = std::accumulate(sections[1].begin(), sections[1].end(), Moves{},[](Moves acc,auto const& line){
    acc += aoc::parsing::to_raw(line);
    return acc;
  });
  Model result{aoc::parsing::to_raw(sections[0]),moves};
  return result;
}

using Args = std::vector<std::string>;

struct Simulation {
  Position pos{};
  Grid grid;
};

Result to_gps_coordinate(Position const& pos) {
  return 100*pos.row + pos.col;
}

Result to_result(Grid const& grid) {
  Result result{};
  auto lanterns = grid.find_all('O');
  result = std::accumulate(lanterns.begin(), lanterns.end(), result,[](auto acc,Position const& pos){
    acc += to_gps_coordinate(pos);
    return acc;
  });
  return result;
}


std::ostream& operator<<(std::ostream& os,Simulation const& sim) {
  std::cout << " pos:" << sim.pos;
  std::cout << NL << sim.grid;
  return os;
}

using aoc::grid::Direction;
using aoc::grid::UP;
using aoc::grid::DOWN;
using aoc::grid::LEFT;
using aoc::grid::RIGHT;

Direction to_direction(Move move) {
  switch (move) {
    case '>' : return RIGHT;
    case '<' : return LEFT;
    case '^' : return UP;
    case 'v' : return DOWN;
      throw std::runtime_error(std::format("Sorry, Unknown move '{}'",move));
  }
}

namespace test {
  struct LogEntry {
    char move;
    Grid grid;
  };
  std::ostream& operator<<(std::ostream& os,LogEntry const& entry) {
    os << "Logged Move " << entry.move << ":";
    os << NL << entry.grid;
    return os;
  }
  using Log = std::vector<LogEntry>;
  std::ostream& operator<<(std::ostream& os,Log const& log) {
    for (auto const& entry : log) {
      std::cout << NL << entry;
    }
    return os;
  }

  using State = std::pair<Simulation,LogEntry>;

  std::ostream& operator<<(std::ostream& os,State const& state) {
    std::cout << " simulated robot:" << state.first.pos;
    std::cout << NL << std::make_pair(state.first.grid, state.second.grid);
    return os;
  }


  Log parse(auto& in) {
    using namespace aoc::parsing;
    auto input = Splitter{in};
    auto sections = input.sections();
    std::cout << NL << "Log Parse:" << sections.size();
    Log result{};
    std::ranges::transform(sections,std::back_inserter(result),[](Section section){
      auto move = *(section[0].str().rbegin()+1);
      section.erase(section.begin());
      return LogEntry{move,Grid{to_raw(section)}};
    });
    return result;
  }

  // return range to shift [begin,end[
  // The range includes the robot position
  // "##@.O..#" <
  //     |
  //     begin
  //     end

  std::pair<Position,Position> to_range_to_shift(Direction dir,Simulation const& sim) {
    std::cout << NL << T << "to_range_to_shift";
    auto end = sim.pos-dir; // range end pos is after robot pos
    auto begin = sim.pos; // start at robot pos
    std::pair<Position,Position> result{end,end};
    while (true) {
      auto candidate = begin + dir; // peek ahead
      std::cout << " candidate:" << candidate;
      auto ch = sim.grid.at(candidate); // peek
      if (ch =='#') break; // candidate is invalid
      begin = candidate; // 'O' or '.' OK
      if (ch == '.') {
        result.first = begin;
        break; // begin refers to empty spot 'O'
      }
      // 'O' continue
    }
    std::cout << " begin:" << result.first << " end:" << result.second;
    return result;
  }

  Simulation& to_next(Simulation& curr,Move move) {
    std::cout << NL << T << "to_next";
    auto dir = to_direction(move);
    auto range_to_shift = to_range_to_shift(dir,curr);
    // shift in negative dir the range [begin,end[
    // iter = begin;
    // We shift [iter+1] -> [iter] as long as [iter+1] != end
    // Edge case for empty range [end,end[
    
    auto begin = range_to_shift.first; // begin == end if no shift shall take place
    auto end = range_to_shift.second; // end
    if (begin==end) return curr;
    // while iter,iter+1 is valid
    auto iter = begin;
    while (iter-dir != end) {
      std::cout << NL << T << "iter:" << iter << " <-- iter+1:" << (iter-dir);
      curr.grid.at(iter) = curr.grid.at(iter - dir);
      iter = iter - dir;
    }
    // We know we have shifted 1 step
    // iter is end+dir
    curr.grid.at(curr.pos) = '.'; // erease old '@' pos
    curr.pos = iter+dir; // moved in dir
    std::cout << NL << "curr:" << NL << curr;
    return curr;
  }

  bool test0() {
    bool result{};
    Result expected{104};
    char const* s = R"(#######
#...O..
#......)";
    aoc::parsing::Splitter input{s};
    Grid grid{aoc::parsing::to_raw(input.lines())};
    std::cout << NL << grid;
    auto grid_gps = to_result(grid);
    std::cout << NL << T << "grid_gps:" << grid_gps << " expected:" << expected;
    result = (to_result(grid) == expected);
    return result;
  }

  bool test1(Model const& model,Log const& log) {
    bool result{true};
    std::cout << NL << "TEST1";
    auto starts = model.grid.find_all('@');
    if (starts.size()==1) {
      auto start = starts[0];
      Simulation curr{start,model.grid};
      for (int i=0;i<log.size()-1;++i) {
        std::cout << NL << NL << "step[" << i << "] " << log[i];
        State state{curr,log[i]};
        std::cout << NL << NL << state;
        result = result and (curr.grid == log[i].grid);
        std::cout << NL << "RESULT SO FAR:" << to_result(curr.grid);
        if (not result) break;
        char move = model.moves[i];
        std::cout << NL << T << "move:" << move << " logged move:" << log[i+1].move;
        if (move != log[i+1].move) break;
        curr = to_next(curr,move);
      }
      if (result) {
        std::cout << NL << T << "passed";
        std::cout << NL << NL << "ANSWER:" << to_result(curr.grid);
      }
      else std::cout << NL << T << "FAILED";
    }
    else {
      std::cerr << NL << "failed to find unique start position";
    }
    return result;
  }

  bool test2(Model const& model) {
    bool result{true};
    std::cout << NL << "TEST1";
    auto starts = model.grid.find_all('@');
    if (starts.size()==1) {
      auto start = starts[0];
      Simulation curr{start,model.grid};
      for (int i=0;i<model.moves.size();++i) {
        Move move = model.moves[i];
        std::cout << NL << NL << "step[" << i << "] " << move;
        curr = to_next(curr,move);
      }
      std::cout << NL << NL << "ANSWER:" << to_result(curr.grid);
    }
    else {
      std::cerr << NL << "failed to find unique start position";
    }
    return true;
  }

}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      std::cout << NL << model;
      if (model.grid.width() == 8) {
        bool test_result{};
        auto log_file_path = to_working_dir_path("example.log");
        std::ifstream in{log_file_path};
        if (in) {
          auto log = test::parse(in);
          test_result = test::test1(model,log);
        }
        else {
          std::cerr << NL << "Sorry, no log file" << log_file_path;
        }
        return (test_result)?0:-1;
      }
      if (model.grid.width() == 10) {
        test::test2(model);
      }
      else {
        // Full puzzle
        test::test2(model);
      }
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

  Answers answers{};
  std::vector<std::chrono::time_point<std::chrono::system_clock>> exec_times{};
  exec_times.push_back(std::chrono::system_clock::now());
  std::vector<int> states = {10};
  for (auto state : states) {
    switch (state) {
      case 101: {
        if (test::test0()) std::cout << NL << "passed test0";
        else std::cout << NL << "FAILED test0";
      } break;
      case 11: {
        auto file = to_working_dir_path("example.txt");
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1 Example",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 12: {
        auto file = to_working_dir_path("example2.txt");
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1 Example 2",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 10: {
        auto file = to_working_dir_path("puzzle.txt");
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1     ",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 21: {
        auto file = to_working_dir_path("example.txt");
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2 Example",part2::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 20: {
        auto file = to_working_dir_path("puzzle.txt");
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
