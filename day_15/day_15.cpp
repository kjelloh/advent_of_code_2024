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
#include <thread>

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
using Result = std::string;

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

using aoc::Args;

struct Simulation {
  Position pos{};
  Grid grid;
};

Integer to_gps_coordinate(Position const& pos) {
  return 100*pos.row + pos.col;
}

Integer to_result(Grid const& grid) {
  Integer result{};
  auto lanterns = grid.find_all('O');
  result = std::accumulate(lanterns.begin(), lanterns.end(), result,[](auto acc,Position const& pos){
    acc += to_gps_coordinate(pos);
    return acc;
  });
  return result;
}


std::ostream& operator<<(std::ostream& os,Simulation const& sim) {
  os << " pos:" << sim.pos;
  os << NL << sim.grid;
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

std::pair<Position,Position> to_range_to_shift(Direction dir,Simulation const& sim) {
  auto end = sim.pos-dir; // range end pos is after robot pos
  auto begin = sim.pos; // start at robot pos
  std::pair<Position,Position> result{end,end};
  while (true) {
    auto candidate = begin + dir; // peek ahead
    auto ch = sim.grid.at(candidate); // peek
    if (ch =='#') break; // candidate is invalid
    begin = candidate; // 'O' or '.' OK
    if (ch == '.') {
      result.first = begin;
      break; // begin refers to empty spot 'O'
    }
    // 'O' continue
  }
  return result;
}

Simulation& to_next(Simulation& curr,Move move) {
  auto dir = to_direction(move);
  auto range_to_shift = to_range_to_shift(dir,curr);
  auto begin = range_to_shift.first; // end if empty or the emtpy pos to push into
  auto end = range_to_shift.second; // one 'after' '@'

  if (begin==end) return curr; // empty range

  auto iter = begin;
  // as-if for (iter=begin,iter!=end-1,++iter) for adjacent shift
  while (iter-dir != end) {
    // Will shift all 'O' and the '@'
    curr.grid.at(iter) = curr.grid.at(iter - dir);
    iter = iter - dir;
  }
  curr.grid.at(curr.pos) = '.'; // erease old '@' pos
  curr.pos = iter+dir; // moved in dir
  return curr;
}

namespace test {

  // Adapt to expected for day puzzle
  struct Expected {
    char move;
    Grid grid;
  };

  std::ostream& operator<<(std::ostream& os,Expected const& entry) {
    os << "Logged Move " << entry.move << ":";
    os << NL << entry.grid;
    return os;
  }

  using Expecteds = aoc::test::Expecteds<Expected>;

  std::vector<aoc::raw::Lines> to_examples(aoc::parsing::Sections const& sections) {
    std::vector<aoc::raw::Lines> result{};
    result.push_back({});
    std::ranges::copy(aoc::parsing::to_raw(sections[19]),std::back_inserter(result.back()));
    result.back().push_back("");
    std::ranges::copy(aoc::parsing::to_raw(sections[20]),std::back_inserter(result.back()));

    result.push_back({});
    std::ranges::copy(aoc::parsing::to_raw(sections[14]),std::back_inserter(result.back()));
    result.back().push_back("");
    std::ranges::copy(aoc::parsing::to_raw(sections[15]),std::back_inserter(result.back()));
    return result;
  }

  std::vector<Expected> to_expecteds(aoc::parsing::Sections const& doc_sections) {
    std::cout << NL << "Log Parse:" << doc_sections.size();
    Expecteds result{};
    for (auto [sx,section] : aoc::views::enumerate(doc_sections)) {
      if (sx>= 22 and sx <= 37) {
        auto move = *(section[0].str().rbegin()+1);
        section.erase(section.begin());
        result.push_back(Expected{move,Grid{to_raw(section)}});
      }
    }
    return result;
  }

  using State = std::pair<Simulation,Expected>;

  std::ostream& operator<<(std::ostream& os,State const& state) {
    os << NL << T << "Evaluated" << T << "Expected";
    os << NL << std::make_pair(state.first.grid, state.second.grid);
    return os;
  }

  bool test0() {
    bool result{};
    Integer expected{104};
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

  bool test1(Model const& model,Expecteds const& expecteds) {
    bool result{true};
    std::cout << NL << "test1";
    auto starts = model.grid.find_all('@');
    if (starts.size()==1) {
      auto start = starts[0];
      Simulation curr{start,model.grid};
      for (int i=0;i<expecteds.size()-1;++i) {
        std::cout << NL << NL << "step[" << i << "]";
        if (i>0) std::cout << NL << T << "After move " << model.moves[i-1];
        State state{curr,expecteds[i]};
        std::cout << NL << state;
        result = result and (curr.grid == expecteds[i].grid);
        if (not result) break;
        char move = model.moves[i];
        if (move != expecteds[i+1].move) break;
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
    std::cout << NL << "test2";
    auto start = model.grid.find('@');
    Simulation curr{start,model.grid};
    for (int i=0;i<model.moves.size();++i) {
      Move move = model.moves[i];
      curr = to_next(curr,move);
    }
    std::cout << NL << NL << "ANSWER:" << to_result(curr.grid);
    return true;
  }

  std::optional<Result> solve_for(std::istream& in,Args args) {
    std::ostringstream response{};
    std::cout << NL << NL << "test";
    if (in) {
      auto model = parse(in);
      auto doc = aoc::doc::parse_doc();
      auto examples = to_examples(*doc);
      auto example_in = aoc::test::to_example_in(examples[0]);
      auto example_model = ::parse(example_in);
      std::cout << NL << NL << "example_model:" << example_model;
      auto expecteds = test::to_expecteds(*doc);
      /* Call tests here */
      if (args.arg["file"] == "example.txt") {
        if (args.arg["part"] == "test0") response << std::string(test0()?"PASSED (file ignored)":"Failed");
        if (args.arg["part"] == "test1") response << std::string(test1(model, expecteds)?" PASSED":" Failed");
        if (args.arg["part"] == "test2") response << std::string(test2(model)?"PASSED (file ignored)":"Failed");
      }
    }
    if (response.str().size()>0) return response.str();
    else return std::nullopt;
  }
}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      
      if (model.grid.width() == 8) {
        bool test_result{};
        auto doc = aoc::doc::parse_doc();
        auto expecteds = test::to_expecteds(*doc);
        test_result = test::test1(model,expecteds);
        
        if (test_result) std::cout << " passed";
        else std::cout << "FAILED";
      }
      if (model.grid.width() == 10) {
         test::test2(model);
      }
      if (model.grid.width() >= 8) {
        bool do_animate = args.options.contains("-animate");
        // Full puzzle for example and full puzzle
        auto starts = model.grid.find_all('@');
        if (starts.size()==1) {
          auto start = starts[0];
          Simulation curr{start,model.grid};
          std::cout << NL << NL << "Before:" << NL << curr;
          for (int i=0;i<model.moves.size();++i) {
            Move move = model.moves[i];
            curr = to_next(curr,move);
            if (do_animate) {
              std::cout << NL << "after move : " << move << NL << curr.grid << " " << (model.moves.size()-i) << std::flush;
              std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
          }
          result = std::to_string(to_result(curr.grid));
          std::cout << NL << NL << "After:" << NL << curr;
        }
      }
    }
    return result;
  }
}

namespace part2 {

  Grid to_expanded_grid(Grid const& part_1_grid) {
    Grid result{{}};
    std::ranges::transform(part_1_grid.base(), std::back_inserter(result.base()), [](std::string const& part_1_row) {
      std::string expanded_row{};
      for (char tile : part_1_row) {
        switch (tile) {
          case '#': expanded_row += "##"; break;
          case 'O': expanded_row += "[]"; break;
          case '.': expanded_row += ".."; break;
          case '@': expanded_row += "@."; break;
          default:  expanded_row += tile; // Default case for unsupported tiles
        }
      }
      return expanded_row;
    });
    return result;
  }

  // Axis alligned bounding box
  struct AABB {
    Position upper_left;
    int width;
    int height;
    bool occupies(Position pos) const {
      return (upper_left.col <= pos.col < upper_left.col + width)
      and (upper_left.row <= pos.row < upper_left.row + height);
    }
    bool operator<(AABB const& other) const {return std::tie(upper_left) < std::tie(other.upper_left);}
  };

  AABB to_moved(AABB const& aabb, Position const& dir) {
    return {
      {aabb.upper_left.row + dir.row, aabb.upper_left.col + dir.col},
      aabb.width,
      aabb.height
    };
  }

  bool does_overlap(const AABB& a, const AABB& b) {
    // Find the overlapping boundaries
    int overlap_left   = std::max(a.upper_left.col, b.upper_left.col);
    int overlap_right  = std::min(a.upper_left.col + a.width, b.upper_left.col + b.width);
    int overlap_top    = std::max(a.upper_left.row, b.upper_left.row);
    int overlap_bottom = std::min(a.upper_left.row + a.height, b.upper_left.row + b.height);
    
    // Check if there is no overlap
    return not (overlap_left >= overlap_right || overlap_top >= overlap_bottom);
  }

  struct Object {
    std::string caption;
    AABB aabb;
    bool is_movable() const {return caption != "#";}
    bool operator<(Object const& other) const {return aabb.upper_left<other.aabb.upper_left;}
  };
  using Objects = std::map<Position,Object>;

  Objects to_objects(Grid const& grid) {
    Objects result{};
    auto pick_object = [&result](Position pos,char ch) {
      Object obj;
      obj.aabb.upper_left = pos;
      if (ch == '[') {
        obj.caption = "[]";
        obj.aabb.width = 2;
        obj.aabb.height = 1;
      } else if (ch == '@') {
        obj.caption = "@";
        obj.aabb.width = 1;
        obj.aabb.height = 1;
      } else if (ch == '#') {
        obj.caption = "#";
        obj.aabb.width = 1;
        obj.aabb.height = 1;
      }
      else return;
      result[pos] = obj;
    };
    grid.for_each(pick_object);
    return result;
  }

  Grid to_grid(Objects const& objects) {
    auto [height,width] = std::ranges::fold_left(objects,Position{},[](Position acc,auto const& entry){
      acc.row = std::max(acc.row,entry.first.row+1); // size is one more than index
      acc.col = std::max(acc.col,entry.first.col+1);
      return acc;
    });
    Grid grid{std::vector(height,std::string(width,'.'))};
    for (auto const& [pos,object] : objects) {
      grid.at(pos) = object.caption[0];
      if (object.caption.size()==2) grid.at(pos + aoc::grid::RIGHT) = object.caption[1];
    }
    return grid;
  }

  std::vector<Object> to_connected(Objects const& objects, Object const& first, Direction const& dir) {
//    std::cout << NL << "to_connected:" << first.aabb.upper_left << " " << first.caption;
    std::vector<Object> result{};
    std::set<Position> visited; // Keep track of visited objects
    std::queue<Object> queue;   // Queue for BFS
    queue.push(first);
    while (!queue.empty()) {
      auto current = queue.front();
      queue.pop();
      auto moved = to_moved(current.aabb,dir);
      // Explore new overlapping
      for (auto const& [pos, object] : objects) {
        if (visited.find(pos) == visited.end() && does_overlap(object.aabb, moved)) {
          queue.push(object);
          visited.insert(pos);
          result.push_back(object);
//          std::cout << NL << T << "Connected: " << object.aabb.upper_left.row << ", " << object.aabb.upper_left.col
//          << " " << object.caption;
        }
      }
    }
    std::reverse(result.begin(), result.end()); // Ensure ordering back-to-front in dir order
    return result;
  }

  Objects to_next(Objects objects,Move move) {
    Objects result{objects};
    auto dir = to_direction(move);
    auto iter = std::find_if(objects.begin(), objects.end(), [](auto const& entry){
      return entry.second.caption == "@";
    });
    auto [pos,robot] = *iter;
    auto pushed = to_connected(objects, robot, dir);
    if (std::all_of(pushed.begin(), pushed.end(), [](Object const& object){
      return object.is_movable();
    })) {
      pushed.push_back(robot);
      for (auto to_push : pushed) {
        result.erase(to_push.aabb.upper_left);
        to_push.aabb.upper_left = to_push.aabb.upper_left + dir;
        result[to_push.aabb.upper_left] = to_push;
      }
    }
    return result;
  }

  Integer to_gps_coordinate(Position const& pos) {
    return 100*pos.row + pos.col;
  }

  Integer to_result(Grid const& grid) {
    Integer result{};
    auto lanterns = grid.find_all('[');
    result = std::accumulate(lanterns.begin(), lanterns.end(), result,[](auto acc,Position const& pos){
      acc += to_gps_coordinate(pos);
      return acc;
    });
    return result;
  }

  namespace test {
  
    using ::test::Expected;
    using ::test::Expecteds;

    std::vector<aoc::raw::Lines> to_examples(aoc::parsing::Sections const& sections) {
      std::vector<aoc::raw::Lines> result{};
      result.push_back({});
      // grid
      std::ranges::copy(aoc::parsing::to_raw(sections[54]),std::back_inserter(result.back()));
      result.back().push_back("");
      // moves
      std::ranges::copy(aoc::parsing::to_raw(sections[15]),std::back_inserter(result.back()));
      return result;
    }

    std::vector<Expected> to_expecteds(aoc::parsing::Sections const& doc_sections,auto config_ix,Args const& args) {
      std::cout << NL << "Log Parse:" << doc_sections.size();
      Expecteds result{};
      for (auto [sx,section] : aoc::views::enumerate(doc_sections)) {
        if (sx>= 59 and sx <= 70) {
          auto move = *(section[0].str().rbegin()+1);
          section.erase(section.begin());
          result.push_back(Expected{move,Grid{to_raw(section)}});
        }
      }
      return result;
    }
  
    std::optional<Result> test0(aoc::parsing::Sections const& doc_sections) {
      auto matches = doc_sections[71].back().groups(R"(\D*(\d+)\D+(\d+)\D+(\d+)\D+(\d+)\D*)");
      auto expected_row = std::stoi(matches[1]);
      auto expected_col = std::stoi(matches[2]);
      auto expected_gps = std::stoi(matches[3]);
      Grid grid{aoc::parsing::to_raw(doc_sections[72])};
      std::cout << NL << grid;
      auto grid_gps = part2::to_result(grid);
      std::cout << NL << T << "grid_gps:" << grid_gps << " expected:" << expected_gps;
      bool result = (to_result(grid) == expected_gps);
      if (result) return "PASSED (file ignored)";
      else return "Failed";
    }
  
    std::optional<Result> test1(aoc::parsing::Sections const& doc_sections) {
      auto matches = doc_sections[75].back().groups(R"(\D*(\d+)\D*)");
      auto expected_gps = std::stoi(matches[0]);
      Grid grid{aoc::parsing::to_raw(doc_sections[74])};
      std::cout << NL << grid;
      auto grid_gps = part2::to_result(grid);
      std::cout << NL << T << "grid_gps:" << grid_gps << " expected:" << expected_gps;
      bool result = (to_result(grid) == expected_gps);
      if (result) return "PASSED (file ignored)";
      else return "Failed";
    }

    std::optional<Result> test2(aoc::parsing::Sections const& doc_sections,Args args) {
      bool result{true};
      std::cout << NL << "test2";
      auto expecteds = test::to_expecteds(doc_sections, 0,args);
      Grid grid{aoc::parsing::to_raw(doc_sections[56])};
      auto moves = aoc::parsing::to_raw(doc_sections[57].back());
      auto expanded = to_expanded_grid(grid);
      Model model{expanded,moves};
      auto start = model.grid.find('@');
      Simulation curr{start,expanded};
      auto  objects = to_objects(expanded);
      for (int i=0;i<expecteds.size()-1;++i) {
        std::cout << NL << NL << "step[" << i << "]";
        if (i>0) std::cout << NL << T << "After move " << model.moves[i-1];
        ::test::State state{curr,expecteds[i]};
        std::cout << NL << state;
        result = result and (curr.grid == expecteds[i].grid);
        if (not result) break;
        char move = model.moves[i];
        if (move != expecteds[i+1].move) break;
        objects = part2::to_next(objects,move);
        curr.grid = to_grid(objects);
      }
      if (result) {
        std::cout << NL << T << "passed";
        std::cout << NL << NL << "ANSWER:" << to_result(curr.grid);
      }
      else std::cout << NL << T << "FAILED";

      if (result) return "PASSED";
      else return "Failed";
    }
  
    std::optional<Result> test3(aoc::parsing::Sections const& doc_sections,Args args) {
      bool result{false};
      auto do_animate = args.options.contains("-animate");
      auto groups = doc_sections[75].back().groups(R"(\D+(\d+).*)");
      auto expected_gps_sum = std::stoi(groups[0]);
      auto examples = ::test::to_examples(doc_sections);
      auto in = aoc::test::to_example_in(examples.back());
      auto model = ::parse(in);
      model.grid = part2::to_expanded_grid(model.grid);
      std::cout << NL << model;
      auto objects = to_objects(model.grid);
      for (auto move : model.moves) {
        objects = to_next(objects, move);
        if (do_animate) {
          auto inspect_grid = to_grid(objects);
          std::cout << NL << "after move: " << move << NL << inspect_grid;
//          std::cout << " : Press <Enter> to continue...";
//          std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
      }
      auto end_grid = to_grid(objects);
      auto expected_grid = Grid{aoc::parsing::to_raw(doc_sections[74])};
      aoc::test::Outcome<Grid> outcome{expected_grid,end_grid};
      std::cout << NL << outcome;
      auto sum = to_result(end_grid);
      result = (sum == expected_gps_sum);
      if (result) return "PASSED (file ignored)";
      else return "Failed";
    }


  } // namespace part2::test

  std::optional<Result> solve_for(std::istream& in,Args args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part2";
    if (args.arg["part"].starts_with("test")) {
      auto doc_sections = aoc::doc::parse_doc();
      if (args.arg["part"] == "test0_p2") return part2::test::test0(*doc_sections);
      if (args.arg["part"] == "test1_p2") return part2::test::test1(*doc_sections);
      if (args.arg["part"] == "test2_p2") return part2::test::test2(*doc_sections,args);
      if (args.arg["part"] == "test3_p2") return part2::test::test3(*doc_sections,args);
    }
    else if (in) {
//      auto do_animate = args.options.contains("-animate");
      auto do_animate = true;
      auto model = parse(in);
      model.grid = part2::to_expanded_grid(model.grid);
      std::cout << NL << model.grid;
      auto objects = to_objects(model.grid);
      for (auto [mx,move] : aoc::views::enumerate(model.moves)) {
        objects = to_next(objects, move);
        if (do_animate) {
          auto inspect_grid = to_grid(objects);
          std::cout << NL << "after move: " << move << NL << inspect_grid << " " << model.moves.size()-mx << std::flush;
//          std::cout << " : Press <Enter> to continue...";
//          std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        else {
          static int loop_count{};
          if (loop_count++ % 444 == 0) std::cout << NL << model.moves.size()-mx << std::flush;
        }
      }
      auto end_grid = to_grid(objects);
      auto sum = to_result(end_grid);
      std::cout << NL << "gps coordinates sum : " << sum;
      result = std::to_string(sum);
    }
    return result;
  }
}

int main(int argc, char *argv[]) {
  aoc::application app{};
  app.add_to_examples(test::to_examples);
  app.add_solve_for("test0",test::solve_for,"example.txt");
  app.add_solve_for("test1",test::solve_for,"example.txt");
  app.add_solve_for("test2",test::solve_for,"example.txt");
  app.add_solve_for("1",part1::solve_for,"example.txt");
  app.add_solve_for("1",part1::solve_for,"example1.txt");
  app.add_solve_for("1",part1::solve_for,"puzzle.txt");
  app.add_solve_for("test0_p2",part2::solve_for,"example.txt");
  app.add_solve_for("test1_p2",part2::solve_for,"example.txt");
  app.add_solve_for("test2_p2",part2::solve_for,"example.txt");
  app.add_solve_for("test3_p2",part2::solve_for,"example.txt");
  app.add_solve_for("2",part2::solve_for,"example.txt");
  app.add_solve_for("2",part2::solve_for,"example1.txt");
  app.add_solve_for("2",part2::solve_for,"puzzle.txt");
  app.run(argc, argv);
  app.print_result();
  /*

   Xcode Debug -O2

   >day_16 -all
   
   ANSWERS
   duration:5ms answer[part test0 in:example.txt] PASSED (file ignored)
   duration:5ms answer[part test1 in:example.txt]  PASSED
   duration:4ms answer[part test2 in:example.txt] PASSED (file ignored)
   duration:3ms answer[part 1 in:example.txt] 2028
   duration:0ms answer[part 1 in:example1.txt] 10092
   duration:7ms answer[part 1 in:puzzle.txt] 1471826
   duration:1ms answer[part test0_p2 in:example.txt] PASSED (file ignored)
   duration:1ms answer[part test1_p2 in:example.txt] PASSED (file ignored)
   duration:4ms answer[part test2_p2 in:example.txt] PASSED
   duration:61ms answer[part test3_p2 in:example.txt] PASSED (file ignored)
   duration:21ms answer[part 2 in:example.txt] 1751
   duration:1205ms answer[part 2 in:example1.txt] 9021
   duration:59730ms answer[part 2 in:puzzle.txt] 1457703
   
   */
  return 0;

}
