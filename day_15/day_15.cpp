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
    bool operator==(Expected const& other) const {
      bool result{true};
      return result;
    }
  };

  std::ostream& operator<<(std::ostream& os,Expected const& entry) {
    os << "Logged Move " << entry.move << ":";
    os << NL << entry.grid;
    return os;
  }

  using Expecteds = aoc::test::Expecteds<Expected>;

  aoc::parsing::Sections parse_doc(Args const& args) {
    std::cout << NL << T << "parse puzzle doc text";
    aoc::parsing::Sections result{};
    using namespace aoc::parsing;
    std::ifstream doc_in{aoc::to_working_dir_path("doc.txt")};
    auto sections = Splitter{doc_in}.same_indent_sections();
    for (auto const& [sx,section] : aoc::views::enumerate(sections)) {
      std::cout << NL << "---------- section " << sx << " ----------";
      result.push_back(section);
      for (auto const& [lx,line] : aoc::views::enumerate(section)) {
        std::cout << NL << T << T << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
      }
    }
    return result;
  }

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

  std::vector<Expected> to_expecteds(aoc::parsing::Sections const& doc_sections,auto config_ix,Args const& args) {
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
    std::cout << NL << T << "Evaluated" << T << "Expected";
    std::cout << NL << std::make_pair(state.first.grid, state.second.grid);
    return os;
  }

  // return range to shift [begin,end[
  // The range includes the robot position
  // "##@.O..#" <
  //     |
  //     begin
  //     end

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
    std::cout << NL << "TEST1";
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
    std::cout << NL << "TEST1";
    auto starts = model.grid.find_all('@');
    if (starts.size()==1) {
      auto start = starts[0];
      Simulation curr{start,model.grid};
      for (int i=0;i<model.moves.size();++i) {
        Move move = model.moves[i];
        curr = to_next(curr,move);
      }
      std::cout << NL << NL << "ANSWER:" << to_result(curr.grid);
    }
    else {
      std::cerr << NL << "failed to find unique start position";
    }
    return true;
  }

  std::optional<Result> solve_for(std::istream& in,Args args) {
    std::ostringstream response{};
    std::cout << NL << NL << "test";
    if (in) {
      auto model = parse(in);
      auto doc = parse_doc(args);
      auto examples = to_examples(doc);
      for (auto const& [ix,example_lines] : aoc::views::enumerate(examples)) {
        if (args.options.contains("-to_example")) {
          auto example_file = aoc::to_working_dir_path(std::format("example{}.txt",ix));
          if (aoc::raw::write_to_file(example_file, example_lines)) {
            response << "Created " << example_file;
          }
          else {
            response << "Sorry, failed to create file " << example_file;
          }
        }
        else {
          std::ostringstream oss{};
          aoc::raw::write_to(oss, example_lines);
          std::istringstream example_in{oss.str()};
          auto example_model = ::parse(example_in);
          std::cout << NL << NL << "example_model:" << example_model;
          auto expecteds = test::to_expecteds(doc, ix,args);
          /* Call tests here */
          if (args.arg["part"] == "test0") return std::string(test0()?"PASSED":"Failed");
          if (args.arg["part"] == "test1") return test1(model, expecteds)?"PASSED":"Failed";
          if (args.arg["part"] == "test2") return test2(model)?"PASSED":"Failed";
        }
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
        auto doc = test::parse_doc(args);
        auto expecteds = test::to_expecteds(doc, 1, args);
        test_result = test::test1(model,expecteds);
        
        if (test_result) std::cout << " passed";
        else std::cout << "FAILED";
      }
      if (model.grid.width() == 10) {
         test::test2(model);
      }
      if (model.grid.width() >= 8) {
        // Full puzzle for example and full puzzle
        auto starts = model.grid.find_all('@');
        if (starts.size()==1) {
          auto start = starts[0];
          Simulation curr{start,model.grid};
          std::cout << NL << NL << "Before:" << NL << curr;
          for (int i=0;i<model.moves.size();++i) {
            Move move = model.moves[i];
            curr = to_next(curr,move);
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

  Objects to_next(Objects objects,Move move) {
    Objects result{objects};
    auto dir = to_direction(move);
    auto iter = std::find_if(objects.begin(), objects.end(), [](auto const& entry){
      return entry.second.caption == "@";
    });
    auto [pos,robot] = *iter;
    auto moved = to_moved(robot.aabb, dir);
    std::cout << NL << "robot:" << robot.aabb.upper_left << " moved:" << moved.upper_left;
    std::set<Object> pushed{};
    for (auto const& [pos,object] : objects) {
      if (does_overlap(object.aabb, moved)) pushed.insert(object);
    }
    for (auto const& object : pushed) std::cout << NL << T << "pushed:" << object.aabb.upper_left
      << " " << object.caption;
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
      std::cout << NL << "TEST1";
      auto doc = ::test::parse_doc(args);
      auto expecteds = test::to_expecteds(doc, 0,args);
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

      if (result) return "PASSED (file ignored)";
      else return "Failed";
    }

  } // namespace part2::test

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part2";
    if (args.options.size()>0) {
      auto doc_sections = ::test::parse_doc(args);
      for (auto option : args.options) {
        if (option == "-test0") return test::test0(doc_sections);
        if (option == "-test1") return test::test1(doc_sections);
        if (option == "-test2") return test::test2(doc_sections,args);
        return std::format("Unknown option '{}'",option);
      }
    }
    else if (in) {
      auto model = parse(in);
      Model expanded{to_expanded_grid(model.grid),model.moves};
      std::cout << NL << expanded;
    }
    return result;
  }
}

using Answers = std::vector<std::pair<std::string,std::optional<Result>>>;
std::vector<Args> to_requests(Args const& args) {
  std::vector<Args> result{};
  result.push_back(args); // No fancy for now
  return result;
}
int main(int argc, char *argv[]) {
  Args user_args{};
  
  // Override by any user input
  for (int i=1;i<argc;++i) {
    user_args.arg["file"] = "example.txt";
    std::string token{argv[i]};
    if (token.starts_with("-")) user_args.options.insert(token);
    else {
      // assume options before <part> and <file>
      auto non_option_index = i - user_args.options.size(); // <part> <file>
      switch (non_option_index) {
        case 1: user_args.arg["part"] = token; break;
        case 2: user_args.arg["file"] = token; break;
        default: std::cerr << NL << "Unknown argument " << std::quoted(token);
      }
    }
  }
  
  auto requests = to_requests(user_args);
  
  if (not user_args or user_args.options.contains("-all")) {
    requests.clear();

    std::vector<std::string> parts = {"test", "1", "2"};
    std::vector<std::string> files = {"example.txt", "puzzle.txt"};
    
    for (const auto& [part, file] : aoc::algo::cartesian_product(parts, files)) {
      if (part.starts_with("test") and file.starts_with("puzzle")) continue;
      Args args;
      args.arg["part"] = part;
      args.arg["file"] = file;
      requests.push_back(args);
    }
  }

  Answers answers{};
  std::vector<std::chrono::time_point<std::chrono::system_clock>> exec_times{};
  exec_times.push_back(std::chrono::system_clock::now());
  for (auto request : requests) {
    auto part = request.arg["part"];
    auto file = aoc::to_working_dir_path(request.arg["file"]);
    std::cout << NL << "Using part:" << part << " file:" << file;
    std::ifstream in{file};
    if (in) {
      if (part=="1") {
        answers.push_back({std::format("part{} {}",part,file.filename().string()),part1::solve_for(in,request)});
      }
      else if (part=="2") {
        answers.push_back({std::format("{} part{} {}",request.options, part,file.filename().string()),part2::solve_for(in,request)});
      }
      else if (part.starts_with("test")) {
        answers.push_back({std::format("{} {}",part,file.filename().string()),test::solve_for(in,request)});
      }
    }
    else std::cerr << "\nSORRY, no file " << file;
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

   Xcode Debug -O2

   >day_xx -all

   ANSWERS
   duration:3ms answer[Part 1 Example] 2028
   duration:2ms answer[Part 1 Example 2] 10092
   duration:15ms answer[Part 1     ] 1471826
   ...
   
   */
  return 0;
}
