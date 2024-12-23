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
using Computer = std::string;
using Model = aoc::graph::Graph<Computer>;

struct Args {
  std::map<std::string,std::string> arg{};
  std::set<std::string> options{};
};

Model parse(auto& in) {
  using namespace aoc::parsing;
  Model result({});
  auto lines = Splitter{in}.lines();
  for (auto const& [lx,line] : aoc::views::enumerate(lines)) {
    std::cout << NL << T << T << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
    auto const& [left,right] = line.split('-');
    result.add_edge(left, right);
    result.add_edge(right,left);
  }
  return result;
}

namespace test {

  // Adapt to expected for day puzzle
  struct LogEntry {
    bool operator==(LogEntry const& other) const {
      bool result{true};
      return result;
    }
  };

  std::ostream& operator<<(std::ostream& os,LogEntry const& entry) {
    return os;
  }

  using LogEntries = aoc::test::LogEntries<LogEntry>;

  LogEntries parse_doc() {
    std::cout << NL << T << "parse puzzle doc text";
    LogEntries result{};
    using namespace aoc::parsing;
    std::ifstream doc_in{aoc::to_working_dir_path("doc.txt")};
    auto sections = Splitter{doc_in}.same_indent_sections();
    for (auto const& [sx,section] : aoc::views::enumerate(sections)) {
      std::cout << NL << "---------- section " << sx << " ----------";
      for (auto const& [lx,line] : aoc::views::enumerate(section)) {
        std::cout << NL << T << T << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
      }
    }
    return result;
  }

  // Helper to print tuple elements
  template <typename Tuple, std::size_t... Is>
  void print_tuple(std::ostream& os, Tuple const& tuple, std::index_sequence<Is...>) {
      ((os << (Is == 0 ? "" : ", ") << std::get<Is>(tuple)), ...);
  }

  // Overload for std::tuple
  template <typename... Args>
  std::ostream& operator<<(std::ostream& os, std::tuple<Args...> const& tuple) {
      os << "(";
      print_tuple(os, tuple, std::index_sequence_for<Args...>{});
      os << ")";
      return os;
  }


  template <typename Graph>
  auto find_triangles(Graph const& graph) {
    using Vertex = typename Graph::Vertex;
    using Vertices = typename Graph::Vertices;
    using Result = std::vector<std::tuple<Vertex, Vertex, Vertex>>;
    Result triangles;
    auto const adjacent = graph.adj();
    for (auto const& [u, neighbors_u] : adjacent) {
      for (auto const& v : neighbors_u) {
        if (u < v) {
          // candidate for {u,v,w} where u < v (u,v edge only once)
          using aoc::set::operator&;
          auto common_neighbors = neighbors_u & adjacent.at(v);
//          Vertices common_neighbors;
//          std::set_intersection(
//             neighbors_u.begin(), neighbors_u.end()
//            ,adjacent.at(v).begin(), adjacent.at(v).end()
//            ,std::inserter(common_neighbors, common_neighbors.begin())
//            );
          for (auto const& w : common_neighbors) {
            if (v < w) {
              // candidate for {u,v,w} where u < v < w (u -> v -> w path only once)
              triangles.emplace_back(u, v, w);
            }
          }
        }
      }
    }
    return triangles;
  }

  template <typename Tuple, typename Predicate>
  bool tuple_any_of(Tuple const& tuple, Predicate pred) {
      bool result = false;
      std::apply([&](auto const&... elems) {
          ((result = result || pred(elems)), ...);
      }, tuple);
      return result;
  }

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "test";
    if (in) {
      auto model = parse(in);
      if (args.options.contains("-to_dot")) {
        auto dot_lines = aoc::graph::to_graphviz_dot(model);
        auto dot_file = aoc::to_working_dir_path("test.dot");
        std::ofstream out{dot_file};
        for (auto const& [lx,line] : aoc::views::enumerate(dot_lines)) {
          if (lx>0) out << NL;
          out << line;
        }
        std::cout << NL << "Created Graphviz DOT file " << dot_file;
      }
      auto doc_file = aoc::to_working_dir_path("doc.txt");
      std::ifstream doc_in{doc_file};
      if (doc_in) {
        using namespace aoc::parsing;
        auto sections = Splitter{doc_in}.same_indent_sections();
        for (auto const& [sx,section] : aoc::views::enumerate(sections)) {
          std::cout << NL << "---------- section " << sx << " ----------";
          for (auto const& [lx,line] : aoc::views::enumerate(section)) {
            std::cout << NL << T << T << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
          }
        }
        if (sections.size()>0) {
          aoc::graph::Graph<std::string> graph({});
          auto lines = sections[9];
          {
            // Conveniant for later
            auto example_file = aoc::to_working_dir_path("example.txt");
            std::ofstream out{example_file};
            if (out) {
              for (auto const& [lx,line] : aoc::views::enumerate(lines)) {
                if (lx>0) out << NL;
                out << line.str();
              }
              std::cout << NL << doc_file << " --> " << example_file;
            }
            else {
              std::cerr << NL << "Sorry, failed to create file " << example_file;
            }
          }
          
          for (auto const& line : lines) {
            auto const& [left,right] = line.split('-');
            graph.add_edge(left, right);
            graph.add_edge(right,left);
          }
          auto triangles = find_triangles(graph);
          for (auto const& triangle : triangles) {
            std::cout << NL << triangle;
            if (tuple_any_of(triangle, [](auto const& computer){
              return computer.find("t") != std::string::npos;
            })) {
              std::cout << " keep";
            }
          }
          
        }
        else {
          std::cerr << "Sorry, exoected doc.txt to contain sections";
        }
      }
      else {
        std::cerr << "Sorry, expected puzzle text file " << doc_file;
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
      Integer acc{};
      auto model = parse(in);
      if (args.options.contains("-parse_only")) return "DONE -parse_only";
      auto triangles = test::find_triangles(model);
      for (auto const& triangle : triangles) {
        using test::operator<<;
        std::cout << NL << triangle;
        if (test::tuple_any_of(triangle, [](auto const& computer){
          return computer.front() == 't';
        })) {
          std::cout << " keep";
          ++acc;
        }
      }
      result = std::to_string(acc);
    }
    return result; // 2462 too high
  }
}

namespace part2 {

  // Bron-Kerbosch to find the largest clique
  // https://en.wikipedia.org/wiki/Bron–Kerbosch_algorithm
  void bron_kerbosch_largest(
     std::set<int>& R // all (clique)
    ,std::set<int>& P // some (clique)
    ,std::set<int>& X // none (clique)
    ,const std::map<int, std::set<int>>& graph
    ,std::set<int>& largest_clique) {
    
    if (P.empty() && X.empty()) {
      // Some and None is empty = done
      // Found a maximal clique; check if it's the largest so far
      if (R.size() > largest_clique.size()) {
        // New largest clique
        largest_clique = R;
      }
      return;
    }
    
    // Iterate over vertices in P (some)
    std::set<int> P_copy = P; // Copy because P will be modified
    for (int v : P_copy) {
      // Add v to the current clique
      R.insert(v);
      
      // Compute P' = P ∩ neighbors(v) and X' = X ∩ neighbors(v)
      std::set<int> P_new, X_new;
      for (int neighbor : graph.at(v)) {
        if (P.contains(neighbor)>0) P_new.insert(neighbor);
        if (X.contains(neighbor)>0) X_new.insert(neighbor);
      }
      
      // Recursive call
      bron_kerbosch_largest(R, P_new, X_new, graph, largest_clique);
      
      // Backtrack: remove v from R, move v from P to X
      R.erase(v);
      P.erase(v);
      X.insert(v);
    }
  }

  std::set<int> find_largest_clique(auto const& graph) {
    std::set<int> largest_clique;
    std::set<int> R, P, X;
    
    // Initialize P with all vertices in the graph
    for (const auto& [vertex, _] : graph.adj()) {
      P.insert(vertex);
    }
    
    bron_kerbosch_largest(R, P, X, graph.adj(), largest_clique);
    return largest_clique;
  }

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part2";
    if (in) {
      auto model = parse(in);
      aoc::graph::GraphAdapter<std::string> to_int_graph(model);
      auto int_graph = to_int_graph.getIntGraph();
      std::cout << NL << "int_graph" << int_graph;
      auto largest_qlique = find_largest_clique(int_graph);
      using aoc::raw::operator<<;
      std::cout << NL << largest_qlique;
      auto computers = std::accumulate(largest_qlique.begin(),largest_qlique.end(),std::string{},[&to_int_graph](auto acc,auto int_v){
        if (acc.size()>0) acc += ",";
        acc += to_int_graph.getVertexFromInt(int_v);
        return acc;
      });
      std::cout << NL << computers;
      result = computers;
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
  
  user_args.arg["part"] = "test";
  user_args.arg["file"] = "doc.txt";

  // Override by any user input
  for (int i=1;i<argc;++i) {
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
  
  if (user_args.options.contains("-all")) {
    requests.clear();
    
    for (int i=0;i<4;++i) {
      Args args{};
      std::string part{};
      std::string file{};
      part = (i/2==0)?"1":"2";
      file = (i%2==0)?"example.txt":"puzzle.txt";
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
        answers.push_back({std::format("part{} {}",part,file.filename().string()),part2::solve_for(in,request)});
      }
      else if (part=="test") {
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

   >day_22 -all

   ANSWERS
   duration:1ms answer[part1 example.txt] 7
   duration:44ms answer[part1 puzzle.txt] 1485
   duration:0ms answer[part2 example.txt] co,de,ka,ta
   duration:88ms answer[part2 puzzle.txt] cc,dz,ea,hj,if,it,kf,qo,sk,ug,ut,uv,wh
   
   */
  return 0;
}
