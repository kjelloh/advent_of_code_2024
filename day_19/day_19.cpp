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

  using Vertex = int;
  using Weight = std::string;
  // Graph with edges weighted with the towel used (consumed)
  using Adjacent = std::set<std::pair<Vertex,Weight>>;
  using Graph = std::unordered_map<Vertex, Adjacent>;
  Graph to_graph(const std::string& design, const std::vector<std::string>& towels,bool find_all_paths) {
    std::cout << NL << "to_graph" << std::flush;
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
      if (current == design.size()) {
        graph[current]; // Add the end vertex
        if (not find_all_paths) break; // One end is enough
        else continue; // try options still in queue
      }
      for (const auto& towel : towels) {
//        if (current + towel.size() > (design.size()-1)) continue; // towel don't fit (redundant?)
        if (design.substr(current, towel.size()) != towel) continue; // towel don't fit
//        std::cout << NL << T << "current:" << current << " consumed:" << towel;
        Vertex next = current + static_cast<int>(towel.size());

//        if (next == design.size()-1) std::cout << NL << "End candidate found";

        auto next_path = path;
        next_path.push_back(towel);
        State next_state{next,next_path};
        if (seen.contains(next_state)) continue; // towel-step with same path already in graph

//        std::cout << NL << T << "graph.add_edge:" << current << " --> " << next << " consumed: << " << towel;

        graph[current].insert({next, towel});
        graph[next]; // Ensure orphan end is in graph
        queue.push_front(next_state); // Explore the next position
//        std::cout << NL << "design:" << design.size() << " next:" << next << " towel:" << towel << " new queue:" << queue.size();
      }
    }
    std::cout << NL << "to_graph END" << std::flush;
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
      stack.pop();
      if (current == end) {
        std::cout << NL << NL << "END!";
        allPaths.push_back(path);
        continue;
      }

      if (seen.contains(path)) continue; // already done once
      seen.insert(path); // process each path only once
      
      for (const auto& [neighbor, weight] : graph.at(current)) {
        using aoc::raw::operator<<;
        std::vector<Vertex> newPath = path;
        newPath.push_back(neighbor);
        stack.push({neighbor, newPath});
//            std::cout << NL << T << current << " --> " << neighbor << " " << weight;
      }

    }
//    std::cout << NL << "to_paths END" << std::flush;
    return allPaths;
  }

  std::optional<Result> test0(Towels const&  towels,Expected const& expected) {
    Integer count{};
    std::cout << NL << NL << "test0:" << expected;
    using aoc::raw::operator<<;
    std::cout << NL << T << "available towels:" << towels;
    using test::operator<<;
    std::cout << NL << T << "expected:" << expected;
    auto design = expected.first;
    auto graph = to_graph(design, towels,false);
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

  std::optional<Result> test1(auto& doc_in,Args args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "test1";
    Integer acc{};
    if (doc_in) {
      auto entries = test::parse(doc_in);
      if (entries.size()==1) {
        auto const& entry = entries.back();
        auto const& towels = entry.example.towels;
        for (auto const& expected : entry.expecteds) {
          if (auto answer = test0(towels,expected)) {
            acc += 1;
          }
        }
      }
      else {
        std::cerr << NL << "Sorry, expected only one entry to test from parsing doc.txt";
      }
    }
    result = std::to_string(acc);
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
      for (auto const& [dx,design] : aoc::views::enumerate(model.designs)) {
        std::cout << NL << "processing design[" << dx << "]:" << design << std::flush;
        auto graph = test::to_graph(design, model.towels,false);
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
    return result; // 213 too low
  }
}

namespace part2 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part2";
    if (in) {
      Integer acc{};
      auto model = parse(in);
      for (auto const& [dx,design] : aoc::views::enumerate(model.designs)) {
        std::cout << NL << "processing design[" << dx << "]:" << design << std::flush;
        auto graph = test::to_graph(design, model.towels,true);
        std::cout << NL << "design[" << dx << "]:" << std::quoted(design);
        if (graph.contains(static_cast<test::Vertex>(design.size()-1))) {
          auto paths = test::to_paths(graph, 0,static_cast<int>(design.size()));
          acc += paths.size();
          std::cout  << " paths: " << paths.size() << " acc:" << acc << std::flush;
//          for (auto const& [px,path] : aoc::views::enumerate(paths)) {
//            using aoc::raw::operator<<;
//            std::cout << NL << T << px << " " << path;
//            std::cout << " <-> ";
//            for (int i=1;i<path.size();++i) {
//              auto v1 = path[i-1];
//              auto v2 = path[i];
//              auto adj = graph[v1];
//              auto iter =  std::find_if(adj.begin(),adj.end(),[v2](auto const& entry){
//                return entry.first == v2;
//              });
//              std::cout << " " << iter->second;
//            }
//          }
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

using Answers = std::vector<std::pair<std::string,std::optional<Result>>>;
int main(int argc, char *argv[]) {
  Args args{};
  for (int i=1;i<argc;++i) {
    args.push_back(argv[i]);
  }

  Answers answers{};
  std::vector<std::chrono::time_point<std::chrono::system_clock>> exec_times{};
  exec_times.push_back(std::chrono::system_clock::now());
  std::vector<int> states = {20};
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
