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
using Result = std::string;
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

Numbers to_transformed(Number number) {
  Numbers result{};
  //If the stone is engraved with the number 0, it is replaced by a stone engraved with the number 1.
  if (is_zero(number)) {
    result.push_back(1);
  }

  //If the stone is engraved with a number that has an even number of digits, it is replaced by two stones. The left half of the digits are engraved on the new left stone, and the right half of the digits are engraved on the new right stone. (The new numbers don't keep extra leading zeroes: 1000 would become stones 10 and 0.)
  else if (has_even_digit_count(number)) {
    auto [left,right] = to_split_number(number);
    result.push_back(left);
    result.push_back(right);
  }
  //If none of the other rules apply, the stone is replaced by a new stone; the old stone's number multiplied by 2024 is engraved on the new stone.
  else {
    result.push_back(number*2024);
  }

  return result;
}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      int blink_count = (args.arg.contains("step"))?std::stoi(args.arg["step"]):25;
      std::cout << NL << "step:" << blink_count;
      auto model = parse(in);
      std::cout << NL << "Initial arrangement:";
      std::cout << NL << model;

      using Key = Number;    // Core component being transformed
      using State = std::pair<int, Key>; // Unique identifier for memoization

      auto transform_fn = [](Key k) -> std::vector<Key> {
          return to_transformed(k); // Replace with actual transformation logic
      };

      auto state_fn = [](int steps, Key k) -> State {
          return {steps, k}; // Combine steps and key into a unique state
      };

      int remaining_steps = 25;

      result = std::to_string(aoc::dfs::find_count<Key, Integer, State>(
          blink_count,
          model,
          transform_fn,
          state_fn
      ));
    }
    return result;
  }

  std::optional<Result> solve_for_1(std::istream& in,Args args) {
    args.arg["step"] = "1";
    return solve_for(in, args);
  }

  std::optional<Result> solve_for_6(std::istream& in,Args args) {
    args.arg["step"] = "6";
    return solve_for(in, args);
  }
  std::optional<Result> solve_for_25(std::istream& in,Args args) {
    args.arg["step"] = "25";
    return solve_for(in, args);
  }
  std::optional<Result> solve_for_75(std::istream& in,Args args) {
    args.arg["step"] = "75";
    return solve_for(in, args);
  }

}

namespace part2 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part2";
    result = part1::solve_for_75(in, args);
    return result;
  }
}

int main(int argc, char *argv[]) {
  aoc::application app{};
  app.add_solve_for("test0",part1::solve_for_1,"example.txt");
  app.add_solve_for("test1",part1::solve_for_6,"example2.txt");
  app.add_solve_for("test2",part1::solve_for_25,"example2.txt");
  app.add_solve_for("1",part1::solve_for,"puzzle.txt");
  app.add_solve_for("2",part2::solve_for,"example.txt");
  app.run(argc, argv);
  app.print_result();
  /*

   Xcode Debug -O2

   >day_0 -all
   
   For my input:
                  
   ANSWERS
   duration:0ms answer[part test0 in:example.txt] 7
   duration:0ms answer[part test1 in:example2.txt] 22
   duration:6ms answer[part test2 in:example2.txt] 55312
   duration:11ms answer[part 1 in:puzzle.txt] 231278
   duration:226ms answer[part 2 in:example.txt] 149161030616311
   
   */
  return 0;

}
