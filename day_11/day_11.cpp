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
#include <functional>

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

template<typename Key, typename Result, typename State>
Result find_count(
    int remaining_steps,
    Key initial_key,
    std::function<std::vector<Key>(Key)> const& transform_fn,
    std::function<State(int, Key)> const& state_fn,
    std::map<State, Result>& seen
) {
    if (remaining_steps == 0) {
        return Result(1); // Base case: count the initial state itself
    }

    State memo_state = state_fn(remaining_steps, initial_key);
    if (seen.contains(memo_state)) {
        return seen[memo_state];
    }

    Result result = Result(0);
    for (auto const& next_key : transform_fn(initial_key)) {
        result += find_count(remaining_steps - 1, next_key, transform_fn, state_fn, seen);
    }

    seen[memo_state] = result;
    return result;
}

// Overload for initial invocation
template<typename Key, typename Result, typename State>
Result find_count(
    int remaining_steps,
    std::vector<Key> const& initial_keys,
    std::function<std::vector<Key>(Key)> const& transform_fn,
    std::function<State(int, Key)> const& state_fn
) {
    Result result{};
    std::map<State, Result> seen;

    for (auto const& key : initial_keys) {
        result += find_count(remaining_steps, key, transform_fn, state_fn, seen);
    }

    return result;
}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      int blink_count = (args.size()>0)?std::stoi(args[0]):25;
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

      result = find_count<Key, Result, State>(
          blink_count,
          model,
          transform_fn,
          state_fn
      );
    }
    return result;
  }
}

namespace part2 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part2";
    result = part1::solve_for(in, args);
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
  std::vector<int> states = {11,126,1225,10,20};
  for (auto state : states) {
    switch (state) {
      case 11: {
        std::filesystem::path file{working_dir / "example.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1 Example 1 blink",part1::solve_for(in,{"1"})});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 126: {
        std::filesystem::path file{working_dir / "example2.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1 Example2 6 blinks",part1::solve_for(in,{"6"})});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 1225: {
        std::filesystem::path file{working_dir / "example2.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1 Example2 25 blinks",part1::solve_for(in,{"25"})});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 10: {
        std::filesystem::path file{working_dir / "puzzle.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1     ",part1::solve_for(in,{"25"})});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 20: {
        std::filesystem::path file{working_dir / "puzzle.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2     ",part2::solve_for(in,{"75"})});
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
   duration:0ms answer[Part 1 Example 1 blink] 7
   duration:0ms answer[Part 1 Example2 6 blinks] 22
   duration:6ms answer[Part 1 Example2 25 blinks] 55312
   duration:11ms answer[Part 1     ] 231278
   duration:249ms answer[Part 2     ] 274229228071551
   
   */
  return 0;
}
