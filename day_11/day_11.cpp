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
#include <filesystem>

// Try to read the path to teh actual working directory
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

auto const NL = "\n";
auto const T = "\t";
auto const NT = "\n\t";

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = Integer;
using Number = Integer;
using Numbers = std::vector<Number>;

// Overload for std::vector
template <std::ranges::range R>
requires std::same_as<R, std::vector<typename R::value_type>>
std::ostream& operator<<(std::ostream& os, const R& vec) {
    os << '[';
    for (auto const& elem : vec) {
        os << elem << (&elem == &vec.back() ? "" : ", ");
    }
    os << ']';
    return os;
}

using Model = Numbers;

Model parse(auto& in) {
  using namespace aoc::parsing;
  Model result{};
  auto numbers = Splitter{in}.splits(' ');
  for (auto const& number : numbers) {
    std::cout << NL << "number:" << number.str();
    result.push_back(std::stol(number));
  }
  
  return result;
}

using Args = std::vector<std::string>;

bool is_zero(Number num) {return num==0;}
bool is_even(Number num) {
  return (num%2==0);
}
Number to_digit_count(Number number) {
  return static_cast<Number>(std::log10(number)) + 1;
}
bool has_even_digit_count(Number number) {
  Number count = to_digit_count(number);
  return is_even(count);
}
std::pair<Number,Number> to_split_number(Number number) {
  Number count = to_digit_count(number);
  int divisor = static_cast<int>(std::pow(10, count/2));
  return {number/divisor,number%divisor};
}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      int blink_count = (args.size()>0)?std::stoi(args[0]):25;
      auto model = parse(in);
      auto before = model;
      std::cout << NL << "Initial arrangement:";
      std::cout << NL << before;
      for (int blink=1;blink<=blink_count;++blink) {
        Numbers after{};
        for (int i=0;i<before.size();++i) {
          auto number = before[i];

          //If the stone is engraved with the number 0, it is replaced by a stone engraved with the number 1.
          if (is_zero(number)) {
            if (blink<10) std::cout << NL << T << "0 --> 1";
            after.push_back(1);
          }

          //If the stone is engraved with a number that has an even number of digits, it is replaced by two stones. The left half of the digits are engraved on the new left stone, and the right half of the digits are engraved on the new right stone. (The new numbers don't keep extra leading zeroes: 1000 would become stones 10 and 0.)
          else if (has_even_digit_count(number)) {
            auto [left,right] = to_split_number(number);
            if (blink<10) std::cout << NL << T << number << " --> " << left << " " << right;
            after.push_back(left);
            after.push_back(right);
          }
          //If none of the other rules apply, the stone is replaced by a new stone; the old stone's number multiplied by 2024 is engraved on the new stone.
          else {
            after.push_back(number*2024);
            if (blink<10) std::cout << NL << T << number << " --> " << after.back();
          }
        }
        if (blink<10) {
          std::cout << NL << "After " << blink << " blink(s)";
          std::cout << NL << "" << after;
        }
        else if (blink%1000==0) {
          std::cout << NL << "After " << blink << " blink(s)";
        }
        before = after;
      }
      // 1: 1 2024 1 0 9 9 2021976
      // 2: 2097446912 14168 4048 2 0 2 4 40 48 2024 40 48 80 96 2 8 6 7 6 0 3 2
      result = before.size();
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
  for (int i=1;i<argc;++i) {
    args.push_back(argv[i]);
  }

  std::filesystem::path working_dir{"../.."};
  if (auto dir = get_working_dir()) {
    working_dir = *dir;
  }
  else {
    std::cout << NL << "No working directory path configured";
  }
  std::cout << NL << "Using working_dir " << working_dir;

  Answers answers{};
  std::vector<std::chrono::time_point<std::chrono::system_clock>> exec_times{};
  exec_times.push_back(std::chrono::system_clock::now());
//  std::vector<int> states = {11};
  std::vector<int> states = {10};
  for (auto state : states) {
    switch (state) {
      case 11: {
        std::filesystem::path file{working_dir / "example.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1 Example",part1::solve_for(in,{"1"})});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 12: {
        std::filesystem::path file{working_dir / "example2.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1 Example2",part1::solve_for(in,{"6"})});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 10: {
        std::filesystem::path file{working_dir / "puzzle.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1     ",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 21: {
        std::filesystem::path file{working_dir / "example.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2 Example",part2::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 20: {
        std::filesystem::path file{working_dir / "puzzle.txt"};
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
