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


namespace aoc {

  namespace raw {
    auto const NL = "\n";
    auto const T = "\t";
    auto const NT = "\n\t";
    using Line = std::string;
    using Lines = std::vector<Line>;
    using Sections = std::vector<Lines>;
    std::ostream& operator<<(std::ostream& os,Lines const& lines) {
      for (auto const& line : lines) {
        os << NL << line;
      }
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
      
      std::vector<std::vector<Splitter>> sections() {
        std::vector<std::vector<Splitter>> result;
        std::istringstream is{m_s};
        result.push_back({});
        aoc::raw::Line line{};
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
  

    struct Vector {
      int row{};
      int col{};
      bool operator<(const Vector& other) const {
        return std::tie(row, col) < std::tie(other.row, other.col);
      }
      bool operator==(const Vector& other) const {
        return row == other.row && col == other.col;
      }
      Vector operator+(Vector const& other) const {return {row+other.row,col+other.col};}
      Vector operator-(Vector const& other) const {return {row-other.row,col-other.col};}
    };
    std::ostream& operator<<(std::ostream& os,Vector const& pos) {
      os << "{row:" << pos.row << ",col:" << pos.col << "}";
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
  
    using Position = Vector;
    using Positions = Vectors;
    using Direction = Vector;
  
    Direction const UP{-1,0};
    Direction const DOWN{1,0};
    Direction const LEFT{0,-1};
    Direction const RIGHT{0,1};

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
        char& at(Position const& pos) {
          if (on_map(pos)) {
              return grid_[pos.row][pos.col];
          }
          throw std::runtime_error(std::format("Sorry, grid pos({},{}) is not on map width:{}, height:{}",pos.row,pos.col,width(),height()));
        }
      
        char operator[](Position const pos) const {
          if (auto och = at(pos)) return *och;
          throw std::runtime_error(std::format("Sorry, grid pos({},{}) is not on map width:{}, height:{}",pos.row,pos.col,width(),height()));
        }

        std::optional<std::string> at_row(int r) const {
          if (r < height()) {
            return grid_[r];
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

    private:
        std::vector<std::string> grid_;
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

} // namespace aoc

#endif /* aoc_hpp */
