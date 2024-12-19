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
using Towels = aoc::raw::Lines;
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
    std::cout << NL << "---------- section " << sx << " ----------";
    for (auto const& [lx,line] : aoc::views::enumerate(section)) {
      std::cout << NL << T << T << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
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
  std::cout << NL << "result:";
  std::cout << NL << T << result.towels;
  std::cout << NL << T << result.designs;
  return result;
}

using Args = std::vector<std::string>;

namespace test {

  using Expected = std::pair<aoc::raw::Line,aoc::raw::Lines>;
  std::ostream& operator<<(std::ostream& os,Expected const& expected) {
    os << "expected:" << std::quoted(expected.first) << " from:";
    for (auto const& [ix,line] : aoc::views::enumerate(expected.second)) {
      std::cout << " " << ix << ":" << line;
    }
    return os;
  }
  // Adapt to expected for day puzzle
  struct LogEntry {
    Model example{};
    std::vector<Expected> expecteds{};
    bool operator==(LogEntry const& other) const {
      bool result{true};
      return result;
    }
  };

  std::ostream& operator<<(std::ostream& os,LogEntry const& entry) {
    std::cout << NL << "LogEntry:";
    std::cout << NL << T << entry.example;
    for (auto const& expected : entry.expecteds){
      std::cout << NL << T << expected;
      
    }
    return os;
  }

  using LogEntries = aoc::test::LogEntries<LogEntry>;

  Expected to_expected(const std::string& line) {
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

  LogEntries parse(auto& doc_in) {
    std::cout << NL << T << "test::parse";
    LogEntries result{};
    using namespace aoc::parsing;
    if (doc_in) {
      auto sections = Splitter{doc_in}.same_indent_sections();
      LogEntry entry{};
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
                auto expected = to_expected(repaired);
                entry.expecteds.push_back(expected);
                repaired.clear();
              }
              auto expected = to_expected(line);
              entry.expecteds.push_back(expected);
            } break;
            case 21: {} break;
          }
        }
      }
      result.push_back(entry);
    }
    return result;
  }

  // A structure to represent a graph vertex.
  struct Vertex {
      std::string towel;
      int position;
      bool operator==(const Vertex& other) const {
          return towel == other.towel && position == other.position;
      }
  };

  // Hash function for the Vertex structure.
  struct VertexHash {
      std::size_t operator()(const Vertex& v) const {
          return std::hash<std::string>()(v.towel) ^ std::hash<int>()(v.position);
      }
  };

  // BFS function to generate the graph.
  using Graph = std::unordered_map<Vertex, std::vector<Vertex>, VertexHash>;
  Graph bfs_generate_graph(const std::string& design, const std::vector<std::string>& towels) {
    Graph graph;
    std::queue<Vertex> queue;
    std::unordered_set<Vertex, VertexHash> visited;
    queue.push({"", 0});
    visited.insert({"", 0});
    
    while (!queue.empty()) {
      Vertex current = queue.front();
      queue.pop();
      int current_pos = current.position;
      for (const auto& towel : towels) {
        if (design.substr(current_pos, towel.size()) == towel) {
          Vertex next = {towel, current_pos + static_cast<int>(towel.size())};
          graph[current].push_back(next);
          if (visited.find(next) == visited.end()) {
            queue.push(next);
            visited.insert(next);
          }
        }
      }
    }
    std::cout << NL << "Generated Graph:";
    for (const auto& [vertex, neighbors] : graph) {
      std::cout << NL << "Vertex (Towel: " << vertex.towel << ", Position: " << vertex.position << ") -> [";
      for (const auto& neighbor : neighbors) {
        std::cout << " (Towel: " << neighbor.towel << ", Position: " << neighbor.position << ")";
      }
    }
    return graph;
  }

  bool fully_matched(std::string const& design,Graph const& graph) {
    return std::any_of(graph.begin(), graph.end(), [&design](auto const& entry){
      return (entry.first.position == (design.size()-1));
    });
  }

  std::optional<Result> test0(Towels const&  towels,Expected const& expected) {
    std::cout << NL << NL << "test0:" << expected;
    using aoc::raw::operator<<;
    std::cout << NL << T << "available towels:" << towels;
    using test::operator<<;
    std::cout << NL << T << "expected:" << expected;
    auto graph = bfs_generate_graph(expected.first, towels);
    if (fully_matched(expected.first,graph)) {
      std::cout << NL << "MATCH
      ";
    }
    else {
      std::cout << NL << "IMPOSSIBLE";
    }
    return std::nullopt;
  }

  std::optional<Result> test1(auto& doc_in,Args args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "test1";
    if (doc_in) {
      auto entries = test::parse(doc_in);
      if (entries.size()==1) {
        auto const& entry = entries.back();
        auto const& towels = entry.example.towels;
        for (auto const& expected : entry.expecteds) {
          test0(towels,expected);
        }
      }
      else {
        std::cerr << NL << "Sorry, expected only one entry to test from parsing doc.txt";
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
  std::vector<int> states = {111
  };
  for (auto state : states) {
    switch (state) {
      case 111: {
        auto doc_file = aoc::to_working_dir_path("doc.txt");
        std::ifstream doc_in{doc_file};
        if (doc_in) answers.push_back({"Part 1 Test Example vs Log",test::test1(doc_in,args)});
        else std::cerr << "\nSORRY, no doc file " << doc_file;
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
