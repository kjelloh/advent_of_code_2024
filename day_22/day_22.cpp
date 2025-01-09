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

namespace test {

  std::vector<aoc::raw::Lines> to_examples(aoc::parsing::Sections const& sections) {
    std::vector<aoc::raw::Lines> result{};
    result.push_back({});
    result.back().append_range(aoc::parsing::to_raw(sections[29]));

    result.push_back({});
    result.back().append_range(aoc::parsing::to_raw(sections[56]));

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
    result.push_back(seed % 10);
    auto sn = seed;
    for (int i=0;i<N;++i) {
      sn = to_evolved(sn);
      result.push_back(sn % 10); // The 'one digit'
    }
    return result;
  }

  bool test0(std::optional<aoc::parsing::Sections> const& sections,Args args) {
    // This function is called by aoc::application if registered with add_test(test::test0)
    // Extract test data from provided sections from the day web page text.
    // See zsh-script pull_text.zsh for support to fetch advent of code day web page text to doc.txt
    std::cout << NL << "test0";
    if (sections) {
      bool result{true};
      std::cout << NL << T << "sections ok";
      
      // If the secret number is 100000000 and you were to prune the secret number, the secret number would become 16113920
      {
        int sn{100000000};
        auto pruned{100000000 % 16777216};
        int expected{16113920};
        result = result and (pruned == expected);
        std::print("\n{} - pruned -> {} is {}? {}",sn,pruned,expected,result);
      }
      
      //If the secret number is 42 and you were to mix 15 into the secret number, the secret number would become 37
      if (result) {
        int sn{42};
        int to_mix_in{15};
        auto mixed{sn xor to_mix_in};
        int expected{37};
        result = result and (mixed == expected);
        std::print("\n{} - pruned -> {} is {}? {}",sn,mixed,expected,result);
      }
      
      if (result) {
        // So, if a buyer had a secret number of 123
        // that buyer's next ten secret numbers would be:
        //---------- section 27 ----------
        //    line[0]:8 "15887950"
        //    line[1]:8 "16495136"
        //    line[2]:6 "527345"
        //    line[3]:6 "704524"
        //    line[4]:7 "1553684"
        //    line[5]:8 "12683156"
        //    line[6]:8 "11100544"
        //    line[7]:8 "12249484"
        //    line[8]:7 "7753432"
        //    line[9]:7 "5908254"
        int seed = 123;
        auto expected = aoc::parsing::to_ints<Integer>((*sections)[27]);

        Integer rn = seed;
        std::vector<Integer> numbers{};
        for (int i=0;i<10;++i) {
          rn = to_evolved(rn);
          numbers.push_back(rn);
          std::cout << NL << "sn:" << rn << " expected:" << expected[i];
        }
        return result and (numbers == expected);
      }
    }
    else {
      std::cout << NL << T << "NO sections";
    }
    return false;
  }

  bool test111(std::optional<aoc::parsing::Sections> const& sections,Args args)  {
    if (sections) {
      auto examples = to_examples(*sections);
      if (examples.size()>0) {
        bool result{true};
        auto in = aoc::test::to_example_in(examples[0]);
        auto model = parse(in);
        //---------- section 31 ----------
        //    line[0]:69 "   In a single day, buyers each have time to generate 2000 new secret"
        //    line[1]:72 "   numbers. In this example, for each buyer, their initial secret number"
        //    line[2]:60 "   and the 2000th new secret number they would generate are:"
        auto ints_31 = aoc::parsing::to_ints((*sections)[31]);
        auto secret_count{ints_31[0]};
        std::print("\ntest {} new secrets",secret_count);
        //---------- section 32 ----------
        //    line[0]:10 "1: 8685429"
        //    line[1]:11 "10: 4700978"
        //    line[2]:13 "100: 15273692"
        //    line[3]:13 "2024: 8667524"
        auto ints_32 = aoc::parsing::to_ints((*sections)[32]);
        for (auto const& [ix,seed] : aoc::views::enumerate(model)) {
          auto evolved = seed;
          for (int j=0;j<secret_count;++j) evolved = to_evolved(evolved);
          auto expected{ints_32[ix*2+1]};
          result = result and (evolved == expected);
          std::print("\n\t{}: evolved:{} expected:{} {}",seed,evolved,expected,result);
          if (not result) {
            std::cout << " FAILED";
            break;
          }
        }
        return result;
      }
      else {
        std::cout << NL << T << "NO examples";
      }
    }
    else {
      std::cout << NL << T << "NO sections";
    }
    return false;
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

  bool test2(std::optional<aoc::parsing::Sections> const& opt_sections,Args args)  {
    std::cout << NL << "test2";
    if (opt_sections) {
      auto const& sections = *opt_sections;
      int seed = 123;
      std::vector<Integer> expected_p1{seed};
      expected_p1.append_range(aoc::parsing::to_ints<Integer>(sections[27]));
      
      std::vector<int> expected_p2{};
      //---------- section 40 ----------
      //    line[0]:12 "3 (from 123)"
      //    line[1]:17 "0 (from 15887950)"
      //    line[2]:17 "6 (from 16495136)"
      //    line[3]:8 "5 (etc.)"
      //    line[4]:1 "4"
      //    line[5]:1 "4"
      //    line[6]:1 "6"
      //    line[7]:1 "4"
      //    line[8]:1 "4"
      //    line[9]:1 "2"
      for (auto const& line : sections[40]) {
        expected_p2.push_back(aoc::parsing::to_ints(line).front());
      }

      bool result{true};
      for (auto [ix,sn] : aoc::views::enumerate(expected_p1)) {
        if (ix < expected_p2.size()) {
          auto computed = to_ones_digit(sn);
          auto expected = expected_p2[ix];
          result = result and (computed == expected);
          std::print("\n{} from {} is expected:{}? {}",computed,sn,expected,result);
          if (not result) break;
        }
      }
      
      if (result) {
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
      
      if (result) {
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
        std::print("\nprice:{}",acc);
        result = result and  (acc == 23);
      }

      return result;
    }
    else {
      std::cout << NL << T << "NO sections";
    }
    return false;
  }


  bool test211(std::optional<aoc::parsing::Sections> const& opt_sections,Args args)  {
    if (opt_sections) {
      auto const& sections = *opt_sections;
      auto examples = to_examples(sections);
      if (examples.size()>1) {
        auto in = aoc::test::to_example_in(examples[1]);
        auto model = parse(in);
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
                  static int loop_count{};
                  if (loop_count++ % 4999 == 0) {
                    std::print("\n{} {} {} {} {}",i1,i2,i3,i4,i);
                    std::cout << std::flush;
                  }
                  auto entry = cached[i];
                  auto history = histories[i];
                  auto iter = std::search(entry.begin(), entry.end(), key.begin(), key.end());
                  if (iter != entry.end()) {
                    auto index = std::distance(entry.begin(),iter)+4;
                    auto price = history[index];
                    acc += price;
                  }
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
        std::cout << NL << "best:" << best << " for key:" << std::format("{}",best_key);
        return (best == 23);

      }
      else {
        std::cout << NL << T << "NO examples (to_examples not implemented?)";
      }
    }
    else {
      std::cout << NL << T << "NO sections (pull_text.zsh not executed or doc.txt empty?)";
    }

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
  struct VectorHash {
    template <typename T>
    std::size_t operator () (const std::vector<T>& vec) const {
      std::size_t hash_value = 0;
      for (const T& element : vec) {
        // The interwebs proposes...
        hash_value ^= std::hash<T>()(element) + 0x9e3779b9 + (hash_value << 6) + (hash_value >> 2);
      }
      return hash_value;
    }
  };

  using Key2Price = std::unordered_map<std::vector<Integer>,std::size_t,VectorHash>;

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part2";
    if (in) {
      auto model = parse(in);
      
      //For the buyer with an initial secret number of 1, changes -2,1,-1,3 first occur when the price is 7.
      // Buyer byer_0{1}; monkey.sell_on({-2,1,-1,3}).to_buyer(buyer_0)
      //For the buyer with initial secret 2, changes -2,1,-1,3 first occur when the price is 7.
      //For the buyer with initial secret 3, the change sequence -2,1,-1,3 does not occur in the first 2000 changes.
      //For the buyer starting with 2024, changes -2,1,-1,3 first occur when the price is 9.
      
      // So, we are looking for the 'Trend' {d1,d2,d3,d4} of price chages (price steps) so that,
      // If we buy from a seller at the price after d4 change, we will maximice the income from all sellers.
      // Note: It is the total income, not the price per buyer, we want to maximise.
      
      // 1. What are the search space of {d1,d2,d3,d4}?
      // A: It is -9,-9,-9,-9 to 9,9,9,9 is it not?

      
      // Pre-compute an unordered map for all four integer window over
      // the cost vectors
      // Custom hash function for std::vector<int>
      std::vector<Key2Price> key2price_vector{};
      std::vector<std::vector<Integer>> trends{};
      std::set<std::vector<Integer>> seen{};
      int loop_count{};
      for (auto const& seed : model) {
        key2price_vector.push_back({});
        trends.push_back({});
        auto history = test::to_history(seed, 2000);
        auto trend = test::to_steps(history);
        for (int i=0;i<trend.size()-3;++i) {
          std::vector<Integer> key{trend[i],trend[i+1],trend[i+2],trend[i+3]};
          if (++loop_count % 50000 == 0) std::cout << NL << "caching at " << std::format("{}",key);
          if (key2price_vector.back().contains(key)) continue; // only first match of interest
          key2price_vector.back()[key] = history[i+4]; // map to price (price vector one longer than steps)
          seen.insert(key);
        }
        trends.back() = trend;
      }
      std::cout << NL << "seeds:" << model.size();
      std::cout << NL << "built key2price_vector size:" << key2price_vector.size() << std::flush;
      // We only need to try the actual seen keys
      Integer best{0};
      std::vector<Integer> best_key{-9,-9,-9,-9};
      for (auto const& key : seen) {
        if (++loop_count % 5000 == 0) std::cout << NL << std::format("{}",key);
        Integer acc{};
        for (int i=0;i<trends.size();++i) {
          auto const& trend = trends[i];
          auto const& key2index = key2price_vector[i];
          if (key2index.contains(key)) {
            auto const cost = key2index.at(key);
            acc += cost;
          }
        }
        if (acc > best) {
          best = acc;
          best_key = key;
          std::cout << NL << std::format("{}",key) <<  " new best:" << best;
        }
      }
      using aoc::raw::operator<<;
      std::cout << NL << "best:" << best << " for key:" << best_key;
      result = std::to_string(best);
      // best:1914 for key:0,0,-1,1 too low (but index error in key match, -4 instead of -3)
      // best:1916 for key:0,0,-1,1 still too low (still to small search space?)
      // best:1925 for key:0,0,-1,1 (fixed a bug when optimising?)
            
    }
    return result;
  }
}

int main(int argc, char *argv[]) {
  aoc::application app{};
  app.add_to_examples(test::to_examples);
  app.add_test("test0",test::test0);
  app.add_test("test111",test::test111);
  app.add_solve_for("1",part1::solve_for,"example.txt");
  app.add_solve_for("1",part1::solve_for,"puzzle.txt");
  app.add_test("test2", test::test2);
  app.add_test("test211", test::test211);
  app.add_solve_for("2",part2::solve_for,"example1.txt");
  app.add_solve_for("2",part2::solve_for,"puzzle.txt");
  app.run(argc, argv);
  app.print_result();
  /*

   Xcode Debug -O2

   day_22 -all
      
   ANSWERS
   duration:0ms answer[part:"test0"] PASSED
   duration:1ms answer[part:"test111"] PASSED
   duration:0ms answer[part 1 in:example.txt] 37327623
   duration:120ms answer[part 1 in:puzzle.txt] 16894083306
   duration:1ms answer[part:"test2"] PASSED
   duration:18522ms answer[part:"test211"] PASSED
   duration:25ms answer[part 2 in:example1.txt] 23
   duration:33266ms answer[part 2 in:puzzle.txt] 1925
   
   */

}
