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

template <>
struct std::formatter<Model> : std::formatter<std::string> {
  template<class FmtContext>
  FmtContext::iterator format(Model const& model, FmtContext& ctx) const {
    using aoc::grid::operator<<;
    std::ostringstream oss{};
    oss << model;
    std::format_to(ctx.out(),"\n{}",oss.str());
    return ctx.out();
  }
};

namespace test {

  std::vector<aoc::raw::Lines> to_examples(aoc::parsing::Sections const& sections) {
    std::vector<aoc::raw::Lines> result{};
    // for each example to parse
    result.push_back({});
    result.back().append_range(aoc::parsing::to_raw(sections[9]));
    return result;
  }

//  // Helper to print tuple elements
//  template <typename Tuple, std::size_t... Is>
//  void print_tuple(std::ostream& os, Tuple const& tuple, std::index_sequence<Is...>) {
//      ((os << (Is == 0 ? "" : ", ") << std::get<Is>(tuple)), ...);
//  }
//
//  // Overload for std::tuple
//  template <typename... Args>
//  std::ostream& operator<<(std::ostream& os, std::tuple<Args...> const& tuple) {
//      os << "(";
//      print_tuple(os, tuple, std::index_sequence_for<Args...>{});
//      os << ")";
//      return os;
//  }

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
          for (auto const& w : common_neighbors) {
            if (v < w) {
              // candidate for {u,v,w} where u < v < w (u -> v -> w only once)
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


  bool test0(std::optional<aoc::parsing::Sections> const& sections,Args args) {
    // This function is called by aoc::application if registered with add_test(test::test0)
    // Extract test data from provided sections from the day web page text.
    // See zsh-script pull_text.zsh for support to fetch advent of code day web page text to doc.txt
    std::cout << NL << "test0";
    if (sections) {
      std::cout << NL << T << "sections ok";
      // return test result here
      
      aoc::graph::Graph<std::string> graph({});
      auto lines = (*sections)[9];
      
      for (auto const& line : lines) {
        auto const& [left,right] = line.split('-');
        graph.add_edge(left, right);
        graph.add_edge(right,left);
      }
      auto triangles = find_triangles(graph);
      Integer acc{};
      for (auto const& triangle : triangles) {
        std::cout << NL << std::format("{}",triangle);
        if (tuple_any_of(triangle, [](auto const& computer){
          return computer.find("t") != std::string::npos;
        })) {
          std::cout << " keep";
          ++acc;
        }
      }
      return acc == 7;
      // end tests
    }
    else {
      std::cout << NL << T << "NO sections";
    }
    return false;
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
      else {
        Integer acc{};
        auto model = parse(in);
        if (args.options.contains("-parse_only")) return "DONE -parse_only";
        auto triangles = test::find_triangles(model);
        for (auto const& triangle : triangles) {
          std::cout << NL << std::format("{}",triangle);
          if (test::tuple_any_of(triangle, [](auto const& computer){
            return computer.front() == 't';
          })) {
            std::cout << " keep";
            ++acc;
          }
        }
        result = std::to_string(acc);
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
        std::cout << NL << std::format("{}",triangle);
        if (test::tuple_any_of(triangle, [](auto const& computer){
          return computer.front() == 't';
        })) {
          std::cout << " keep";
          ++acc;
        }
      }
      result = std::to_string(acc);
    }
    return result;
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
      
      // Backtrack: remove v from R and move v from P to X
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

   >day_23 -all
   
   ANSWERS
   duration:0ms answer[part:"test0"] PASSED
   duration:0ms answer[part 1 in:example.txt] 7
   duration:40ms answer[part 1 in:puzzle.txt] 1485
   duration:0ms answer[part 2 in:example.txt] co,de,ka,ta
   duration:89ms answer[part 2 in:puzzle.txt] cc,dz,ea,hj,if,it,kf,qo,sk,ug,ut,uv,wh
   
   */

}
