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

auto const NL = "\n";
auto const T = "\t";
auto const NT = "\n\t";

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = Integer;
using Model = std::vector<std::string>;

namespace parsing {
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

Model parse(auto& in) {
  using namespace parsing;
  Model result{};
  auto input = Splitter{in};
  auto lines = input.lines();
  for (auto const& line : lines) {
    std::cout << NL << line.str();
    result.push_back(line);
  }
  return result;
}

using Args = std::vector<std::string>;

struct Position {
  int row{};
  int col{};
  bool operator<(const Position& other) const {
    return std::tie(row, col) < std::tie(other.row, other.col);
  }
};
std::ostream& operator<<(std::ostream& os,Position const& pos) {
  os << "{row:" << pos.row << ",col:" << pos.col << "}";
  return os;
}
using Positions = std::vector<Position>;
std::ostream& operator<<(std::ostream& os,Positions const& positions) {
  for (auto const& pos : positions) {
    os << " " << pos;
  }
  return os;
}
using TrailHeads = std::map<Position,Positions>;
std::ostream& operator<<(std::ostream& os,TrailHeads const& ths) {
  for (auto const& [start,ends] : ths) {
    os << NL << start;
    for (auto const& end : ends) {
      os << ends;
    }
  }
  return os;
}

Result to_scores_sum(TrailHeads const& ths) {
  Result result{};
  for (auto const& [start,ends] : ths) {
    auto score = ends.size();
    std::cout << NL << T << "score start:" << start << " -> " << ends << " = " << score;
    result += score;
  }
  return result;
}

auto height(Model const& model) {
  return model.size();
}

auto width(Model const& model) {
  return model[0].size();
}

bool on_map(Position const& pos,Model const& model) {
  auto const [row,col] = pos;
  return (row>=0 and row < height(model) and col >= 0 and col < width(model));
}

std::optional<char> at(Position pos,Model const& model) {
  std::optional<char> result{};
  auto const& [row,col] = pos;
  if (on_map(pos,model)) result = model[row][col];
  return result;
}

Positions to_start_candidates(Model const& model) {
  Positions result;
  std::cout << NL << "to_start_candidates";
  for (int row=0;row<height(model);++row) {
//    std::cout << NL << T;
    for (int col=0;col<width(model);++col) {
      Position pos{row,col};
      if (auto och = at(pos,model)) {
//        std::cout << "{" << *och << "}";
        if (*och == '0') result.push_back(pos);
      }
    }
  }
  return result;
}

using Visited = std::set<Position>;

Positions find_ends(Position const& start,Model const& model) {
  Positions result{};
  Visited visited{};
  std::stack<Position> to_visit;

  to_visit.push(start);

  while (!to_visit.empty()) {
    Position current = to_visit.top();
    std::cout << NL << "current:" << current;
    to_visit.pop();

    // Skip if already visited
    if (visited.count(current) > 0) continue;

    visited.insert(current);

    char cell_value = *at(current,model);

    if (cell_value == '9') {
      std::cout << NL << "is end:" << current;
      result.push_back(current);
    }

    // Explore neighbors (up, down, left, right)
    std::vector<Position> neighbors = {
        {current.row - 1, current.col}, // Up
        {current.row + 1, current.col}, // Down
        {current.row, current.col - 1}, // Left
        {current.row, current.col + 1}  // Right
    };

    for (const auto& neighbor : neighbors) {
      if (auto och = at(neighbor,model)) {
        auto nch = *och;
        if (nch != cell_value+1) continue;
        if (visited.contains(neighbor)) continue;
        std::cout << "try" << neighbor << " '" << nch << "''";
        to_visit.push(neighbor);
      }
    }
  }
  return result;
}

TrailHeads to_trail_heads(Position const& start,Model const& model) {
  TrailHeads result{};
  auto ends = find_ends(start, model);
  result[start] = ends;
  return result;
}

TrailHeads to_trail_heads(Model const& model) {
  TrailHeads result{};
  std::cout << NL << "to_trail_heads";
  auto starts = to_start_candidates(model);
  for (auto const& start : starts) {
    std::cout << NL << "processing start:" << start;
    auto ths = to_trail_heads(start, model);
    result[start] = ths[start];
  }
  std::cout << result;
  return result;
}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      auto trail_heads = to_trail_heads(model);
      result = to_scores_sum(trail_heads);
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
  for (int i=0;i<argc;++i) {
    args.push_back(argv[i]);
  }
  Answers answers{};
  std::vector<std::chrono::time_point<std::chrono::system_clock>> exec_times{};
  exec_times.push_back(std::chrono::system_clock::now());
//  std::vector<int> states = {0};
//  std::vector<int> states = {0,1};
  std::vector<int> states = {0,1,10};
//  std::vector<int> states = {2};
//  std::vector<int> states = {2,3};
//  std::vector<int> states = {0,1,2,3};
  for (auto state : states) {
    switch (state) {
      case 0: {
        std::filesystem::path file{"../../example.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1 Example",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 1: {
        std::filesystem::path file{"../../example2.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1 Example2",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 10: {
        std::filesystem::path file{"../../puzzle.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1     ",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 2: {
        std::filesystem::path file{"../../example.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2 Example",part2::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 3: {
        std::filesystem::path file{"../../puzzle.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2     ",part2::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      default:{std::cerr << "\nSORRY, no action for state " << state;} break;
    }
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
