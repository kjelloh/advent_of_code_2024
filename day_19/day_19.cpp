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
#include <unordered_map>
#include <unordered_set>

using aoc::raw::NL;
using aoc::raw::T;
using aoc::raw::NT;

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = aoc::raw::Line;
using Towel = aoc::raw::Line;
using Towels = aoc::raw::Lines;
using Design = aoc::raw::Line;
using Designs = aoc::raw::Lines;

struct Model {
  Towels towels{};
  Designs designs{};
};

template <>
struct std::formatter<Model> : std::formatter<std::string> {
  template<class FmtContext>
  FmtContext::iterator format(Model const& model, FmtContext& ctx) const {
    std::format_to(ctx.out(),"model:\n\ttowels:{}\n\tdesigns:{}",model.towels,model.designs);
    return ctx.out();
  }
};

using aoc::raw::operator<<;

Model parse(auto& in) {
  using namespace aoc::parsing;
  Model result{};
  auto sections = Splitter{in}.sections();
  for (auto const& [sx,section] : aoc::views::enumerate(sections)) {
//    std::cout << NL << "---------- section " << sx << " ----------";
    for (auto const& [lx,line] : aoc::views::enumerate(section)) {
//      std::cout << NL << T << T << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
      if (sx==0) {
        for (auto const& towel : line.splits(',')) {
          result.towels.push_back(towel.trim());
        }
      }
      else if (sx==1) {
        result.designs.push_back(line);
      }
      else std::cerr << NL << "DESIGN INSUFFICIENCY: Section " << sx << " not expected nor processed";
    }
  }
  return result;
}

namespace test {

  using ExpectedDesignTowels = std::pair<aoc::raw::Line,aoc::raw::Lines>;

  std::ostream& operator<<(std::ostream& os,ExpectedDesignTowels const& expected) {
    os << "expected:" << std::quoted(expected.first);
    if (expected.second.size()>0) {
      os << " from:";
      for (auto const& [ix,line] : aoc::views::enumerate(expected.second)) {
        os << " " << ix << ":" << line;
      }
    }
    else {
      os << " impossible";
      
    }
    return os;
  }

  struct Expected {
    Towels available_towels{};
    std::vector<ExpectedDesignTowels> expected_design_towels_v{};
  };

  std::ostream& operator<<(std::ostream& os,Expected const& entry) {
    std::cout << NL << "Expected:";
    std::cout << NL << T << "available towels:" << std::format("{}",entry.available_towels);
    for (auto const& expected_design_towels : entry.expected_design_towels_v){
      std::cout << NL << T << expected_design_towels;
    }
    return os;
  }

  using Expecteds = aoc::test::Expecteds<Expected>;

  ExpectedDesignTowels to_expected_design_towels(const std::string& line) {
    std::string design;
    std::vector<std::string> towels;
    
    // Regular expression for matching the design (first word made up of w,u,b,r,g)
    std::regex designRegex("\\b[wbgru]+\\b");
    std::smatch designMatch;
    
    // Extract the design
    std::string remainingText = line;
    if (std::regex_search(remainingText, designMatch, designRegex)) {
      design = designMatch.str();
      // Remove the design from the text to prevent it from being matched again
      remainingText = designMatch.suffix().str();
    }
    
    // Regular expression for matching towel descriptions
    std::regex towelRegex("\\b(a|an|two|three|four|five)?\\s?([wbgru]+)\\b");
    std::smatch towelMatch;
    
    // Find all towel matches in the remaining text
    while (std::regex_search(remainingText, towelMatch, towelRegex)) {
      std::string towel = towelMatch.str(2); // Group 2 is the towel code
      towels.push_back(towel);
      if (towelMatch[1] == "two") {
        towels.push_back(towel);
      }
      
      // Move past this match
      remainingText = towelMatch.suffix().str();
    }
    
    return {design, towels};
  }

  std::vector<aoc::raw::Lines> to_examples(aoc::parsing::Sections const& sections) {
    std::vector<aoc::raw::Lines> result{};
    result.push_back({});
    result.back().append_range(aoc::parsing::to_raw(sections[13]));
    result.back().push_back("");
    result.back().append_range(aoc::parsing::to_raw(sections[14]));
    return result;
  }

  using Vertex = int;
  using Weight = std::string;
  // Graph with edges weighted with the towel used (consumed)
  using Adjacent = std::set<std::pair<Vertex,Weight>>;
  using Graph = std::unordered_map<Vertex, std::vector<std::pair<Vertex,Weight>>>;

  Graph to_graph(const std::string& design, const std::vector<std::string>& towels) {
    Graph graph;

    struct State {
      Vertex pos;
      std::vector<std::string> path;
      bool operator<(State const& other) const {
        return std::tie(pos, path) < std::tie(other.pos, other.path);
      }
    };
    using Seen = std::set<State>;

    std::deque<State> queue;
    Seen seen{};

    State start{0,{}};
    queue.push_front(start);
    while (!queue.empty()) {
      State state = queue.front();
      queue.pop_front();
      seen.insert(state); // Do not revisit
      auto const& [current,path] = state;
      if (current == design.size()-1) {
        graph[current]; // Add the end vertex
        break; // One end is enough for now
      }
      for (const auto& towel : towels) {
        if (current + towel.size() > (design.size()-1)) continue; // towel don't fit (redundant?)
        if (design.substr(current, towel.size()) != towel) continue; // towel don't fit
        Vertex next = current + static_cast<int>(towel.size());

        auto next_path = path;
        next_path.push_back(towel);
        State next_state{next,next_path};
        if (seen.contains(next_state)) continue; // towel-step with same path already in graph

        graph[current].push_back({next, towel});
        queue.push_front(next_state); // Explore the next position
      }
    }
    return graph;
  }

  using Path = std::vector<Vertex>;
  std::vector<Path> to_paths(const Graph& graph, Vertex start, Vertex end) {
    std::vector<Path> allPaths;
    std::stack<std::pair<Vertex, Path>> stack;
    std::set<Path> seen{};
    
    stack.push({start, {start}});
    while (!stack.empty()) {
      auto [current, path] = stack.top(); // a path to current
      stack.pop();
      if (current == end) {
        allPaths.push_back(path);
        continue;
      }
      seen.insert(path); // process each path only once
      if (not graph.contains(current)) break;
      for (const auto& [neighbor, weight] : graph.at(current)) {
        std::vector<Vertex> newPath = path;
        newPath.push_back(neighbor);
        if (seen.contains(newPath)) continue;
        stack.push({neighbor, newPath});
      }

    }
    return allPaths;
  }

  bool test0(std::optional<aoc::parsing::Sections> const& opt_sections,Args args) {
    // This function is called by aoc::application if registered with add_test(test::test0)
    // Extract test data from provided sections from the day web page text.
    // See zsh-script pull_text.zsh for support to fetch advent of code day web page text to doc.txt
    std::cout << NL << "test0";
    if (opt_sections) {
      auto const& sections = *opt_sections;
      std::cout << NL << T << "sections ok";
      auto examples = to_examples(sections);
      if (examples.size()>0) {
        bool result{};
        std::cout << NL << T << "examples ok";
        auto example_in = aoc::test::to_example_in(examples[0]);
        auto example_model = parse(example_in);
        std::cout << NL << std::format("\n{}",example_model);
        
        auto line = aoc::parsing::to_line(sections[18]).str();
        line += aoc::parsing::to_line(sections[19]).str();
        auto expected = to_expected_design_towels(line);
                
        auto const& available_towels = example_model.towels;
        std::cout << NL << T << "available towels:" << std::format("{}",available_towels);
        using test::operator<<;
        std::cout << NL << T << expected;
        auto design = expected.first;
        auto graph = to_graph(design, available_towels);
        auto paths = to_paths(graph, 0, static_cast<Vertex>(design.size()-1));
        std::cout << NL << "paths:" << paths.size();
        if (paths.size()>0) {
          std::print(" {}",paths);
          return true;
        }
        else {
          std::cout << " impossible";
        }
        result = false;;
      }
      else {
        std::cout << NL << T << "NO examples";
      }
    }
    else {
      std::cout << NL << T << "NO sections";
    }
    return false;
  }

  bool test1(std::optional<aoc::parsing::Sections> const& opt_sections,Args args) {
    std::cout << NL << "test0";
    if (opt_sections) {
      auto const& sections = *opt_sections;
      std::cout << NL << T << "sections ok";
      auto examples = to_examples(sections);
      if (examples.size()>0) {
        bool result{true};
        std::cout << NL << T << "examples ok";
        auto example_in = aoc::test::to_example_in(examples[0]);
        auto example_model = parse(example_in);
        std::cout << NL << std::format("\n{}",example_model);
        
        std::vector<ExpectedDesignTowels> expected_design_towels_v{};

        auto line = aoc::parsing::to_line(sections[18]).str();
        line += aoc::parsing::to_line(sections[19]).str();
        expected_design_towels_v.push_back(to_expected_design_towels(line));
        for (auto const& line : sections[20]) {
          expected_design_towels_v.push_back(to_expected_design_towels(line));
        }
                
        auto const& available_towels = example_model.towels;
        Integer acc{};
        std::cout << NL << "available towels:" << std::format("{}",available_towels);
        for (auto const& expected_design_towels : expected_design_towels_v) {
          std::cout << NL << NL << std::format("expected: {}",expected_design_towels);
          auto design = expected_design_towels.first;
          auto graph = to_graph(design, available_towels);
          auto paths = to_paths(graph, 0, static_cast<Vertex>(design.size()-1));
          std::cout << NL << NL << T;
          if (paths.size()>0) {
            result = result and (0 < expected_design_towels.second.size());
            std::print(" {} matches {}? {}",paths,expected_design_towels.second,result);
            ++acc; // possible
          }
          else {
            result = result and (0 == expected_design_towels.second.size());
            std::print(" {} matches {}? {}",paths,expected_design_towels.second,result);
          }
        }
        std::print("\nacc:{}",acc);
        result = result and (acc == 6);
        return result;
      }
      else {
        std::cout << NL << T << "NO examples";
      }
    }
    else {
      std::cout << NL << T << "NO sections";
    }
    return false;
  }


  std::optional<Result> test0_old(Towels const&  towels,ExpectedDesignTowels const& expected) {
    Integer count{};
    std::cout << NL << NL << "test0:" << expected;
    using aoc::raw::operator<<;
    std::cout << NL << T << "available towels:" << towels;
    using test::operator<<;
    std::cout << NL << T << "expected:" << expected;
    auto design = expected.first;
    auto graph = to_graph(design, towels);
    auto paths = to_paths(graph, 0, static_cast<Vertex>(design.size()-1));
    std::cout << NL << "paths:" << paths.size();
    if (paths.size()>0) {
      return std::to_string(paths.size());
    }
    else {
      std::cout << " impossible";
    }
    return std::nullopt;
  }

}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      Integer acc{};
      auto model = parse(in);
      for (auto const& [dx,design] : aoc::views::enumerate(model.designs)) {
        std::cout << NL << "processing design[" << dx << "]:" << design << std::flush;
        auto graph = test::to_graph(design, model.towels);
        std::cout << NL << "design[" << dx << "]:" << std::quoted(design);
        if (graph.contains(static_cast<test::Vertex>(design.size()-1))) {
          ++acc;
          std::cout  << " OK : " << acc << std::flush;
        }
        else {
          std::cout  << " Impossible";
        }
      }
      result = std::to_string(acc);
    }
    return result;
  }
}

namespace part2 {

  // This is a "Word Break Problem" or "String Reconstruction from Segments"
  // 'design' is 'the word' to assemble
  // 'towels' are 'the snippets' from which to assemble 'the word' (design)
  std::size_t to_combination_count(const std::string &word, const std::vector<std::string> &snippets) {
    auto const N = word.size();
    std::vector<std::size_t> dp(N + 1, 0);
    dp[0] = 1; // Base case: one way to form an empty string.
    
    for (int pos = 1; pos <= N; ++pos) {
      for (const std::string &snippet : snippets) {
        auto len = snippet.size();
        if (pos >= len && word.substr(pos - len, len) == snippet) {
          dp[pos] += dp[pos - len]; // add history count
        }
      }
    }
    return dp[N]; // The number of ways to form the entire word.
  }

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part2";
    if (in) {
      Integer acc{};
      auto model = parse(in);
      for (auto const& [dx,design] : aoc::views::enumerate(model.designs)) {
        std::cout << NL << "processing design[" << dx << "]:" << design << std::flush;
        // This is a "Word Break Problem" or "String Reconstruction from Segments"
        auto count = to_combination_count(design,model.towels);
        acc += count;
        std::cout << " -> " << count << " acc:" << acc;
      }
      result = std::to_string(acc);
    }
    return result;
  }
}

int main(int argc, char *argv[]) {
  aoc::application app{};
  app.add_to_examples(test::to_examples);
  app.add_test("test0",test::test0);
  app.add_test("test1",test::test1);
  app.add_solve_for("1",part1::solve_for,"example.txt");
  app.add_solve_for("1",part1::solve_for,"puzzle.txt");
  app.add_solve_for("2",part2::solve_for,"example.txt");
  app.add_solve_for("2",part2::solve_for,"puzzle.txt");
  app.run(argc, argv);
  app.print_result();
  /*

   Xcode Debug -O2

   >day_19 -all
   
   ANSWERS
   duration:1ms answer[part:"test0"] PASSED
   duration:3ms answer[part:"test1"] PASSED
   duration:0ms answer[part 1 in:example.txt] 6
   duration:230ms answer[part 1 in:puzzle.txt] 276
   duration:0ms answer[part 2 in:example.txt] 16
   duration:490ms answer[part 2 in:puzzle.txt] 681226908011510

   */

}
