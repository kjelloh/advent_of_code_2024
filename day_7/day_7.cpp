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

auto const NL = "\n";
auto const T = "\t";
auto const NT = "\n\t";

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = std::string;
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

using Operators = std::string;
Integer eval(Operators const& operators,Operands const& operands) {
//  std::cout << NL << "eval(" << operators << "," << operands << ")";
  Integer result{};
  // Evaluate using reversed polish notation
  std::stack<Integer> vals{operands.rbegin(),operands.rend()};
  result = vals.top();vals.pop();
  for (auto const& op : operators) {
    auto x = vals.top();vals.pop();
//    std::cout << NL << T << op << " " << result << " " << x;
    switch (op) {
      case '+': result += x;break;
      case '*': result *= x;break;
      case '&': {
        auto s1 = std::to_string(result);
        auto s2 = std::to_string(x);
        auto s = s1 + s2;
        result = std::stol(s);
      } break;
      default:std::cerr << "ERROR, unknown operator " << op;break;
    }
//    std::cout << " = " << result;
  }
  return result;
}

namespace part1 {
  Operators to_operators(int index,auto length) {
    Operators result{};
    std::bitset<32> bits = index;
    for (int i=0;i<length;++i) {
      if (bits[i]) result.push_back('*');
      else result.push_back('+');
    }
    return result;
  }

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    Integer acc{};
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
            std::cout << NL << T << ops << " on " << operands << " is " << y << " OK acc:" << acc;
            break;
          }
        }
      }
      result = std::to_string(acc);
    }
    return result; // 6167992527400 is too low
                   // 6231007345478 ok
  }
}

namespace part2 {
  Operators to_operators(int index,auto length) {
    Operators result{};
    // interpret index in base 3
    std::array<char,3> const OPS{'&','*','+'};
    for (int i=0;i<length;++i) {
      result.push_back(OPS[index % 3]);
      index /= 3;
    }
    return result;
  }

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    Integer acc{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      std::cout << model;
      for (auto const& entry : model) {
        std::cout << NL << "processing:" << entry.first << " : " << entry.second;
        auto y = entry.first;
        auto const& operands = entry.second;
        auto N = operands.size()-1;
        auto M = std::pow(3,N); // 3^N
        int loop_count{};
        for (auto i=decltype(M){};i<M;++i) {
//          if (loop_count++ % 20000 == 0) std::cout << '.';
          auto ops = to_operators(i,N);
          if (y == eval(ops,operands)) {
            acc += y;
            std::cout << NL << T << ops << " on " << operands << " is " << y << " OK acc:" << acc;
            break;
          }
        }
      }
      result = std::to_string(acc);
    }
    return result;
  }
}

int main(int argc, char *argv[]) {
  aoc::application app{};
  app.add_solve_for("1",part1::solve_for,"example.txt");
  app.add_solve_for("1",part1::solve_for,"puzzle.txt");
  app.add_solve_for("2",part2::solve_for,"example.txt");
  app.add_solve_for("2",part2::solve_for,"puzzle.txt");
  app.run(argc, argv);
  app.print_result();
  /*

   Xcode Debug -O2

   >day_7 -all
   
   For my input:
       
   ANSWERS
   duration:1ms answer[part 1 in:example.txt] 3749
   duration:273ms answer[part 1 in:puzzle.txt] 6231007345478
   duration:0ms answer[part 2 in:example.txt] 11387
   duration:26283ms answer[part 2 in:puzzle.txt] 333027885676693

   */
  return 0;

}
