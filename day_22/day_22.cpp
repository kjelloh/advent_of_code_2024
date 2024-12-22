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

using aoc::raw::NL;
using aoc::raw::T;
using aoc::raw::NT;

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = aoc::raw::Line;
using Model = std::vector<Integer>;

Model parse(auto& in) {
  using namespace aoc::parsing;
  Model result{};
  auto lines = Splitter{in}.lines();
  for (auto const& [lx,line] : aoc::views::enumerate(lines)) {
    std::cout << NL << T << T << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
    result.push_back(std::stoi(line));
  }
  return result;
}

using Args = std::vector<std::string>;

namespace test {

  // Adapt to expected for day puzzle
  struct LogEntry {
    aoc::raw::Lines lines{};
    bool operator==(LogEntry const& other) const {
      bool result{true};
      return result;
    }
  };

  std::ostream& operator<<(std::ostream& os,LogEntry const& entry) {
    using aoc::raw::operator<<;
    os << entry.lines;
    return os;
  }

  using LogEntries = aoc::test::LogEntries<LogEntry>;

  LogEntries parse(auto& doc_in) {
    std::cout << NL << T << "test::parse";
    LogEntries result{};
    using namespace aoc::parsing;
    auto sections = Splitter{doc_in}.same_indent_sections();
    for (auto const& [sx,section] : aoc::views::enumerate(sections)) {
      std::cout << NL << "---------- section " << sx << " ----------";
      result.push_back({aoc::parsing::to_raw(section)});
      for (auto const& [lx,line] : aoc::views::enumerate(section)) {
        std::cout << NL << T << T << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
      }
    }
    return result;
  }

  Integer to_evolved(Integer rn) {
    Integer result{rn};
    //In particular, each buyer's secret number evolves into the next secret number in the sequence via the following process:
    //
    //Calculate the result of multiplying the secret number by 64. Then, mix this result into the secret number. Finally, prune the secret number.
    //To mix a value into the secret number, calculate the bitwise XOR of the given value and the secret number. Then, the secret number becomes the result of that operation. (If the secret number is 42 and you were to mix 15 into the secret number, the secret number would become 37.)
    //To prune the secret number, calculate the value of the secret number modulo 16777216. Then, the secret number becomes the result of that operation. (If the secret number is 100000000 and you were to prune the secret number, the secret number would become 16113920.)
    result = (result xor (result << 6)) % 16777216;
    
    //Calculate the result of dividing the secret number by 32. Round the result down to the nearest integer. Then, mix this result into the secret number. Finally, prune the secret number.
//    auto y2 = result >> 5; // round vs trucation?
//    result = result xor y2;
//    result = result % 16777216;
    result = (result xor (result >> 5)) % 16777216;
    //Calculate the result of multiplying the secret number by 2048. Then, mix this result into the secret number. Finally, prune the secret number.
    auto y3 = (result << 11);
    result = result xor y3;
    result = result % 16777216;
    return result;
  }

  std::vector<Integer> to_history(Integer seed,Integer const& N) {
    std::vector<Integer> result{};
    result.push_back(seed);
    auto sn = seed;
    for (int i=0;i<N;++i) {
      sn = to_evolved(sn);
      result.push_back(sn % 10); // The 'one digit'
    }
    return result;
  }

  std::optional<Result> test0(Args args) {
    std::cout << NL << NL << "test0";
    
    // If the secret number is 100000000 and you were to prune the secret number, the secret number would become 16113920
    std::cout << NL << 100000000 << " - pruned -> " << 100000000 % 16777216 << " should be 16113920";
    
    //If the secret number is 42 and you were to mix 15 into the secret number, the secret number would become 37
    std::cout << NL << 42 << " - mixed -> " << (42 xor 15) << " should be 37";

    
    // So, if a buyer had a secret number of 123
    // that buyer's next ten secret numbers would be:

    int seed = 123;
    std::vector<Integer> expected{
      15887950
      ,16495136
      ,527345
      ,704524
      ,1553684
      ,12683156
      ,11100544
      ,12249484
      ,7753432
      ,5908254};
    
    Integer rn = seed;
    std::vector<Integer> numbers{};
    for (int i=0;i<10;++i) {
      rn = to_evolved(rn);
      numbers.push_back(rn);
      std::cout << NL << "sn:" << rn << " expected:" << expected[i];
    }
    if (numbers == expected) {
      std::cout << NL << "PASSED";
      return "PASSED ok";
    }
    return std::nullopt;
  }

  std::optional<Result> test111(auto& in, auto& doc_in,Args args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "test1";
    if (in) {
      auto model = ::parse(in);
      if (doc_in) {
        auto log = test::parse(doc_in);
        std::cout << NL << log[31];
        std::cout << NL << log[32];
      }
      for (auto seed : model) {
        for (int j=0;j<2000;++j) seed = to_evolved(seed);
        std::cout << NL << "sn:" << seed;
      }

    }
    return result;
  }

  Integer to_ones_digit(Integer sn) {
    return sn % 10;
  }

  std::vector<Integer> to_steps(std::vector<Integer> const& history) {
    std::vector<Integer> result{history};
    // Transform into steps
    // Assume first in history is seed and can be safelly overwritten wih the step entry[1]-seed;
    for (int i=0;i<result.size()-1;++i) {
      auto step = (result[i+1]) - (result[i]);
      result[i] = step; // replace left with step right-left
    }
    result.pop_back();
    return result;
  }

  std::optional<Result> test2(Args args) {
    std::cout << NL << NL << "test0";
        
    std::vector<Integer> sns{
      123
      ,15887950
      ,16495136
      ,527345
      ,704524
      ,1553684
      ,12683156
      ,11100544
      ,12249484
      ,7753432
      ,5908254};

    for (auto sn : sns) {
      std::cout << NL << sn << " ones count: " << to_ones_digit(sn);
    }
    //For the buyer with an initial secret number of 1, changes -2,1,-1,3 first occur when the price is 7.
    // Buyer byer_0{1}; monkey.sell_on({-2,1,-1,3}).to_buyer(buyer_0)
    //For the buyer with initial secret 2, changes -2,1,-1,3 first occur when the price is 7.
    //For the buyer with initial secret 3, the change sequence -2,1,-1,3 does not occur in the first 2000 changes.
    //For the buyer starting with 2024, changes -2,1,-1,3 first occur when the price is 9.
    
    // So, we are looking for the 'Trend' {d1,d2,d3,d4} of price chages (price steps) so that,
    // If we bou from a seller at the price after d4 change, we will maximice the income from all sellers.
    // Note: It is the total income, not the price per buyer, we want to maximise.
    
    // 1. What are the search space of {d1,d2,d3,d4}?
    // A: It is -9,-9,-9,-9 to 9,9,9,9 is it not?
    
    {
      Model model{
        123
      };
      
      for (auto const& seed : model) {
        auto history = to_history(seed, 2000);
        auto trend = to_steps(history);
        std::cout << NL << "trend:" << seed;
        for (int i=0;i<9;++i) {
          std::cout << NL << T << trend[i];
        }
      }
    }
    
    {
      Model model{
         1
        ,2
        ,3
        ,2024
      };
      std::vector<std::vector<Integer>> cached{};
      std::vector<std::vector<Integer>> histories{};
      for (auto const& seed : model) {
        cached.push_back({});
        histories.push_back({});
        auto history = to_history(seed, 2000);
        auto trend = to_steps(history);
        cached.back() = trend;
        histories.back() = history;
        std::cout << NL << "trend:" << seed;
        for (int i=0;i<9;++i) {
          std::cout << NL << T << trend[i];
        }
      }
      Integer acc{};
      std::vector<Integer> key{-2,1,-1,3};
      for (int i=0;i<cached.size();++i) {
        auto entry = cached[i];
        auto history = histories[i];
        std::cout << NL << i << " history:" << history.size() << " entry:" << entry.size();
        auto iter = std::search(entry.begin(), entry.end(), key.begin(), key.end());
        if (iter != entry.end()) {
          auto index = std::distance(entry.begin(),iter)+4;
          std::cout << T << " first match [" << index << "]";
          for (int j=static_cast<int>(index-4);j<=static_cast<int>(index+4);++j) {
            if (j<history.size()) std::cout << " " << j << ":" << history[j];
            else std::cout << j << ":-";
          }
          auto price = history[index];
          std::cout << " price:" << price;
          acc += price;
        }
        else {
          std::cout << T << " void";
        }
      }
      return std::to_string(acc);
    }
    
    return std::nullopt;
  }


  std::optional<Result> test211(auto& in, auto& doc_in,Args args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "test2";
    if (in) {
      auto model = ::parse(in);
      if (doc_in) {
        auto log = test::parse(doc_in);
        if (log.size()>36) {
          std::cout << NL << log[39];
          std::cout << NL << log[40];
        }
        else {
          std::cout << NL << "Doc is to short. Are you sure you have solved part 1?";
        }
      }
      std::vector<std::vector<Integer>> cached{};
      std::vector<std::vector<Integer>> histories{};
      for (auto const& seed : model) {
        cached.push_back({});
        histories.push_back({});
        auto history = to_history(seed, 2000);
        auto trend = to_steps(history);
        cached.back() = trend;
        histories.back() = history;
        std::cout << NL << "trend:" << seed;
        for (int i=0;i<9;++i) {
          std::cout << NL << T << trend[i];
        }
      }
      
      // Brute force?
      std::vector<int> range{-9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

      // Generate all combinations (with replacement)
      Integer best{0};
      std::vector<int> best_key{-9,-9,-9,-9};
      for (int i1 : range) {
        for (int i2 : range) {
          for (int i3 : range) {
            for (int i4 : range) {
              std::vector<int> key{i1, i2, i3, i4};
              Integer acc{};
              for (int i=0;i<cached.size();++i) {
                auto entry = cached[i];
                auto history = histories[i];
                auto iter = std::search(entry.begin(), entry.end(), key.begin(), key.end());
                if (iter != entry.end()) {
                  auto index = std::distance(entry.begin(),iter)+4;
                  auto price = history[index];
                  acc += price;
                }
//                else {
//                  using aoc::raw::operator<<;
//                  std::cout << NL << key << " void";
//                }
              }
              if (acc > best) {
                best = acc;
                best_key = key;
                std::cout << NL << "best:" << best;
              }
            }
          }
        }
      }
      using aoc::raw::operator<<;
      std::cout << NL << "best:" << best << " for key:" << best_key;
    }
    return std::nullopt;
  }
}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      Integer acc{};
      auto model = parse(in);
      for (auto seed : model) {
        for (int j=0;j<2000;++j) seed = test::to_evolved(seed);
        std::cout << NL << "sn:" << seed;
        acc += seed;
      }
      result = std::to_string(acc);
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

  Answers answers{};
  std::vector<std::chrono::time_point<std::chrono::system_clock>> exec_times{};
  exec_times.push_back(std::chrono::system_clock::now());
  std::vector<int> states = {211};
//  std::vector<int> states = {0,111};
  for (auto state : states) {
    switch (state) {
      case 0: {
        answers.push_back({"test0",test::test0(args)});
      } break;
      case 111: {
        auto doc_file = aoc::to_working_dir_path("doc.txt");
        std::ifstream doc_in{doc_file};
        auto file = aoc::to_working_dir_path("example.txt");
        std::ifstream in{file};
        if (in and doc_in) answers.push_back({"Part 1 Test Example vs Log",test::test111(in,doc_in,args)});
        else std::cerr << "\nSORRY, no file " << file << " or doc_file " << doc_file;
      } break;
      case 11: {
        auto file = aoc::to_working_dir_path("example.txt");
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1 Example",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
      } break;
      case 10: {
        auto file = aoc::to_working_dir_path("puzzle.txt");
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1     ",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
      } break;
      case 2: {
        answers.push_back({"test2",test::test2(args)});
      } break;
      case 211: {
        auto doc_file = aoc::to_working_dir_path("doc.txt");
        std::ifstream doc_in{doc_file};
        auto file = aoc::to_working_dir_path("example2.txt");
        std::ifstream in{file};
        if (in and doc_in) answers.push_back({"Part 2 Test Example vs Log",test::test211(in,doc_in,args)});
        else std::cerr << "\nSORRY, no file " << file << " or doc_file " << doc_file;
      } break;
      case 21: {
        auto file = aoc::to_working_dir_path("example2.txt");
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2 Example",part2::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
      } break;
      case 20: {
        auto file = aoc::to_working_dir_path("puzzle.txt");
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2     ",part2::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
      } break;
      default:{std::cerr << "\nSORRY, no action for state " << state;} break;
    }
    exec_times.push_back(std::chrono::system_clock::now());
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
