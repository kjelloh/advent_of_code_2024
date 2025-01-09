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
using ColumnCount = std::vector<int>;
using OptionalColumnCount = std::optional<ColumnCount>;
using LockOrKey = std::pair<OptionalColumnCount,OptionalColumnCount>;
using Model = std::vector<LockOrKey>;

template <>
struct std::formatter<LockOrKey> : std::formatter<std::string> {
  template<class FmtContext>
  FmtContext::iterator format(LockOrKey const& lock_or_key, FmtContext& ctx) const {
    if (lock_or_key.first) {
      std::format_to(ctx.out(),"lock:{}", *lock_or_key.first);
    }
    else if (lock_or_key.second) {
      std::format_to(ctx.out(),"key:{}", *lock_or_key.second);
    }
    else {
      std::format_to(ctx.out(),"??");
    }
    return ctx.out();
  }
};

std::ostream& operator<<(std::ostream& os,LockOrKey const& lock_or_key) {
  os << std::format("{}",lock_or_key);
  return os;
}

template <>
struct std::formatter<Model> : std::formatter<std::string> {
  template<class FmtContext>
  FmtContext::iterator format(Model const& model, FmtContext& ctx) const {
    for (auto const& lock_or_key : model) std::format_to(ctx.out(),"\n{}",lock_or_key);
    return ctx.out();
  }
};

Model parse(auto& in) {
  std::print("\nparse");
  using namespace aoc::parsing;
  Model result{};
  auto sections = Splitter{in}.sections();
  for (auto const& [sx,section] : aoc::views::enumerate(sections)) {
    std::cout << NL << "---------- section " << sx << " ----------";
    result.push_back({});
    ColumnCount col_count(section[0].size(),-1); // reduced for solid count '###..."
    for (auto const& [lx,line] : aoc::views::enumerate(section)) {
      std::cout << NL << T << T << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
      auto entry = line.str();
      for (int i=0;i<entry.size();++i) {
        if (entry[i]=='#') ++col_count[i];
      }
      if (lx==0) {
        if (std::all_of(entry.begin(), entry.end(),[](char ch){return ch == '.';})) {
          result.back().second = col_count; // mark key
        }
        else {
          result.back().first = col_count; // lock
        }
      }
      if (result.back().first) result.back().first = col_count;
      else result.back().second = col_count;
    }
  }
  return result;
}



namespace test {

  std::vector<aoc::raw::Lines> to_examples(aoc::parsing::Sections const& sections) {
    std::cout << NL << "To example";
    std::vector<aoc::raw::Lines> result;
    result.push_back({});
    for (int i=15;i<=19;++i) {
      result.back().append_range(aoc::parsing::to_raw(sections[i]));
      result.back().push_back("");
    }
    return result;
  }

  Integer to_fits_count(Model const& model) {
    Integer result{};
    for (int i=0;i<model.size();++i) {
      for (int j=i+1;j<model.size();++j) {
        auto const& lhs = model[i];
        auto const& rhs = model[j];
        ColumnCount result_counts{};
        ColumnCount lhs_counts{};
        ColumnCount rhs_counts{};
        if (lhs.first and rhs.second) {
          lhs_counts = *lhs.first;
          rhs_counts = *rhs.second;
        }
        else if (lhs.second and rhs.first) {
          lhs_counts = *lhs.second;
          rhs_counts = *rhs.first;
        }
        else {
          // skip
          continue;
        }
        std::transform(lhs_counts.begin(), lhs_counts.end(), rhs_counts.begin(), std::back_insert_iterator(result_counts), [](int c1,int c2){
          return c2+c1;
        });
        using aoc::raw::operator<<;
        std::cout << NL << "result_counts:" << result_counts;
        if (std::all_of(result_counts.begin(), result_counts.end(), [](int c){
          return c <= 5;
        })) {
          ++result;
          std::cout << NL << result;
        }
      }
    }
    return result;
  }

  bool test0(std::optional<aoc::parsing::Sections> const& sections,Args args) {
    std::cout << NL << "test0";
    if (sections) {
      std::cout << NL << T << "sections ok";
      auto examples = to_examples(*sections);
      if (examples.size()>0) {
        std::cout << NL << T << "examples ok";
        auto example_in = aoc::test::to_example_in(examples[0]);
        auto example_model = parse(example_in);
        std::cout << NL << std::format("\n{}",example_model);
        auto acc = to_fits_count(example_model);
        std::cout << NL << "acc:" << acc;
        return acc == 3;
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

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "test";
    if (in) {
      auto model = parse(in);
      auto acc = test::to_fits_count(model);
      std::cout << NL << "acc:" << acc;
      result = std::to_string(acc);
    }
    return result;
  }

}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      auto acc = test::to_fits_count(model);
      std::cout << NL << "acc:" << acc;
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

int main(int argc, char *argv[]) {
  
  aoc::application app{};
  app.add_to_examples(test::to_examples);
  app.add_test("test0", test::test0);
  app.add_solve_for("1",part1::solve_for,"example.txt");
  app.add_solve_for("1",part1::solve_for,"puzzle.txt");
  app.run(argc, argv);
  app.print_result();
  /*

   Xcode Debug -O2

   >day_25 -all
   
   For my input:
            
   ANSWERS
   duration:0ms answer[part:"test0"] PASSED
   duration:0ms answer[part 1 in:example.txt] 3
   duration:247ms answer[part 1 in:puzzle.txt] 3196
   
   */

}
