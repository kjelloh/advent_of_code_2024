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

std::ostream& operator<<(std::ostream& os,LockOrKey const& lock_or_key) {
  using aoc::raw::operator<<;
  if (lock_or_key.first) std::cout << "lock:" << *lock_or_key.first;
  else if (lock_or_key.second) std::cout << "key:" << *lock_or_key.second;
  else os << "?";
  return os;
}

Model parse(auto& in) {
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
          std::cout << NL << "Parsing key";
        }
        else {
          result.back().first = col_count; // lock
          std::cout << NL << "Parsing Lock";
        }
      }
      if (result.back().first) result.back().first = col_count;
      else result.back().second = col_count;
    }
  }
  return result;
}

struct Args {
  std::map<std::string,std::string> arg{};
  std::set<std::string> options{};
};

namespace test {

  // Adapt to expected for day puzzle
  struct Expected {
    bool operator==(Expected const& other) const {
      bool result{true};
      return result;
    }
  };

  std::ostream& operator<<(std::ostream& os,Expected const& entry) {
    return os;
  }

  using Expecteds = aoc::test::Expecteds<Expected>;

  aoc::parsing::Sections parse_doc(Args const& args) {
    std::cout << NL << T << "parse puzzle doc text";
    aoc::parsing::Sections result{};
    using namespace aoc::parsing;
    std::ifstream doc_in{aoc::to_working_dir_path("doc.txt")};
    auto sections = Splitter{doc_in}.same_indent_sections();
    for (auto const& [sx,section] : aoc::views::enumerate(sections)) {
      std::cout << NL << "---------- section " << sx << " ----------";
      result.push_back(section);
      for (auto const& [lx,line] : aoc::views::enumerate(section)) {
        std::cout << NL << T << T << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
      }
    }
    return result;
  }

  aoc::raw::Lines to_example(aoc::parsing::Sections const& sections) {
    std::cout << NL << "To example";
    aoc::raw::Lines result;
    for (int i=15;i<=19;++i) {
      std::cout << NL << i;
      if (i>15) result.push_back({});
      for (auto const& line : sections[i]) {
        result.push_back(line);
        std::cout << NL << T << line.str();
      }
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

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "test";
    if (in) {
      auto model = parse(in);
      auto doc = parse_doc(args);
      auto example_lines = to_example(doc);
      if (args.options.contains("-to_example")) {
        std::ostringstream oss{};
        auto example_file = aoc::to_working_dir_path("example.txt");
        if (aoc::raw::write_to_file(example_file, example_lines)) {
          oss << "Created " << example_file;
        }
        else {
          oss << "Sorry, failed to create file " << example_file;
        }
        return oss.str();
      }
      std::ostringstream oss{};
      aoc::raw::write_to(oss, example_lines);
      std::istringstream example_in{oss.str()};
      auto example_model = ::parse(example_in);
      for (auto const& entry : example_model) {
        using ::operator<<;
        std::cout << NL << entry;
      }
      
      auto acc = to_fits_count(example_model);
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

using Answers = std::vector<std::pair<std::string,std::optional<Result>>>;
std::vector<Args> to_requests(Args const& args) {
  std::vector<Args> result{};
  result.push_back(args); // No fancy for now
  return result;
}
int main(int argc, char *argv[]) {
  Args user_args{};
  
  user_args.arg["part"] = "test";
  user_args.arg["file"] = "doc.txt";

  // Override by any user input
  for (int i=1;i<argc;++i) {
    std::string token{argv[i]};
    if (token.starts_with("-")) user_args.options.insert(token);
    else {
      // assume options before <part> and <file>
      auto non_option_index = i - user_args.options.size(); // <part> <file>
      switch (non_option_index) {
        case 1: user_args.arg["part"] = token; break;
        case 2: user_args.arg["file"] = token; break;
        default: std::cerr << NL << "Unknown argument " << std::quoted(token);
      }
    }
  }
  
  auto requests = to_requests(user_args);
  
  if (user_args.options.contains("-all")) {
    requests.clear();
    
    for (int i=0;i<4;++i) {
      Args args{};
      std::string part{};
      std::string file{};
      part = (i/2==0)?"1":"2";
      file = (i%2==0)?"example.txt":"puzzle.txt";
      args.arg["part"] = part;
      args.arg["file"] = file;
      requests.push_back(args);
    }
  }

  Answers answers{};
  std::vector<std::chrono::time_point<std::chrono::system_clock>> exec_times{};
  exec_times.push_back(std::chrono::system_clock::now());
  for (auto request : requests) {
    auto part = request.arg["part"];
    auto file = aoc::to_working_dir_path(request.arg["file"]);
    std::cout << NL << "Using part:" << part << " file:" << file;
    std::ifstream in{file};
    if (in) {
      if (part=="1") {
        answers.push_back({std::format("part{} {}",part,file.filename().string()),part1::solve_for(in,request)});
      }
      else if (part=="2") {
        answers.push_back({std::format("part{} {}",part,file.filename().string()),part2::solve_for(in,request)});
      }
      else if (part=="test") {
        answers.push_back({std::format("{} {}",part,file.filename().string()),test::solve_for(in,request)});
      }
    }
    else std::cerr << "\nSORRY, no file " << file;
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

   Xcode Debug -O2

   >day_22 -all

   ANSWERS
   ...
   
   */
  return 0;
}
