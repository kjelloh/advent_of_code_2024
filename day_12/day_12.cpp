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

auto const NL = "\n";
auto const T = "\t";
auto const NT = "\n\t";

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = std::string;
using Model = aoc::raw::Lines;

Model parse(auto& in) {
  using namespace aoc::parsing;
  Model result{};
  auto input = Splitter{in};
  auto lines = input.lines();
  if (lines.size()>1) {
    std::cout << NL << T << lines.size() << " lines";
    for (int i=0;i<lines.size();++i) {
      auto line = lines[i];
      std::cout << NL << T << T << "line[" << i << "]:" << line.size() << " " << std::quoted(line.str());
      result.push_back(line);
    }
  }
  else {
    // single line
    std::cout << NL << T << T << "input:" << input.size() << " " << std::quoted(input.str());
    result.push_back(input.trim());
  }
  return result;
}

using aoc::grid::Grid;
using aoc::grid::Position;
using aoc::grid::Positions;
using aoc::grid::Direction;

using Region = std::pair<char,Positions>;
using Perimeter = Positions;

std::vector<Position> to_adjacent(Position const& pos) {
    return {
      {pos.row - 1, pos.col} // Up
      ,{pos.row + 1, pos.col} // Down
      ,{pos.row, pos.col - 1} // Left
      ,{pos.row, pos.col + 1}  // Right
//      ,{pos.row - 1, pos.col + 1}  // Up Right
//      ,{pos.row + 1, pos.col + 1}  // Down Right
//      ,{pos.row + 1, pos.col - 1}  // Down Left
//      ,{pos.row - 1, pos.col - 1}  // Up Left
    };
}

std::vector<Position> to_nighbours(Position const& pos) {
    return {
      {pos.row - 1, pos.col} // Up
      ,{pos.row + 1, pos.col} // Down
      ,{pos.row, pos.col - 1} // Left
      ,{pos.row, pos.col + 1}  // Right
    };
}

using RegionInfo = std::pair<Region, Perimeter>; // Region and perimeter

Perimeter to_perimeter(Region const& region,Grid const& grid) {
  Perimeter result{};

  auto const& [region_id,members] = region;
  for (auto const& pos : members) {
    for (auto const& neighbour : to_nighbours(pos)) {
      if (not grid.on_map(neighbour) or grid.at(neighbour) != region_id) {
        result.push_back(neighbour);
      }
    }
  }
  return result;
}

// Flood-fill function
RegionInfo flood_fill(Grid const& grid, Position start, std::set<Position>& visited) {
  std::queue<Position> to_visit;
  Region region;

  // Get the ID of the region (character at the starting position)
  char region_id = grid.at(start);
  region.first = region_id;

  // Begin the flood-fill
  to_visit.push(start);
  visited.insert(start);

  while (!to_visit.empty()) {
    Position current = to_visit.front();
    to_visit.pop();

    region.second.push_back(current);

    // Check neighbors
    for (Position const& adjacent : to_adjacent(current)) {
      if (not grid.on_map(adjacent)) continue;
      if (visited.count(adjacent)) continue;
      if (grid.at(adjacent) != region_id) continue;
      to_visit.push(adjacent);
      visited.insert(adjacent);
    }
  }
  return {region, to_perimeter(region,grid)};
}

using RegionInfos = std::vector<RegionInfo>;
// Main function to compute regions and their perimeters
RegionInfos compute_regions(Grid const& grid) {
  RegionInfos result;
  std::set<Position> visited;

  for (int row = 0; row < static_cast<int>(grid.height()); ++row) {
    for (int col = 0; col < static_cast<int>(grid.width()); ++col) {
      Position pos = {row, col};
      if (!visited.count(pos)) {
        auto region_info = flood_fill(grid, pos, visited);
        result.push_back(region_info);
      }
    }
  }
  return result;
}

namespace test {
  std::vector<aoc::raw::Lines> to_examples(aoc::parsing::Sections const& sections) {
    std::vector<aoc::raw::Lines> result{};

    result.push_back({});
    result.back().append_range(aoc::parsing::to_raw(sections[10])); // -
    result.push_back({});
    result.back().append_range(aoc::parsing::to_raw(sections[22])); // 1
    result.push_back({});
    result.back().append_range(aoc::parsing::to_raw(sections[29])); // 2
    if (sections.size()>35) {
      // part two
      result.push_back({});
      result.back().append_range(aoc::parsing::to_raw(sections[39])); // 3
      result.push_back({});
      result.back().append_range(aoc::parsing::to_raw(sections[44])); // 4
      result.push_back({});
      result.back().append_range(aoc::parsing::to_raw(sections[47])); // 5
      result.push_back({}); // 6 (Own test - three regions shared corner)
      result.back().push_back("VBB");
      result.back().push_back("BNB");
      result.back().push_back("BBB");
    }
    return result;
  }

}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    Integer acc{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      Grid grid{model};
      auto infos = compute_regions(grid);
      for (const auto& [region,perimeter] : infos) {
        auto id = region.first;
        auto area = region.second.size();
        auto price = area*perimeter.size();
        std::cout << NL << "Plot ID: " << id;
        std::cout << NL << T << "Region:" << region.second;
        std::cout << NL << T << "Area:" << area;
        std::cout << NL << T << "Perimeter: " << perimeter;
        std::cout << NL << T << "Price: " << price;
        std::cout << NL << "-------------------------";
        acc += price;
      }
      result = std::to_string(acc);
    }
    return result;
  }
}

namespace part2 {

  // Directions for moving up, down, left, right
  Direction const RIGHT{0,1};
  Direction const DOWN{1,0};
  Direction const LEFT{0,-1};
  Direction const UP{-1,0};
  using Directions = std::vector<Direction>;
  Directions const directions = {RIGHT,RIGHT+DOWN,DOWN,DOWN+LEFT,LEFT,LEFT+UP,UP,UP+RIGHT};

  struct Edge {
    Position start;
    Position end;
    Direction normal; // Normal vector for the edge
    
    bool operator==(const Edge& other) const {
      return (start == other.start && end == other.end and normal == other.normal);
    }

    bool operator<(const Edge& other) const {
        if (start != other.start)
            return start < other.start;
        if (end != other.end)
            return end < other.end;
        return normal < other.normal;
    }
  };
  using Edges = std::vector<Edge>;

  std::ostream& operator<<(std::ostream& os,Edge const& edge) {
    auto const& [start,end,normal] = edge;
    os << "edge{start:" << start << ",end:" << end << ",normal:" << normal << "}";
    return os;
  }

  std::ostream& operator<<(std::ostream& os,Edges const& edges) {
    std::cout << "edges[";
    for (auto const& edge : edges) {
      if (edge != edges.front()) std::cout << "'";
      std::cout << edge;
    }
    std::cout << "]";
    return os;
  }


  Edges to_all_grid_pos_edges(Region const& region) {
//    std::cout << NL << "to_all_grid_pos_edges region:" << region.first << " : " << region.second.size();
    Edges result{};
    for (auto const& pos : region.second) {
      // Represent edge as if upper left corner is at plot {row,col}
      Edge top{pos,pos+RIGHT,UP};
      Edge right{pos+RIGHT,pos+RIGHT+DOWN,RIGHT};
      Edge bottom{pos+RIGHT+DOWN,pos+DOWN,DOWN};
      Edge left{pos+DOWN,pos,LEFT};
      result.push_back(top);
      result.push_back(right);
      result.push_back(bottom);
      result.push_back(left);
    }
    return result;
  }

  Edge to_reversed(Edge const& edge) {
    auto [start,end,normal] = edge;
    Direction reversed_normal{-normal.row,-normal.col};
    return {end,start,reversed_normal};
  }

  // Remove all 'internal edges'
  Edges to_perimeter_grid_pos_edges(Edges const& edges) {
//    std::cout << NL << "to_perimeter_grid_pos_edges(edges" << edges.size() << ")";
    Edges result{};
    std::set<Edge> outer_edges{};
    for (auto const& edge : edges) {
      auto reversed_edge = to_reversed(edge);
      if (outer_edges.contains(reversed_edge)) {
        outer_edges.erase(reversed_edge);
      }
      else {
        outer_edges.insert(edge);
      }
    }
    result.assign(outer_edges.begin(), outer_edges.end());
//    std::cout << NL << T << " -> edges:" << result.size();
    return result;
  }

  class DisconnectedGraph {
  public:
      // Constructor to process edges and find disconnected graphs
      DisconnectedGraph(Edges const& edges) {
//        std::cout << NL << "DisconnectedGraph(edges:" << edges.size() << ")";
        findDisconnectedGraphs(edges);
      }

      // Get the resulting subgraphs
      std::vector<Edges> const& getSubgraphs() const {
          return subgraphs;
      }

  private:
      std::vector<Edges> subgraphs{};

      std::vector<int> parent{};
      std::vector<int> rank{};

      int find(int v) {
          if (parent[v] != v)
              parent[v] = find(parent[v]); // Path compression
          return parent[v];
      }

      void unionSets(int u, int v) {
          int rootU = find(u);
          int rootV = find(v);
          if (rootU != rootV) {
              if (rank[rootU] < rank[rootV]) {
                  parent[rootU] = rootV;
              } else if (rank[rootU] > rank[rootV]) {
                  parent[rootV] = rootU;
              } else {
                  parent[rootV] = rootU;
                  ++rank[rootU];
              }
          }
      }

      void findDisconnectedGraphs(Edges const& edges) {
          // Step 1: Collect all unique vertices
          std::map<Position, int> vertexToId;
          int vertexId = 0;

          for (const auto& edge : edges) {
              if (vertexToId.find(edge.start) == vertexToId.end())
                  vertexToId[edge.start] = vertexId++;
              if (vertexToId.find(edge.end) == vertexToId.end())
                  vertexToId[edge.end] = vertexId++;
          }

          // Initialize union-find (disjoint set) structure
          parent.resize(vertexId);
          rank.resize(vertexId, 0);

          for (int i = 0; i < vertexId; ++i)
              parent[i] = i;

          // Step 3: Union edges
          for (const auto& edge : edges) {
              int u = vertexToId[edge.start];
              int v = vertexToId[edge.end];
              unionSets(u, v);
          }

          // Step 4: Group edges by connected component
          std::map<int, Edges> componentEdges;
          for (const auto& edge : edges) {
              int compId = find(vertexToId[edge.start]);
              componentEdges[compId].push_back(edge);
          }

          // Step 5: Store the result
          for (const auto& [_, component] : componentEdges) {
              subgraphs.push_back(component);
          }
      }
  };

  // Assemble edges into a perimeter graph
  std::optional<Edges> to_perimeter(const Edges& edges) {
    struct State {
      Edges path_from_start;
      State operator+(Edge const& edge) {
        State result{path_from_start};
        result.path_from_start.push_back(edge);
        return result;
      }
    };
    std::set<Edge> seen{};
    std::deque<State> q{};
    auto start = State{} + edges.front();
    q.push_back(start);
    while (not q.empty()) {
//      std::cout << NL << q.size();
      auto current = q.front();
      q.pop_front();
//      std::cout << NL << T << current.path_from_start;
      auto in_current_path = [&p = current.path_from_start](Edge const& edge) {
        auto iter = std::find(p.begin(),p.end(),edge);
        bool result = (iter != p.end());
//        std::cout << NL << " " << edge << " in_current_path:" << result;
        return result;
      };

      if (current.path_from_start.size() == edges.size()) {
        // All edges consumed
        return current.path_from_start;
      }

      for (auto const& next : edges) {
        if (in_current_path(next)) continue; // Edge already used
        if (current.path_from_start.back().end == next.start) {
          // candidate to link after current
          q.push_back(current + next);
        }
      }
    }
    return std::nullopt;
  }

  Edges compressEdges(const Edges& edges) {
      if (edges.empty()) return {};

      Edges compressed;
      Edge current = edges[0]; // Start with the first edge

      for (size_t i = 1; i <= edges.size(); ++i) {
          const Edge& next = edges[i % edges.size()]; // Wrap around for a closed loop

          // Check if the current and next edges can be merged
          if (current.normal == next.normal && current.end == next.start) {
              // Extend the current edge
              current.end = next.end;
          } else {
              // Push the current edge and start a new one
              compressed.push_back(current);
              current = next;
          }
      }
      
      return compressed;
  }

  Integer to_side_count(Region const& region) {
//    std::cout << NL << "to_side_count id:" << region.first;
    Integer result{};
    auto all_grid_pos_edges = to_all_grid_pos_edges(region);
    auto perimeter_grid_pos_edges = to_perimeter_grid_pos_edges(all_grid_pos_edges);
    DisconnectedGraph disconnected_graph{perimeter_grid_pos_edges};
//    std::cout << NL << "disconnected_graph count:" << disconnected_graph.getSubgraphs().size() << std::flush;
    for (auto const& subgraph : disconnected_graph.getSubgraphs()) {
//      std::cout << NL << T << "subgraph:" << subgraph;
      // TODO: Compress subgraph (edges) into straight edges of the region.
      //       Edges start,end,start,end,... with the same normal vector should be merged into single start,end,normal
      //       We can then just count the final region edges
      // Put edges in order
      auto perimeter = to_perimeter(subgraph);
      if (perimeter) {
//        std::cout << NL << "perimeter:" << *perimeter;
        auto se = compressEdges(*perimeter);
//        std::cout << NL << T << "compressed:" << se << std::flush;
        result += se.size(); // we should have only the region sides left?
      }
      else {
        std::cerr << NL << "Sorry, Failed to asemble a perimeter";
      }
    }
    return result;
  }

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part2";
    if (in) {
      auto model = parse(in);
      Grid grid{model};
      auto infos = compute_regions(grid);
      Integer acc{};
      for (const auto& [region,perimeter] : infos) {
        auto id = region.first;
        auto area = region.second.size();
        auto side_count = to_side_count(region);
        auto price = area*side_count;
        std::cout << NL << "Plot ID: " << id;
        std::cout << NL << T << "Region:" << region.second;
        std::cout << NL << T << "Area:" << area;
        std::cout << NL << T << "Perimeter: " << perimeter;
        std::cout << NL << T << "side_count: " << side_count;
        std::cout << NL << T << "Price: " << price;
        std::cout << NL << "-------------------------";
        acc += price;
        std::cout << T << "acc:" << acc;
      }
      result = std::to_string(acc);
    }
    return result;
  }
}

using Answers = std::vector<std::pair<std::string,std::optional<Result>>>;
int main(int argc, char *argv[]) {
  aoc::application app{};
  app.add_to_examples(test::to_examples);
  app.add_solve_for("1", part1::solve_for,"example.txt");
  app.add_solve_for("1", part1::solve_for,"example1.txt");
  app.add_solve_for("1", part1::solve_for,"example2.txt");
  app.add_solve_for("1", part1::solve_for,"puzzle.txt");
  app.add_solve_for("2", part2::solve_for,"example3.txt");
  app.add_solve_for("2", part2::solve_for,"example1.txt");
  app.add_solve_for("2", part2::solve_for,"example2.txt");
  app.add_solve_for("2", part2::solve_for,"example4.txt");
  app.add_solve_for("2", part2::solve_for,"example5.txt");
  app.add_solve_for("2", part2::solve_for,"example6.txt");
  app.add_solve_for("2", part2::solve_for,"puzzle.txt");
  app.run(argc,argv);
  app.print_result();
  
  /*

   Xcode Debug -O3
   
   For my input:
   
   ANSWERS
   duration:1ms answer[part 1 in:example.txt] 140
   duration:0ms answer[part 1 in:example1.txt] 772
   duration:0ms answer[part 1 in:example2.txt] 1930
   duration:41ms answer[part 1 in:puzzle.txt] 1352976
   duration:0ms answer[part 2 in:example3.txt] 80
   duration:0ms answer[part 2 in:example1.txt] 436
   duration:0ms answer[part 2 in:example2.txt] 1206
   duration:0ms answer[part 2 in:example4.txt] 236
   duration:0ms answer[part 2 in:example5.txt] 368
   duration:0ms answer[part 2 in:example6.txt] 78
   duration:51ms answer[part 2 in:puzzle.txt] 808796

   */
  return 0;
}
