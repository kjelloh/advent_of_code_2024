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
  class Splitter; // Forward
  class Splitters; // Forward
  using SplitterPair = std::pair<Splitter,Splitter>;

  class Splitter {
  public:
    Splitter(std::string const& s) : m_s{s} {}
    Splitter(std::istream& is) : m_s{
       std::istreambuf_iterator<char>(is)
      ,std::istreambuf_iterator<char>()
    } {};
    Splitters lines() const;
    SplitterPair split(char ch) const;
    Splitters splits(char sep = ' ') const;
    std::string const& str() const {return m_s;}
    operator std::string() const {return m_s;}
    auto size() const {return m_s.size();}
  private:
    std::string m_s{};
  };

  class Splitters {
  public:
    Splitters& push_back(Splitter const& splitter) {
      m_splitters.push_back(splitter);
      return *this;
    }
    auto begin() {return m_splitters.begin();}
    auto end() {return m_splitters.end();}
    
  private:
    std::vector<Splitter> m_splitters{};
  };

  Splitters Splitter::lines() const {
    Splitters result{};
    std::istringstream is{m_s};
    std::string line{};
    while (std::getline(is,line)) result.push_back(line);
    return result;
  }

  SplitterPair Splitter::split(char sep) const {
    std::size_t pos = m_s.find(sep);
    if (pos == std::string::npos) {
      return {m_s, std::string{}};
    }
    // Split at the separator
    return {m_s.substr(0, pos), m_s.substr(pos + 1)};
  }

  std::string trim(const std::string& str) {
      auto start = std::find_if_not(str.begin(), str.end(), ::isspace);
      auto end = std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();
      return std::string(start, end);
  }

  Splitters Splitter::splits(char sep) const {
    Splitters result{};
    auto split_range =
        m_s
      | std::ranges::views::split(sep)  // Split by separator
      | std::ranges::views::transform([](auto&& range) {
          return std::string(range.begin(), range.end());
        })  // Convert each split range to a string
      | std::ranges::views::transform(trim)  // Trim each part
      | std::ranges::views::filter([](const std::string& part) {
          return !part.empty();  // Filter out empty parts
        });

    // Collect the results into the vector
    for (const auto& part : split_range) {
        result.push_back(part);
    }
    return result;
  }
}

Model parse(auto& in) {
  using namespace parsing;
  std::cout << "\n<BEGIN parse>";
  Model result{};
  std::string line{};
  int count{};
  auto lines = Splitter{in}.lines();
  for (auto const& line : lines) {
    std::cout << "\nLine[" << count++ << "]:" << " " << std::quoted(line.str()) << ":" << line.size();
    result.push_back(line);
  }
  std::cout << "\n<END parse>";
  return result;
}


using Args = std::vector<std::string>;

using Position = std::pair<int,int>;
using Delta = Position;
using PositionPair = std::pair<Position,Position>;

std::ostream& operator<<(std::ostream& os,Position pos) {
  os << "[r:" << pos.first << ",c:" << pos.second << "]";
  return os;
}

Delta to_delta(PositionPair const& pp) {
  // pp.first + delta = pp.second
  return {{pp.second.first-pp.first.first},{pp.second.second-pp.first.second}};
}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      auto const WIDTH = model[0].size();
      auto const HEIGHT = model.size();
      std::map<char,std::vector<Position>> c2p{};
      for (int r=0;r<model.size();++r) {
        for (int c=0;c<model[0].size();++c) {
          auto ch = model[r][c];
          if (ch != '.') c2p[ch].push_back({r,c});
        }
      }
      if (true) {
        for (auto const& entry : c2p) {
          std::cout << NL << entry.first;
          for (auto const& pos : entry.second) {
            std::cout << " [r:" << pos.first << ",c:" << pos.second << "]";
          }
        }
      }
      
      std::set<Position> candidates{};
      for (auto const& entry : c2p) {
        auto ch = entry.first;
        auto v = entry.second;
        for (int i=0;i<v.size();++i) {
          for (int j=0;j<v.size();++j) {
            if (i!=j) {
              auto vi = v[i];
              auto vj = v[j];
              PositionPair pp{vi,vj};
              auto delta = to_delta(pp);
              
              // vi+delta=vj --> vj+delta = vi+2*delta
              Position c{vj.first+delta.first,vj.second+delta.second};
              std::cout << NL << vi << " and " << vj;
              std::cout << " candidate " << c;
              if (c.first>=0 and c.first<WIDTH and c.second>=0 and c.second < HEIGHT) {
                std::cout << " OK (on grid)";
                candidates.insert(c);
              }

              if (delta.first % 2 == 0 and delta.second % 2 == 0) {
                std::cout << " between candidates ";
                // halfway between is valid c1=vi+0.5*delta, c2=vj-0.5*delta
                Position c1{vi.first+delta.first/2,vi.second+delta.second/2};
                Position c2{vj.first-delta.first/2,vj.second-delta.second/2};
                std::cout << c1 << " " << c2;
              }
            }
          }
        }
      }
      result = candidates.size();
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
  std::vector<int> states = {0,1};
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
