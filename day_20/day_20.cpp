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
using Model = aoc::grid::Grid;
using aoc::grid::operator<<;

Model parse(auto& in) {
  using namespace aoc::parsing;
  Model result{};
  auto lines = Splitter{in}.lines();
  for (auto const& [lx,line] : aoc::views::enumerate(lines)) {
    std::cout << NL << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
    result.push_back(line);
  }
  return result;
}

using Args = std::vector<std::string>;

using aoc::grid::Position;
using aoc::grid::Positions;
using aoc::grid::Grid;
using PositionPair = std::pair<Position,Position>;
using PositionPairs = std::vector<PositionPair>;
using aoc::grid::Path;

namespace test {

  // Adapt to expected for day puzzle
  struct Expected {
    bool operator==(Expected const& other) const {
      bool result{true};
      return result;
    }
  };

  std::ostream& operator<<(std::ostream& os,Expected const& entry) {
    return os;
  }

  using Expecteds = aoc::test::Expecteds<Expected>;

  Expecteds parse(auto& doc_in) {
    std::cout << NL << T << "test::parse";
    Expecteds result{};
    using namespace aoc::parsing;
    auto sections = Splitter{doc_in}.same_indent_sections();
    for (auto const& [sx,section] : aoc::views::enumerate(sections)) {
      std::cout << NL << "---------- section " << sx << " ----------";
      for (auto const& [lx,line] : aoc::views::enumerate(section)) {
        std::cout << NL << T << T << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
      }
    }
    return result;
  }

  std::optional<Result> test0(Args args) {
    std::cout << NL << NL << "test0";
    return std::nullopt;
  }

  std::size_t to_saving(Path track,PositionPair cheat) {
    std::size_t best{track.size()};

    auto dir = cheat.second - cheat.first;
    auto cut = cheat.first - dir;
    auto cut_iter = std::find(track.begin(),track.end(),cut);

    auto cheat_iter = std::find(track.begin(),track.end(),cheat.second);

//    auto start_to_cut = std::distance(track.begin(), cut_iter);
    auto cut_to_cheat = std::distance(cut_iter, cheat_iter);
//    auto cheat_to_end = std::distance(cheat_iter, track.end());
    return (cut_to_cheat>0)?cut_to_cheat-2:0;
  }

  std::ostream& operator<<(std::ostream& os,PositionPair const& pp) {
    std::cout << "{" << pp.first << "," << pp.second << "}";
    return os;
  }
  std::ostream& operator<<(std::ostream& os,PositionPairs const& pps) {
    for (auto const& [ix,pp] : aoc::views::enumerate(pps)) {
      std::cout << NL << T << std::format("[{}]",ix) << pp;
    }
    return os;
  }
  std::pair<Path,PositionPairs> to_cheats(Grid const& grid) {
    std::pair<Path,PositionPairs> result{};
    auto start = grid.find_all('S')[0];
    auto end = grid.find_all('E')[0];
//    std::cout << NL << start << " to " << end;

    Grid::Seen seen{};
    Path track{};
    std::deque<Position> q{};
    q.push_back(start);
    while (not q.empty()) {
      auto curr = q.front(); // BFS
      q.pop_front();
      track.push_back(curr);
      if (curr == end) break;
      seen.insert(curr); // no backtrack
      for (auto const& dir : aoc::grid::ortho_directions) {
        auto cand = curr + dir;
        if (not grid.on_map(cand)) continue;
        if (grid.at(cand) == '#') {
          // Cheat?
          auto cheat = cand + dir;
          if (not grid.on_map(cheat)) continue;
          if (grid.at(cheat) == '#') continue;
          if (seen.contains(cheat)) continue; // no back track
          result.second.push_back({cand,cheat});
          continue;
        }
        if (seen.contains(cand)) continue;
        q.push_back(cand);
      }
    }
    result.first = track;
    return result;
  }

  std::optional<Result> test1(auto& in, auto& doc_in,Args args) {
    std::optional<Result> result{};
    Integer acc{};
    std::cout << NL << NL << "test1";
    if (in) {
      auto model = ::parse(in);
      if (doc_in) {
        auto expecteds = test::parse(doc_in);
      }
      std::cout << NL << model;
      auto [track,cheats] = to_cheats(model);
      auto computed = aoc::grid::to_dir_traced(model, track);
      std::cout << NL << computed;
      
//      std::cout << NL << cheats;
      std::map<std::size_t,PositionPairs> savings{};
      for (auto const& cheat : cheats) {
        auto saving = to_saving(track, cheat);
        std::cout << NL << cheat << " saving:" << saving;
        savings[saving].push_back(cheat);
      }
      for (auto const& [saving,cheats] : savings) {
        std::cout << NL << "There are " << cheats.size() << " cheats saving " << saving << " picoseconds.";
        if (saving >= 100) acc += cheats.size();
      }
      result = std::to_string(acc);
    }
    return result;
  }

}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      Integer acc{};
      auto model = parse(in);
      bool is_example = (model.height()==15);
      std::cout << NL << model;
      auto [track,cheats] = test::to_cheats(model);
      if (is_example) {
        auto computed = aoc::grid::to_dir_traced(model, track);
        std::cout << NL << computed;
      }
      std::map<std::size_t,PositionPairs> savings{};
      for (auto const& cheat : cheats) {
        using test::operator<<;
        auto saving = test::to_saving(track, cheat);
//
        savings[saving].push_back(cheat);
      }
      int gain_requirement{1};
      if (not is_example) gain_requirement = 100;
      for (auto const& [saving,cheats] : savings) {
        if (is_example) std::cout << NL << "There are " << cheats.size() << " cheats saving " << saving << " picoseconds.";
        if (saving >= gain_requirement) {
          acc += cheats.size();
        }
      }
      result = std::to_string(acc);
    }
    return result;
  }
}

namespace part2 {

  std::size_t to_large_enough_cheats(Grid const& grid,int gain_requirement) {
    std::size_t result{};
    auto start = grid.find_all('S')[0];
    auto end = grid.find_all('E')[0];

    // Create the track
    Grid::Seen seen{};
    Path track{};
    std::deque<Position> q{};
    q.push_back(start);
    while (not q.empty()) {
      auto curr = q.front(); // BFS
      q.pop_front();
      track.push_back(curr);
      if (curr == end) break;
      seen.insert(curr); // no backtrack
      for (auto const& dir : aoc::grid::ortho_directions) {
        auto cand = curr + dir;
        if (not grid.on_map(cand)) continue; // no outside map
        if (grid.at(cand) == '#') continue; // no outside track
        if (seen.contains(cand)) continue; // no back track
        q.push_back(cand);
      }
    }
    // Every position on track can cheat to another track position at a manhattan position 2..20
    std::map<std::size_t,std::size_t> counts{};
    std::cout << NL << std::flush;
    for (int r=2;r<=20;++r) {
      std::cout << '.' << std::flush;
      for (int i=0;i<track.size();++i) {
//        std::cout << NL << "r:" << r << " i:" << i;
        auto from = track[i];
        for ( auto cheat_iter = track.begin()+i
             ;cheat_iter != track.end()
             ;cheat_iter = std::find_if(cheat_iter+1,track.end(),[r,&from](Position const& to){
                return ((std::abs(to.row-from.row) + std::abs(to.col - from.col))==r);
             })) {
          // cheat candidate at distance r
          auto j = std::distance(track.begin(),cheat_iter);
          auto gain = (j - i) - r;
//          std::cout << NL << r << " :: " << i << ":" << from << " -- " << gain << " --> " << j << ":" << *cheat_iter;
          if (gain >= gain_requirement) {
            counts[gain] += 1;
//            std::cout << NL << "r:" << r << " " << i << ":" << from << " -- " << gain << " --> " << j << ":" << *cheat_iter << " count:" << count;
          }
        }
      }
    }
    if (grid.width()==15) {
      // Example input
      for (auto const& [gain,count] : counts) std::cout << NL << "There are " << count << " cheats that gain that save " << gain << " picosecond";
    }
    result = std::accumulate(counts.begin(), counts.end(), std::size_t{},[](auto acc,auto const& entry){
      acc += entry.second;
      return acc;
      
    });
    return result;
  }

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part2";
    if (in) {
      Integer acc{};
      auto model = parse(in);
      std::cout << NL << model;
      int gain_requirement{50};
      if (model.height()>15) gain_requirement = 100;
      auto count = to_large_enough_cheats(model,gain_requirement);
      result = std::to_string(count);
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
  std::vector<int> states = {11,10,21,20};
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
      case 101: {
        auto doc_file = aoc::to_working_dir_path("doc.txt");
        std::ifstream doc_in{doc_file};
        auto file = aoc::to_working_dir_path("puzzle.txt");
        std::ifstream in{file};
        if (in and doc_in) answers.push_back({"Part 1 Test Example vs Log",test::test1(in,doc_in,args)});
        else std::cerr << "\nSORRY, no file " << file << " or doc_file " << doc_file;
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
   duration:3ms answer[Part 1 Example] 44
   duration:593ms answer[Part 1     ] 1497
   duration:1ms answer[Part 2 Example] 285
   duration:5468ms answer[Part 2     ] 1030809
   
  */
  return 0;
}
