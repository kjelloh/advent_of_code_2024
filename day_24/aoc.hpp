//
//  aoc.hpp
//
//  Created by Kjell-Olov Högdahl on 2024-12-10.
//

#ifndef aoc_hpp
#define aoc_hpp

#include <string>
#include <iostream> // E.g., std::istream, std::ostream...
#include <sstream> // E.g., std::istringstream, std::ostringstream
#include <algorithm> // E.g., std::find, std::all_of,...
#include <regex>
#include <functional>
#include <map>
#include <fstream>
#include <filesystem>
#include <set>
#include <deque>
#include <iterator>

namespace aoc {

  namespace views {
    template <typename Iterator>
    class enumerate_iterator {
    public:
        // Constructor to initialize the iterator and the index
        enumerate_iterator(Iterator iter, typename std::iterator_traits<Iterator>::difference_type index)
            : iter_(iter), index_(index) {}

        // Dereference operator to return the index and the element
        auto operator*() const {
            return std::tuple(index_, *iter_);
        }

        // Prefix increment to advance the iterator
        enumerate_iterator& operator++() {
            ++iter_;
            ++index_;
            return *this;
        }

        // Comparison operator to compare iterators
        bool operator!=(const enumerate_iterator& other) const {
            return iter_ != other.iter_;
        }

    private:
        Iterator iter_;  // The iterator pointing to the current element
      typename std::iterator_traits<Iterator>::difference_type index_;   // The current index of the element
    };

    template <typename Range>
    class enumerate_view {
    public:
        // Constructor to accept the range
        enumerate_view(Range& range) : range_(range) {}

        // Begin function returning an enumerate_iterator with index 0
        auto begin() {
            return enumerate_iterator{std::ranges::begin(range_), 0};
        }

        // End function returning an enumerate_iterator pointing to the end
        auto end() {
            return enumerate_iterator{std::ranges::end(range_), std::ranges::distance(range_)};
        }

    private:
        Range& range_;  // The range we are enumerating over
    };

    // Helper function to create an enumerate view
    template <typename Range>
    auto enumerate(Range& range) {
        return enumerate_view<Range>(range);
    }
  } // namespace views


  namespace raw {
  
    template <typename T>
    int sign(T value) {
        return (value > T(0)) - (value < T(0));
    }
  
    auto const NL = "\n";
    auto const T = "\t";
    auto const NT = "\n\t";
    using Line = std::string;
    using Lines = std::vector<Line>;
    using Sections = std::vector<Lines>;
    std::ostream& operator<<(std::ostream& os,Lines const& lines) {
      for (auto const& [lx,line] : aoc::views::enumerate(lines)) {
        os << raw::NL << "line[" << lx << "]:" << line.size() << " "  << std::quoted(line);
      }
      return os;
    }

    // A concept for integral types for operator<<(std::vector<Int>) below
    template <typename T>
    concept Integral = std::is_integral_v<T>;

    template <typename T>
    requires Integral<T>
    std::ostream& operator<<(std::ostream& os, const std::vector<T>& ints) {
      os << "[";
      for (auto const& [ix,n] : aoc::views::enumerate(ints)) {
        if (ix>0) os << ',';
        os << n;
      }
      os << "]";
      return os;
    }
  
    template <typename T>
    requires Integral<T>
    std::ostream& operator<<(std::ostream& os, const std::set<T>& ints) {
      os << "{";
      for (auto const& [ix,n] : aoc::views::enumerate(ints)) {
        if (ix>0) os << ',';
        os << n;
      }
      os << "}";
      return os;
    }
  
    // Helper function to print std::set
    template <typename T>
    std::ostream& operator<<(std::ostream& os, std::set<T> const& s) {
        os << "{ ";
        for (auto const& elem : s) {
            os << elem << " ";
        }
        os << "}";
        return os;
    }
  }
  namespace parsing {
    class Splitter; // Forward
    using Line = Splitter;
    using Lines = std::vector<Splitter>;
    using Section = Lines;
    using Sections = std::vector<Section>;
  
    class Splitter {
    public:
      Splitter(std::string const& s) : m_s{s} {}
      Splitter(std::istream& is) : m_s{
         std::istreambuf_iterator<char>(is)
        ,std::istreambuf_iterator<char>()
      } {};
      
      // Groups lins into sections separate by one or more empty lines
      // That is, no empty sections are created.
      std::vector<std::vector<Splitter>> sections() {
        std::vector<std::vector<Splitter>> result;
        std::istringstream is{m_s};
        aoc::raw::Line line{};
        bool wait_start_section{true};
        while (std::getline(is,line)) {
          if (line.size() == 0) {
            wait_start_section = true;
          }
          else {
            if (wait_start_section) {
              result.push_back({}); // non empty line marks new section
              wait_start_section = false;
            }
            result.back().push_back(line);
          }
        }
        return result;
      }
      
      // Groups lins into sections with no leading indentation space
      // No empty sections are created.
      std::vector<std::vector<Splitter>> same_indent_sections() {
        std::vector<std::vector<Splitter>> result;
        std::istringstream is{m_s};
        aoc::raw::Line line{};
        std::size_t section_indent_size{0};
        result.push_back({}); // First section
        while (std::getline(is,line)) {
          auto line_indent_size = line.find_first_not_of(' ');
          if (line_indent_size != section_indent_size) {
            // New section
            if (result.back().size()>0 and line.size()>0) {
              // new if current section not empty and new line is empty
              result.push_back({});
            }
            section_indent_size = line_indent_size;
          }
          if (line.size()>0) result.back().push_back(line);
        }
        return result;
      }

      std::vector<Splitter> lines() const {
        std::vector<Splitter> result{};
        std::istringstream is{m_s};
        std::string line{};
        while (std::getline(is,line)) result.push_back(line);
        return result;
      }
      std::pair<Splitter,Splitter> split(char ch) const {
        std::size_t pos = m_s.find(ch);
        if (pos == std::string::npos) {
          return {m_s, std::string{}};
        }
        return {m_s.substr(0, pos), m_s.substr(pos + 1)};
      }
      std::vector<Splitter> splits(char sep) const {
        std::vector<Splitter> result{};
        std::istringstream is{m_s};
        std::string line{};
        while (std::getline(is, line, sep)) {
          result.push_back(line);
        }
        return result;
      }
      Splitter trim() const {
          auto start = std::find_if_not(m_s.begin(), m_s.end(), ::isspace);
          auto end = std::find_if_not(m_s.rbegin(), m_s.rend(), ::isspace).base();
          return std::string(start, end);
      }
      std::vector<Splitter> groups(std::string const& regexPattern) const {
          std::vector<Splitter> result;
          std::regex pattern(regexPattern);
          std::smatch matches;

          if (std::regex_search(m_s, matches, pattern)) {
              // Iterate over the captured groups (start from 1, as 0 is the whole match)
              for (size_t i = 1; i < matches.size(); ++i) {
                  result.push_back(matches[i].str());
              }
          }
          return result;
      }

      std::string const& str() const {return m_s;}
      operator std::string() const {return m_s;}
      auto size() const {return m_s.size();}
    private:
      std::string m_s{};
    };
    aoc::raw::Line to_raw(Line const& line) {return line.str();}
    aoc::raw::Lines to_raw(Lines const& lines) {
      aoc::raw::Lines result{};
      std::ranges::transform(lines,std::back_inserter(result),[](Line const& line){
        return to_raw(line);
      });
      return result;
    }
    aoc::raw::Sections to_raw(Sections const& sections) {
      aoc::raw::Sections result{};
      std::ranges::transform(sections,std::back_inserter(result),[](Section const& section){
        return to_raw(section);
      });
    }
  }
  
  namespace graph {
  
    template <typename T>
    class Graph {
    public:
      using Vertex = T;
      using Vertices = std::set<Vertex>;
      using AdjList = std::map<Vertex,Vertices>;
      Graph(Vertices const& vertices) {
        for (auto const& v : vertices) {
          add_vertex(v);
        }
      }
      void add_vertex(Vertex const& v) {m_adj.insert({v,{}});}
      void add_edge(Vertex const& v1,Vertex const& v2) {
        m_adj[v1].insert(v2);
      }
      AdjList const& adj() const {return m_adj;}
      auto size() const {return adj().size();}
    private:
      AdjList m_adj{};
    };
  
    template <typename T>
    std::ostream& operator<<(std::ostream& os,typename Graph<T>::AdjList const& adj_list) {
      
      return os;
    }
  
    template <typename T>
    std::ostream& operator<<(std::ostream& os,Graph<T> const& graph) {
      std::cout << "vertices:" << graph.size();
      for (auto const& adjacency : graph.adj()) {
        std::cout << raw::NL << raw::T << adjacency.first;
        using aoc::raw::operator<<;
        std::cout << " --> " << adjacency.second;
      }
      
      return os;
    }
  
    template <typename T>
    std::vector<std::string> to_graphviz_dot(Graph<T> const& graph) {
      std::vector<std::string> result{};
      //digraph G {
      result.push_back("digraph G {");
      //    A -> B;
      //    B -> C;
      //    A -> C;
      for (auto const& [v1,vertices] : graph.adj()) {
        for (auto const& v2 : vertices) {
          // Graph::Vertex must be streamable by an operator<<
          std::ostringstream oss{};
          oss << v1 << " -> " << v2;
          result.push_back(oss.str());
        }
      }
      //}
      result.push_back("}");
      return result;
    }
  
    template <typename T>
    class GraphAdapter {
    public:
        using Graph = aoc::graph::Graph<T>;
        using IntGraph = aoc::graph::Graph<int>;
      GraphAdapter(Graph const& originalGraph) : intGraph({}) {
            convertToIntGraph(originalGraph);
        }

        // Get the graph with int vertices
        const IntGraph& getIntGraph() const {
            return intGraph;
        }

        // Get the mapping from int to string
        const std::unordered_map<int, std::string>& getIntToVertexMap() const {
            return intToVertex;
        }

        // Get the mapping from string to int
        const std::unordered_map<std::string, int>& getVertexToIntMap() const {
            return vertexToInt;
        }

        // Get the original vertex from an int index
        std::string getVertexFromInt(int idx) const {
            auto it = intToVertex.find(idx);
            if (it != intToVertex.end()) {
                return it->second;
            }
            throw std::out_of_range("Index not found in the mapping");
        }

        // Get the int index from a vertex
        int getIntFromVertex(const std::string& vertex) const {
            auto it = vertexToInt.find(vertex);
            if (it != vertexToInt.end()) {
                return it->second;
            }
            throw std::out_of_range("Vertex not found in the mapping");
        }

    private:
        IntGraph intGraph;
        std::unordered_map<typename Graph::Vertex, int> vertexToInt;
        std::unordered_map<int, typename Graph::Vertex> intToVertex;

        // Helper method to perform the conversion
        void convertToIntGraph(Graph const& originalGraph) {
            int index = 0;

            // Create mappings from string to int and int to string
            for (const auto& [vertex, _] : originalGraph.adj()) {
                vertexToInt[vertex] = index;
                intToVertex[index] = vertex;
                index++;
            }

            // Convert the adjacency list with string nodes to one with int nodes
            for (const auto& [vertex, neighbors] : originalGraph.adj()) {
                int u = vertexToInt[vertex];
                for (const auto& neighbor : neighbors) {
                    int v = vertexToInt[neighbor];
                    intGraph.add_edge(u, v);
                }
            }
        }
    };

  }

  namespace xy {
    struct Vector {
      int64_t x{};
      int64_t y{};
      bool operator<(const Vector& other) const {
        return std::tie(x, y) < std::tie(other.x, other.y);
      }
      bool operator==(const Vector& other) const {
        return x == other.x && y == other.y;
      }
      Vector operator+(Vector const& other) const {return {x+other.x,x+other.y};}
      Vector operator-(Vector const& other) const {return {x-other.x,y-other.y};}
      template <typename T>
      Vector operator*(T scalar) const {return {scalar * x,scalar * y};}
    };
    std::ostream& operator<<(std::ostream& os,Vector const& pos) {
      os << "{x:" << pos.x << ",y:" << pos.y << "}";
      return os;
    }
    using Vectors = std::vector<Vector>;
    std::ostream& operator<<(std::ostream& os,Vectors const& positions) {
      int count{};
      os << "{";
      for (auto const& pos : positions) {
        if (count++>0) os << ",";
        os << pos;
      }
      os << "}";
      return os;
    }
  
    class Solver {
    public:
      // rhs = m * coeffA + n * coeffB --> integer {m,n}?

      // xy = m*da + n*db
      // x = m*da_x + n*db_x
      // y = m*da_y + n*db_y
      
      // Eliminate m
      // x*da_y = m*da_x*da_y + n*db_x*da_y
      // y*da_x = m*da_x*da_y + n*db_y*da_x
      // So,...
      // x*da_y - y*da_x = m*da_x*da_y - m*da_x*da_y + n*db_x*da_y - n*db_y*da_x
      // n = (x*da_y - y*da_x) / (db_x*da_y - db_y*da_x)
      
      // Eliminate n
      // x*db_y = m*da_x*db_y + n*db_x*db_y
      // y*db_x = m*da_y*db_x + n*db_y*db_x
      // x*db_y - y*db_x = m*da_x*db_y - m*da_y*db_x  + n*db_x*db_y - n*db_y*db_x
      // m = (x*db_y - y*db_x) / (da_x*db_y - da_y*db_x)

      Solver(const Vector& rhs, const Vector& coeffA, const Vector& coeffB)
      : rhs_(rhs), coeffA_(coeffA), coeffB_(coeffB) {}
      
      // Solve the system and return a pair {m, n}
      std::optional<std::pair<int64_t, int64_t>> solve() {
        // Unpack the components of the vectors
        int64_t x = rhs_.x, y = rhs_.y;
        int64_t da_x = coeffA_.x, da_y = coeffA_.y;
        int64_t db_x = coeffB_.x, db_y = coeffB_.y;
                
        // Calculate m's and n's numerators and denominators
        int64_t m_num = (x * db_y - y * db_x);
        int64_t m_denom = (da_x * db_y - da_y * db_x);

        if (m_denom == 0) {
          throw std::runtime_error("Zero denominator when solving for {m,n}");
        }

        int64_t n_num = (x * da_y - y * da_x);
        int64_t n_denom = -m_denom;

        // Check if m and n are integer solutions
        try {
          int64_t m = solve_for_scalar(m_num, m_denom, "m");
          int64_t n = solve_for_scalar(n_num, n_denom, "n");
          return std::make_pair(m, n);
        }
        catch (...) {
        }
        return std::nullopt;

      }
      
    private:
      Vector rhs_;      // Right-hand side vector (xy)
      Vector coeffA_;   // Coefficients for vector A (da)
      Vector coeffB_;   // Coefficients for vector B (db)
      
      // Helper function to solve for a scalar (m or n)
      int64_t solve_for_scalar(int64_t numerator, int64_t denominator, const std::string& scalar_name) {
        if (numerator % denominator == 0) {
          return numerator / denominator;
        } else {
          throw std::runtime_error("No integer solution for " + scalar_name + " press count.");
        }
      }
    };
  } // namespace xy

  namespace grid {
    struct Vector {
      using value_type = int;
      value_type row{};
      value_type col{};
      bool operator<(const Vector& other) const {
        return std::tie(row, col) < std::tie(other.row, other.col);
      }
      bool operator==(const Vector& other) const {
        return row == other.row && col == other.col;
      }
      Vector operator+(Vector const& other) const {return {row+other.row,col+other.col};}
      Vector operator-(Vector const& other) const {return {row-other.row,col-other.col};}
      int cross(const Vector& other) const {
          return row * other.col - col * other.row;
      }
    };
    std::ostream& operator<<(std::ostream& os,Vector const& v) {
      os << "{row:" << v.row << ",col:" << v.col << "}";
      return os;
    }
    using Vectors = std::vector<Vector>;
    using Seen = std::set<Vector>; // Easy lookup

    // Concept to restrict to specific containers of Vector
    template <typename T>
    concept IterableVectors =
        (std::is_same_v<T, std::vector<Vector>> || std::is_same_v<T, std::set<Vector>>);
  
    // operator<< for any iterable 'vectors' of Vector elements
    template <typename Vectors>
    requires IterableVectors<Vectors>
    std::ostream& operator<<(std::ostream& os, const Vectors& vectors) {
      int count{};
      os << "[";
      for (auto const& v : vectors) {
        if (count++>0) os << ",";
        os << v;
      }
      os << "]";
      return os;
    }

    using Position = Vector;
    using Positions = Vectors;
    using Direction = Vector;
  
    Direction const UP{-1,0};
    Direction const DOWN{1,0};
    Direction const LEFT{0,-1};
    Direction const RIGHT{0,1};

    class Grid {
    public:
      using Seen = std::set<Position>;
      Grid& push_back(raw::Line const& row) {
        m_grid.push_back(row);
        return *this;
      }
      Grid(std::vector<std::string> grid = {}) : m_grid(std::move(grid)) {}
      
      // Returns the height of the grid
      size_t height() const {
        return m_grid.size();
      }
      
      // Returns the width of the grid
      size_t width() const {
        return m_grid.empty() ? 0 : m_grid[0].size();
      }
      
      // Checks if a position is within the bounds of the grid
      bool on_map(Position const& pos) const {
        auto const [row, col] = pos;
        return (row >= 0 && row < static_cast<int>(height()) &&
                col >= 0 && col < static_cast<int>(width()));
      }
      
      // Retrieves the character at a given position, if valid
      std::optional<char> at(Position const& pos) const {
        if (on_map(pos)) {
          return m_grid[pos.row][pos.col];
        }
        return std::nullopt;
      }
      char& at(Position const& pos) {
        if (on_map(pos)) {
          return m_grid[pos.row][pos.col];
        }
        throw std::runtime_error(std::format("Sorry, grid pos({},{}) is not on map width:{}, height:{}",pos.row,pos.col,width(),height()));
      }
      
      char operator[](Position const pos) const {
        if (auto och = at(pos)) return *och;
        throw std::runtime_error(std::format("Sorry, grid pos({},{}) is not on map width:{}, height:{}",pos.row,pos.col,width(),height()));
      }
      
      std::optional<std::string> at_row(int r) const {
        if (r < height()) {
          return m_grid[r];
        }
        return std::nullopt;
      }
      
      std::string operator[](int r) const {
        if (auto orow = at_row(r)) return *orow;
        else throw std::runtime_error(std::format("Sorry, Row {} is outside grid height {}",r,height()));
      }
      
      void for_each(auto f) const {
        for (int row=0;row<height();++row) {
          for (int col=0;col<width();++col) {
            f(*this,Position{row,col});
          }
        }
      }
      
      Position find(char ch) {
        Position result{-1,-1};
        for (int row=0;row<height();++row) {
          for (int col=0;col<width();++col) {
            if (at({row,col})==ch) result = {row,col};
          }
        }
        return result;
      }
      
      Positions find_all(char ch) const {
        Positions result{};
        auto push_back_matched = [ch,&result](Grid const& grid,Position const& pos) {
          if (grid.at(pos) == ch) result.push_back(pos);
        };
        for_each(push_back_matched);
        return result;
      }
      
      bool contains(Position const& pos) const {
        return at(pos).has_value();
      }
      
      bool operator==(Grid const& other) const {
        bool result{true};
        auto all_equal = [this,other,&result](Grid const& grid,Position const& pos){
          result = result and (this->at(pos) == other.at(pos));
        };
        this->for_each(all_equal);
        return result;
      }
      
      Position top_left() const {return {0,0};}
      Position bottom_right() const {
        return {
           static_cast<Position::value_type>(height()-1)
          ,static_cast<Position::value_type>(width()-1)
        };
      }
      
    private:
      std::vector<std::string> m_grid;
    };
      
    std::ostream& operator<<(std::ostream& os,Grid const& grid) {
      os << raw::NL << raw::T;
      for (int col=0;col<grid.width();++col) {
        os << (col%10);
      }
      for (int row=0;row<grid.height();++row) {
        os << raw::NL << row << ":" << raw::T << *grid.at_row(row);
      }
      return os;
    }
  
    using GridPair = std::pair<Grid,Grid>;
    std::ostream& operator<<(std::ostream& os,GridPair const& gg) {
      os << raw::NL << raw::T;
      for (int col=0;col<gg.first.width();++col) {
        os << (col%10);
      }
      os << raw::T;
      for (int col=0;col<gg.second.width();++col) {
        os << (col%10);
      }
      std::pair<std::string,std::string> indent{std::string(gg.first.height(),' '),std::string(gg.second.height(),' ')};
      auto max_height = std::max(gg.first.height(),gg.second.height());
      for (int row=0;row<static_cast<int>(max_height);++row) {
        os << raw::NL << row << ":" << raw::T;
        if (row < gg.first.height()) os << *gg.first.at_row(row);
        else {
          os << indent.first;
        }
        os << raw::T;
        if (row < gg.second.height()) os << *gg.second.at_row(row);
        else {
          os << indent.first;
        }
      }

      return os;
    }
  
    using Path = Positions;
    // Helper Functions for Specific Use Case
    std::vector<Position> to_ortho_neighbours(Position const& pos) {
        return {
            {pos.row - 1, pos.col}, // Up
            {pos.row + 1, pos.col}, // Down
            {pos.row, pos.col - 1}, // Left
            {pos.row, pos.col + 1}  // Right
        };
    }
  
    const std::vector<Position> ortho_directions = {
        {0, 1},  // Right
        {1, 0},  // Down
        {0, -1}, // Left
        {-1, 0}  // Up
    };

    int to_direction_index(Position const& from, Position const& to) {
      auto it = std::find(ortho_directions.begin(), ortho_directions.end(),to-from);
      return (it != ortho_directions.end()) ? static_cast<int>(std::distance(ortho_directions.begin(), it)) : -1;
    }
  
    char to_dir_char(Position const& from, Position const& to) {
      char result{'?'};
      switch (to_direction_index(from, to)) {
        case 0: result = '>'; break;
        case 1: result = 'v'; break;
        case 2: result = '<'; break;
        case 3: result = '^'; break;
        case -1: break;
      }
      return result;
    }

    Grid& to_dir_traced(Grid& grid,Path const& path) {
      for (int i=1;i<path.size()-1;++i) {
        auto from = path[i];
        auto to = path[i+1];
        switch (to_direction_index(from, to)) {
          case 0: grid.at(from) = '>'; break;
          case 1: grid.at(from) = 'v'; break;
          case 2: grid.at(from) = '<'; break;
          case 3: grid.at(from) = '^'; break;
          case -1: break;
        }
      }
      return grid;
    }

    template <typename T>
    Grid& to_filled(Grid& grid,T const& seen,char filler = 'O') {
      for (auto const& visited : seen) {
        grid.at(visited) = filler;
      }
      return grid;
    }

    Seen to_flood_fill(Grid const& grid,Position start) {
      Seen result{};
      std::deque<Position> q{};
      Seen visited{};
      if (grid.on_map(start)) {
        q.push_back(start);
        char ch = *grid.at(start);
        visited.insert(start);
        while (not q.empty()) {
          auto curr = q.front();q.pop_front();
          for (auto const& next : to_ortho_neighbours(curr)) {
            if (not grid.on_map(next)) continue;
            if (visited.contains(next)) continue;
            if (grid.at(next) != ch) continue;
            q.push_back(next);
            visited.insert(next);
          }
        }
      }
      result = visited;
      return result;
    }

  } // namespace grid

  namespace set {
  
    template <typename T>
    std::set<T> operator&(const std::set<T>& lhs, const std::set<T>& rhs) {
      std::set<T> result;
      std::set_intersection(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                            std::inserter(result, result.begin()));
      return result;
    }
  }

  namespace dfs {
    /*
     DFS to count number of created (visited) Keys.
     From 2024 day_11
      Key is a Number and transform_fn transforms the Number
      into a vector of new numbers.
      state_fn simply creates the memoised State std::pair<remaining_blinks,numer>
     
     Future re-use:
       For a path length finding problem on a Grid Key can be a position.
       transform_fn generates the next positions to move to.
       State can now memoise current remaining steps and position to a known count.
     
       I suppose Key may also be a Path is we need the State to distinguish between
       the ways we reach a position in the grid?
     
       Lets see if we can reuse this code :)
     
     */
    template<typename Key, typename Result, typename State>
    Result find_count(
        int remaining_steps,
        Key initial_key,
        std::function<std::vector<Key>(Key)> const& transform_fn,
        std::function<State(int, Key)> const& state_fn,
        std::map<State, Result>& seen) {
        if (remaining_steps == 0) {
            return Result(1); // Base case: count the initial state itself
        }

        State memo_state = state_fn(remaining_steps, initial_key);
        if (seen.contains(memo_state)) {
            return seen[memo_state];
        }

        Result result = Result(0);
        for (auto const& next_key : transform_fn(initial_key)) {
            result += find_count(remaining_steps - 1, next_key, transform_fn, state_fn, seen);
        }

        seen[memo_state] = result;
        return result;
    }

    // Overload for initial invocation
    template<typename Key, typename Result, typename State>
    Result find_count(
        int remaining_steps,
        std::vector<Key> const& initial_keys,
        std::function<std::vector<Key>(Key)> const& transform_fn,
        std::function<State(int, Key)> const& state_fn) {
        Result result{};
        std::map<State, Result> seen;

        for (auto const& key : initial_keys) {
            result += find_count(remaining_steps, key, transform_fn, state_fn, seen);
        }

        return result;
    }

  }

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
        std::cout << raw::NL << "No working directory path configured";
      }
      std::cout << raw::NL << "Using working_dir " << *cached;
    }
    return *cached / file_name;
  }

  namespace test {
    template <class T>
    using LogEntries = std::vector<T>;
  
    template <class T>
    std::ostream& operator<<(std::ostream& os,LogEntries<T> const& log) {
      for (auto const& entry : log) {
        os << raw::NL << entry;
      }
      return os;
    }

    template <class T>
    struct Outcome {
      T expected;
      T computed;
      bool accepted() const {return computed == expected;};
    };

    template <class T>
    std::ostream& operator<<(std::ostream& os,Outcome<T> const& outcome) {
      std::ostringstream expected_os{};
      expected_os << outcome.expected;
      std::ostringstream computed_os{};
      computed_os << outcome.computed;
      aoc::raw::Lines expected_lines{};
      aoc::raw::Lines computed_lines{};
      std::istringstream expected_is{expected_os.str()};
      std::istringstream computed_is{computed_os.str()};
      aoc::raw::Line line{};
      while (std::getline(expected_is,line)) expected_lines.push_back(line);
      while (std::getline(computed_is,line)) computed_lines.push_back(line);
      auto max_lines = std::max(expected_lines.size(),computed_lines.size());
      std::size_t
      last_width{};
      std::cout << raw::NL << "Expected " << raw::T << "Computed";
      for (int i=0;i<max_lines;++i) {
        std::cout << raw::NL;
        if (i==0) {
          std::cout << raw::NL << raw::T << "Expected:" << expected_lines[i];
          std::cout << raw::NL << raw::T << " Computed:" << computed_lines[i];
        }
        else {
          if (i<expected_lines.size()) {
            std::cout << expected_lines[i];
            last_width = expected_lines[i].size();
          }
          else {
            std::cout << std::string(last_width,' ');
          }
          std::cout << raw::T;
          if (i < computed_lines.size()) {
            std::cout << computed_lines[i];
          }
        }
      }
      if (outcome.accepted()) {
        std::cout << raw::NL << "COMPUTED is equal to EXPECTED OK";
      }
      else {
        std::cout << raw::NL << "COMPUTED and EXPECTED - DIFFERS...";
      }
      return os;
    }
  } // namespace test

} // namespace aoc

#endif /* aoc_hpp */
