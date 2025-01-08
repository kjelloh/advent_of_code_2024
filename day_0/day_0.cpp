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
#include <expected>

using aoc::raw::NL;
using aoc::raw::T;
using aoc::raw::NT;

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = aoc::raw::Line;
struct Model {
  aoc::raw::Lines entries;
};

std::ostream& operator<<(std::ostream& os,Model const& model) {
  os << NL << "Model:";
  using aoc::raw::operator<<;
  os << NL << model.entries;
  return os;
}

Model parse(auto& in) {
  using namespace aoc::parsing;
  Model result{};
  auto sections = Splitter{in}.sections();
  for (auto const& [sx,section] : aoc::views::enumerate(sections)) {
    std::cout << NL << "---------- section " << sx << " ----------";
    for (auto const& [lx,line] : aoc::views::enumerate(section)) {
      std::cout << NL << T << T << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
      result.entries.push_back(line);
    }
  }
  return result;
}

namespace test {

  std::vector<aoc::raw::Lines> to_examples(aoc::parsing::Sections const& sections) {
    std::vector<aoc::raw::Lines> result{};
    // for each example to parse
    // example0 (from 2024 day 17)
    result.push_back({});
    result.back().append_range(aoc::parsing::to_raw(sections[37]));
    result.back().push_back("");
    result.back().append_range(aoc::parsing::to_raw(sections[38]));
    // example1 (from 2024 day 17)
    result.push_back({});
    result.back().append_range(aoc::parsing::to_raw(sections[46]));
    result.back().push_back("");
    result.back().append_range(aoc::parsing::to_raw(sections[47]));
    return result;
  }

  bool test0(std::optional<aoc::parsing::Sections> const& sections,Args args) {
    // This function is called by aoc::application if registered with add_test(test::test0)
    // Extract test data from provided sections from the day web page text.
    // See zsh-script pull_text.zsh for support to fetch advent of code day web page text to doc.txt
    return false;
  }

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::ostringstream response{};
    std::cout << NL << NL << "test";
    auto model = parse(in);
    std::cout << NL << NL << "model from in " << NL << model;
    /* Call tests here as apropriate for parsed model) */
    // ...
    
    if (response.str().size()>0) return response.str();
    else return std::nullopt;
  }

} // namespace test

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::ostringstream response{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
    }
    if (response.str().size()>0) return response.str();
    else return std::nullopt;
  }
}

namespace part2 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::ostringstream response{};
    std::cout << NL << NL << "part2";
    if (in) {
      auto model = parse(in);
    }
    if (response.str().size()>0) return response.str();
    else return std::nullopt;
  }
}

int main(int argc, char *argv[]) {
  
  aoc::application app{};
  app.add_to_examples(test::to_examples);
  app.add_test("test0",test::test0);
  app.add_solve_for("1",part1::solve_for,"example.txt");
  app.add_solve_for("1",part1::solve_for,"puzzle.txt");
  app.add_solve_for("2",part2::solve_for,"example.txt");
  app.add_solve_for("2",part2::solve_for,"puzzle.txt");
  app.run(argc, argv);
  app.print_result();
  /*

   Xcode Debug -O2

   >day_13 -all
   
   For my input:
            
   ANSWERS
   duration:84ms answer[part 1 in:example.txt] 480
   duration:3036ms answer[part 1 in:puzzle.txt] 39290
   duration:0ms answer[part 2 in:example.txt] 875318608908
   duration:18ms answer[part 2 in:puzzle.txt] 73458657399094

   */

}
