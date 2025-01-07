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
std::ostream& operator<<(std::ostream& os,Model const& model) {
  os << NL << "model:";
  using aoc::raw::operator<<;
  os << NL << T << "towels:" << model.towels;
  os << NL << T << "designs:" << model.designs;
  return os;
}

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
//  std::cout << NL << "result:";
//  std::cout << NL << T << result.towels;
//  std::cout << NL << T << result.designs;
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
  // Adapt to expected for day puzzle
  struct Expected {
    Model example{};
    std::vector<ExpectedDesignTowels> expected_design_towels{};
    bool operator==(Expected const& other) const {
      bool result{true};
      return result;
    }
  };

  std::ostream& operator<<(std::ostream& os,Expected const& entry) {
    std::cout << NL << "Expected:";
    std::cout << NL << T << entry.example;
    for (auto const& expected : entry.expected_design_towels){
      std::cout << NL << T << expected;
      
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


  Expecteds to_expecteds(aoc::parsing::Sections const& sections,auto config_ix,Args const& args) {
    std::cout << NL << T << "to_expecteds";
    Expecteds result{};
    using namespace aoc::parsing;
    Expected entry{};
    std::string repaired{};
    for (auto const& [sx,section] : aoc::views::enumerate(sections)) {
      std::cout << NL << "---------- section " << sx << " ----------";
      for (auto const& [lx,line] : aoc::views::enumerate(section)) {
        std::cout << NL << T << T << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
        switch (sx) {
            /*
             ---------- section 13 ----------
                 line[0]:28 "r, wr, b, g, bwu, rb, gb, br"
             ---------- section 14 ----------
                 line[0]:5 "brwrr"
                 line[1]:4 "bggr"
                 line[2]:4 "gbbr"
                 line[3]:6 "rrbgbr"
                 line[4]:4 "ubwu"
                 line[5]:6 "bwurrg"
                 line[6]:4 "brgr"
                 line[7]:6 "bbrgwb"
             */
          case 13: {
              for (auto const& untrimmed : line.splits(',')) {
                entry.example.towels.push_back(untrimmed.trim());
              }
          } break;
          case 14: {
              entry.example.designs.push_back(line);
          } break;
          /*
           ---------- section 18 ----------
               line[0]:67 "     * brwrr can be made with a br towel, then a wr towel, and then"
           ---------- section 19 ----------
               line[0]:26 "       finally an r towel."
           ---------- section 20 ----------
               line[0]:74 "     * bggr can be made with a b towel, two g towels, and then an r towel."
               line[1]:60 "     * gbbr can be made with a gb towel and then a br towel."
               line[2]:48 "     * rrbgbr can be made with r, rb, g, and br."
               line[3]:26 "     * ubwu is impossible."
               line[4]:48 "     * bwurrg can be made with bwu, r, r, and g."
               line[5]:42 "     * brgr can be made with br, g, and r."
               line[6]:28 "     * bbrgwb is impossible."

           */
          case 18: {
            repaired = line;
          } break;
          case 19: {
            repaired += line;
          } break;
          case 20: {
            if (repaired.size()>0) {
              auto expected = to_expected_design_towels(repaired);
              entry.expected_design_towels.push_back(expected);
              repaired.clear();
            }
            auto expected = to_expected_design_towels(line);
            entry.expected_design_towels.push_back(expected);
          } break;
          case 21: {} break;
        }
      }
    }
    result.push_back(entry);

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
//    std::cout << NL << "to_paths(graph:" << graph.size() << ",..)" << std::flush;
    std::vector<Path> allPaths;
    std::stack<std::pair<Vertex, Path>> stack;
    std::set<Path> seen{};
    
    stack.push({start, {start}});
    while (!stack.empty()) {
//      std::cout << NL << stack.size() << std::flush;
      auto [current, path] = stack.top(); // a path to current
      using aoc::raw::operator<<;
      std::cout << NL << current << " " << path;
      stack.pop();
      if (current == end) {
        std::cout << NL << NL << "END!";
        allPaths.push_back(path);
        continue;
      }
      seen.insert(path); // process each path only once
      if (not graph.contains(current)) break;
      for (const auto& [neighbor, weight] : graph.at(current)) {
        using aoc::raw::operator<<;
        std::vector<Vertex> newPath = path;
        newPath.push_back(neighbor);
        if (seen.contains(newPath)) continue;
        stack.push({neighbor, newPath});
//            std::cout << NL << T << current << " --> " << neighbor << " " << weight;
      }

    }
//    std::cout << NL << "to_paths END" << std::flush;
    return allPaths;
  }

  std::optional<Result> test0(Towels const&  towels,ExpectedDesignTowels const& expected) {
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

  std::optional<Result> test1(Args args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "test1";
    Integer acc{};
    auto doc = aoc::doc::parse_doc(args);
    auto entries = test::to_expecteds(doc,0,args);
    if (entries.size()==1) {
      auto const& entry = entries.back();
      auto const& towels = entry.example.towels;
      for (auto const& expected : entry.expected_design_towels) {
        if (auto answer = test0(towels,expected)) {
          acc += 1;
        }
      }
    }
    else {
      std::cerr << NL << "Sorry, expected only one entry to test from parsing doc.txt";
    }
    result = std::to_string(acc);
    return result;
  }

  std::vector<aoc::raw::Lines> to_examples(aoc::parsing::Sections const& sections) {
    std::vector<aoc::raw::Lines> result{};
    result.push_back({});
    result.back().append_range(aoc::parsing::to_raw(sections[13]));
    result.back().push_back("");
    result.back().append_range(aoc::parsing::to_raw(sections[14]));
    return result;
  }

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::ostringstream response{};
    std::cout << NL << NL << "test";
    if (in) {
      auto model = parse(in);
      auto doc = aoc::doc::parse_doc(args);
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
          auto log = test::to_expecteds(doc, ix,args);
          /* Call tests here */
          return test1(args);
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
    
    std::vector<std::tuple<std::set<std::string>,std::string,std::string>> states{
       {{""},"test",""}
      ,{{""},"test","example.txt"}
      ,{{""},"1","example.txt"}
      ,{{""},"1","puzzle.txt"}
      ,{{""},"2","example.txt"}
      ,{{""},"2","puzzle.txt"}
    };
    
    for (const auto& [options,part, file] : states) {
      Args args;
      args.options = options;
      args.arg["part"] = part;
      if (file.size()>0) args.arg["file"] = file;
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
    if (std::filesystem::is_regular_file(file) and in) {
      if (part=="1") {
        answers.push_back({std::format("part{} {}",part,file.filename().string()),part1::solve_for(in,request)});
      }
      else if (part=="2") {
        answers.push_back({std::format("part{} {}",part,file.filename().string()),part2::solve_for(in,request)});
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

   >day_19 -all

   ANSWERS
   duration:0ms answer[test example.txt] 6
   duration:10ms answer[part1 example.txt] 6
   duration:0ms answer[part1 puzzle.txt] 276
   duration:223ms answer[part2 example.txt] 16
   duration:0ms answer[part2 puzzle.txt] 681226908011510
   
   */
  return 0;
}
