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

namespace aoc {
  namespace parsing {
    using Line = std::string;
    using Lines = std::vector<Line>;
    using Sections = std::vector<Lines>;
    class Splitter {
    public:
      Splitter(std::string const& s) : m_s{s} {}
      Splitter(std::istream& is) : m_s{
         std::istreambuf_iterator<char>(is)
        ,std::istreambuf_iterator<char>()
      } {};
      
      std::vector<std::vector<Splitter>> sections() {
        std::vector<std::vector<Splitter>> result;
        std::istringstream is{m_s};
        result.push_back({});
        Line line{};
        while (std::getline(is,line)) {
          if (line.size() == 0) {
            result.push_back({});
          }
          else {
            result.back().push_back(line);
          }
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
      Splitter trim() {
          auto start = std::find_if_not(m_s.begin(), m_s.end(), ::isspace);
          auto end = std::find_if_not(m_s.rbegin(), m_s.rend(), ::isspace).base();
          return std::string(start, end);
      }
      std::vector<Splitter> groups(std::string const& regexPattern) {
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
  }

  namespace grid {

    struct Position {
      int row{};
      int col{};
      bool operator<(const Position& other) const {
        return std::tie(row, col) < std::tie(other.row, other.col);
      }
      bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
      }
      Position operator+(Position const& other) const {return {row+other.row,col+other.col};}
      Position operator-(Position const& other) const {return {row-other.row,col-other.col};}
    };
    std::ostream& operator<<(std::ostream& os,Position const& pos) {
      os << "{row:" << pos.row << ",col:" << pos.col << "}";
      return os;
    }
    using Positions = std::vector<Position>;
    std::ostream& operator<<(std::ostream& os,Positions const& positions) {
      int count{};
      os << "{";
      for (auto const& pos : positions) {
        if (count++>0) os << ",";
        os << pos;
      }
      os << "}";
      return os;
    }
  
    class Grid {
    public:

        Grid(std::vector<std::string> grid) : grid_(std::move(grid)) {}

        // Returns the height of the grid
        size_t height() const {
            return grid_.size();
        }

        // Returns the width of the grid
        size_t width() const {
            return grid_.empty() ? 0 : grid_[0].size();
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
                return grid_[pos.row][pos.col];
            }
            return std::nullopt;
        }
      bool contains(Position const& pos) const {
        return at(pos).has_value();
      }

    private:
        std::vector<std::string> grid_;
    };
  
    using Path = Positions;
    using Visited = std::map<Position, std::vector<Path>>;
  
    // Helper Functions for Specific Use Case
    std::vector<Position> default_neighbors(Position const& pos) {
        return {
            {pos.row - 1, pos.col}, // Up
            {pos.row + 1, pos.col}, // Down
            {pos.row, pos.col - 1}, // Left
            {pos.row, pos.col + 1}  // Right
        };
    }

  } // namespace grid

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
} // namespace aoc

#endif /* aoc_hpp */
