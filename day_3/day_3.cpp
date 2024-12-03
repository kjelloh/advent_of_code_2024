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
struct Expression {
  std::string f{"nop"};
  int left_op{0};
  int right_op{0};
};
using Model = std::vector<Expression>;

Model parse(auto& in) {
  std::cout << "\n<BEGIN parse>";
  Model result{};
  std::string line{};
  while (std::getline(in,line)) {
    std::cout << "\n\tline:" << std::quoted(line);
    // part 1, match 'mul(a,b)' into groups for a and b
    // std::regex pattern(R"(mul\((\d+),(\d+)\))"); // match 'mul(a,b)' into a,b subgroups
    // part 2, match either 'word()' or 'word(a,b)'
    std::regex pattern(R"((([a-zA-Z']+)\(\))|(([a-zA-Z]+)\((\d+),(\d+)\)))");
    auto matches_begin = std::sregex_iterator(line.begin(), line.end(), pattern);
    auto matches_end = std::sregex_iterator();
    for (std::sregex_iterator iter = matches_begin; iter != matches_end; ++iter) {
      std::smatch match = *iter;
      std::string s = match.str();
      std::cout << "\nop:" << s;
      Expression exp{};
      if (match[1].matched) {  // Case 1: Empty parentheses
        std::cout << "\nMatched text: " << match[1] << " (word with empty parentheses)";
        exp.f = match[1];
      }
      else if (match[3].matched) {  // Case 2: Integers in parentheses
        std::cout << "\nMatched text: " << match[3] << " (word with integers)";
        std::cout << "\n\tWord: " << match[4] << ", a: " << match[5] << ", b: " << match[6] << std::endl;
        exp.f = match[4].str();
        exp.left_op = std::stoi(match[5].str());
        exp.right_op = std::stoi(match[6].str());
      }
      std::cout << " --> " << std::quoted(exp.f) << " " << exp.left_op << " " << exp.right_op;
      result.push_back(exp);
    }
  }
  std::cout << "\n<END parse>";
  return result;
}

using Args = std::vector<std::string>;

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      Result acc{};
      auto model = parse(in);
      for (auto const& exp : model) {
        acc += exp.left_op*exp.right_op;
      }
      result = acc;
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
      bool disabled{false};
      Result acc{};
      for (auto const& exp : model) {
        if (exp.f.find("don't") != std::string::npos) {
          disabled = true;
          std::cout << "\nOFF";
        }
        else if (exp.f.find("do(") != std::string::npos) {
          disabled = false;
          std::cout << "\nON";
        }
        else if (exp.f.find("mul") != std::string::npos and not disabled) {
          acc += exp.left_op*exp.right_op;
          std::cout << "\n" << exp.f << " " << exp.left_op << " " << exp.right_op << " --> acc:" << acc;
        }
        else {
          std::cout << "\nNO OPERATION ON:" << std::quoted(exp.f);
        }
      }
      result = acc;
    }
    return result;
  }
}

using Answers = std::vector<std::pair<std::string,std::optional<Result>>>;
int main(int argc, char *argv[])
{
  Args args{};
  for (int i=0;i<argc;++i) {
    args.push_back(argv[i]);
  }
  Answers answers{};
  std::vector<std::chrono::time_point<std::chrono::system_clock>> exec_times{};
  exec_times.push_back(std::chrono::system_clock::now());
  std::vector<int> states = {3};
  for (auto state : states) {
    switch (state) {
      case 0: {
        std::filesystem::path file{"../../example.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1 Test",part1::solve_for(in,args)});
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
        std::filesystem::path file{"../../example2.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2 Test",part2::solve_for(in,args)});
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
