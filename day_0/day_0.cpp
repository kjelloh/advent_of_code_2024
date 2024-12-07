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

  Model parse(auto& in) {
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
}

Model parse(auto& in) {
  return parsing::parse(in);
}

using Args = std::vector<std::string>;

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
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
