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
using Operands = std::vector<Integer>;
using Model = std::vector<std::pair<Integer,Operands>>;

Model parse(auto& in) {
  std::cout << "\n<BEGIN parse>";
  Model result{};
  std::string line{};
  int count{};
  while (std::getline(in,line)) {
    std::cout << "\nLine[" << count++ << "]:" << line.size() << " " << std::quoted(line);
    result.push_back({});
    std::istringstream is{line};
    char colon{};
    Integer operand{};
    is >> result.back().first >> colon;
    while (is >> operand) result.back().second.push_back(operand);
  }
  std::cout << "\n<END parse>";
  return result;
}

std::ostream& operator<<(std::ostream& os,Operands const& operands) {
  for (auto const& operand : operands) {
    os << " " << operand;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os,Model const& model) {
  int count{};
  for (auto const& entry : model) {
    os << NL << "equation[" << count++ << "]";
    os << " " << entry.first << " =? " << entry.second;
  }
  return os;
}

using Args = std::vector<std::string>;

using Operators = std::string;
Result eval(Operators const& operators,Operands const& operands) {
  std::cout << NL << "eval(" << operators << "," << operands << ")";
  Result result{};
  // Evaluate using reversed polish notation
  std::stack<Integer> vals{operands.rbegin(),operands.rend()};
  result = vals.top();vals.pop();
  for (auto const& op : operators) {
    auto x = vals.top();vals.pop();
    std::cout << NL << T << op << " " << result << " " << x;
    switch (op) {
      case '+': result += x;break;
      case '*': result *= x;break;
      default:std::cerr << "ERROR, unknown operator " << op;break;
    }
    std::cout << " = " << result;
  }
  return result;
}

Operators to_operators(int index,auto length) {
  Operators result{};
  std::bitset<32> bits = index;
  for (int i=0;i<length;++i) {
    if (bits[i]) result.push_back('*');
    else result.push_back('+');
  }
  return result;
}


namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    Result acc{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      std::cout << model;
      for (auto const& entry : model) {
        auto y = entry.first;
        auto const& operands = entry.second;
        auto N = operands.size()-1;
        auto M = 1 << N; // 2^N
        for (auto i=decltype(M){};i<M;++i) {
          auto ops = to_operators(i,N);
          if (y == eval(ops,operands)) {
            acc += y;
            std::cout << NL << ops << " on " << operands << " is " << y << " OK acc:" << acc;
            break;
          }
//          else {
//            std::cout << NL << ops << " on " << operands << " is " << y << " FAILED";
//          }
        }
      }
      result = acc;
    }
    return result; // 6167992527400 is too low
                   // 6231007345478 ok
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
  std::vector<int> states = {1};
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
