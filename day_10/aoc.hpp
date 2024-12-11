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


namespace aoc {
  namespace parsing {
    using Line = std::string;
    using Lines = std::vector<Line>;
    class Splitter {
    public:
      Splitter(std::string const& s) : m_s{s} {}
      Splitter(std::istream& is) : m_s{
         std::istreambuf_iterator<char>(is)
        ,std::istreambuf_iterator<char>()
      } {};
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

    private:
        std::vector<std::string> grid_;
    };
  } // namespace grid
} // namespace aoc

#endif /* aoc_hpp */
