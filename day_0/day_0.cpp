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
  if (lines.size()>1) {
    std::cout << NL << T << lines.size() << " lines";
    for (int i=0;i<lines.size();++i) {
      auto line = lines[i];
      std::cout << NL << T << T << "line[" << i << "]:" << line.size() << " " << std::quoted(line.str());
    }
  }
  else {
    // single line
    std::cout << NL << T << T << "input:" << input.size() << " " << std::quoted(input.str());
  }
  return result;
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
//  std::vector<int> states = {11};
  std::vector<int> states = {11,10,21,20};
  for (auto state : states) {
    switch (state) {
      case 11: {
        std::filesystem::path file{"../../example.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1 Example",part1::solve_for(in,args)});
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
      case 21: {
        std::filesystem::path file{"../../example.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2 Example",part2::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 20: {
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
