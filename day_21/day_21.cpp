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
using Model = aoc::raw::Lines;

Model parse(auto& in) {
  using namespace aoc::parsing;
  Model result{};
  auto lines = Splitter{in}.lines();
  for (auto const& [lx,line] : aoc::views::enumerate(lines)) {
    std::cout << NL << T << T << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
    result.push_back(line);
  }
  return result;
}

using aoc::Args;

namespace test {

  // Adapt to expected for day puzzle
  struct LogEntry {
    aoc::raw::Line code;
    aoc::raw::Line expected_presses;
    bool operator==(LogEntry const& other) const {
      bool result{true};
      return result;
    }
  };

  std::ostream& operator<<(std::ostream& os,LogEntry const& entry) {
    std::cout << entry.code << " -- ? --> " << entry.expected_presses;
    return os;
  }

  using LogEntries = aoc::test::LogEntries<LogEntry>;

  LogEntries parse(auto& doc_in) {
    std::cout << NL << T << "test::parse";
    LogEntries result{};
    using namespace aoc::parsing;
    auto sections = Splitter{doc_in}.same_indent_sections();
    int target_sx{-1};
    for (auto const& [sx,section] : aoc::views::enumerate(sections)) {
      std::cout << NL << "---------- section " << sx << " ----------";
      for (auto const& [lx,line] : aoc::views::enumerate(section)) {
        std::cout << NL << T << T << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
        if (line.str().find("here is a shortest sequence of button presses") != std::string::npos) {
          target_sx = static_cast<int>(sx)+1;
        }
        if (sx == target_sx) {
          auto const& [left,right] = line.split(':');
          LogEntry entry{left,right.trim()};
          result.push_back(entry);
        }
      }
    }
    return result;
  }

  using aoc::grid::Grid;
  using aoc::grid::Position;

  // Return all options to press the remote to move the robot from key 'first' to key 'second' and press it
  // E.g., 'A' -> '2' on numering keypad returns options '<^A' and '^<A'.
  std::vector<std::string> to_remote_press_options(Grid const& grid,char first,char second) {
    std::vector<std::string> result{};
    auto start = grid.find_all(first)[0];
    auto end = grid.find_all(second)[0];
    using State = std::tuple<Position,std::string>; // path candidate to position
    std::deque<State> q{};
    q.push_back({start,""});
    std::set<State> seen{};
    int lc{};
    int const OPTIMAL_PATH_LENGTH{aoc::grid::to_manhattan_distance(start, end)};
    while (not q.empty()) {
      auto curr = q.front();
      q.pop_front();
      auto [pos,path] = curr;
      std::cout << NL << pos << " " << path;
//      if (++lc > 1000) break;
      if (pos == end) {
        result.push_back(path+'A');
        std::cout << " ! ";
        continue;
      }
      if (path.size()>OPTIMAL_PATH_LENGTH) break;
      auto [r,c] = pos;
      for (auto next : std::vector<std::pair<Position,char>>{{{r+1,c},'v'},{{r,c+1},'>'},{{r-1,c},'^'},{{r,c-1},'<'}}) {
        if (not grid.on_map(next.first)) continue;
        if (grid.at(next.first) == ' ') continue;
        if (next.first == pos) continue;
        using aoc::raw::operator+;
        State state{next.first,path+next.second};
        if (seen.contains(state)) continue;
        q.push_back(state); // bfs
        seen.insert(state);
      }
    }
    return result;
  }

  std::vector<std::vector<std::string>> generate_combinations(const std::vector<std::vector<std::string>>& segment_options) {
    std::vector<std::vector<std::string>> result{};
    for (auto const& options : segment_options) {
      print("\noptions:{}",options);
    }
    // Initialize a queue that holds partial combinations
    std::queue<std::vector<std::string>> q;
    
    // Start with an empty combination
    q.push({});
    
    // Process the queue until all combinations are generated
    while (!q.empty()) {
      // Get the current partial combination
      std::vector<std::string> current_combination = q.front();
      q.pop();
      
      // If the current combination is complete, print it
      if (current_combination.size() == segment_options.size()) {
        result.push_back(current_combination);
        std::print("\ncombination:{}",current_combination);
//        std::cout << NL << "combination:";
//        for (const auto& [ix,element] : aoc::views::enumerate(current_combination)) {
//          if (ix>0) std::cout << ',';
//          std::cout << element;
//        }
      } else {
        // Otherwise, add all possibilities for the next vector to the current combination
        size_t depth = current_combination.size(); // which vector are we adding from?
        for (const auto& option : segment_options[depth]) {
          // Copy the current combination, append the new option, and push it to the queue
          std::vector<std::string> new_combination = current_combination;
          new_combination.push_back(option);
          q.push(new_combination);
        }
      }
    }
    return result;
  }
  // Returns all the options to press the remote to have the robot press all keyes in 'keyes'
  std::vector<std::string> to_remote_press_options(Grid const& grid,std::string const& keyes) {
    std::vector<std::string> result;
    std::vector<std::vector<std::string>> segment_options{};
    for (int i=-1;i<static_cast<int>(keyes.size()-1);++i) {
      segment_options.push_back({});
      auto first = (i<0)?'A':keyes[i];
      auto second = keyes[i+1];
      std::print("\nfirst:{},second:{}",first,second);
      auto options = to_remote_press_options(grid, first, second);
      using aoc::raw::operator+;
      segment_options.back() = options; // options for moving from first to second and press it
    }
    // Now all possible ways are to pick all combinations of optional segments to press each key
    // This is the cartesian product of all segment options
    auto combinations = generate_combinations(segment_options);
    for (auto const& combination : combinations) {
      auto candidate = std::accumulate(combination.begin(), combination.end(), std::string{});
      result.push_back(candidate);
    }
    return result;
  }

  std::vector<std::string> to_remote_press_options(Grid const& grid,std::vector<std::string> const& keyes_options) {
    std::vector<std::string> result{};
    
    std::vector<std::string> all{};
    for (auto const& [ix,keyes_option] : aoc::views::enumerate(keyes_options)) {
      auto press_options = to_remote_press_options(grid, keyes_option); // all options to move on grid to press keyes
      std::print("\n{}:{} -> {}",ix,keyes_option,press_options);
      std::copy(press_options.begin(), press_options.end(), std::back_insert_iterator(all));
    }
    auto best = std::accumulate(all.begin(), all.end(), std::numeric_limits<std::size_t>::max(),[](auto acc,std::string const& path){
      acc = std::min(acc,path.size());
      return acc;
    });
    std::copy_if(all.begin(), all.end(), std::back_insert_iterator(result), [best](std::string const& path){
      return (path.size()==best);
    });
    std::print("\n{} : {}",best,result);
    return result;
  }

  std::optional<Result> test0(Args args) {
    std::ostringstream oss{};
    std::cout << NL << NL << "test0";
    //    For example, to make the robot type 029A on the numeric keypad, one sequence of inputs on the directional keypad you could use is:
    //
    //    < to move the arm from A (its initial position) to 0.
    //    A to push the 0 button.
    //    ^A to move the arm to the 2 button and push it.
    //    >^^A to move the arm to the 9 button and push it.
    //    vvvA to move the arm to the A button and push it.
    aoc::grid::Grid keypad({
       "789"
      ,"456"
      ,"123"
      ," 0A"
    });
    
    auto computed = to_remote_press_options(keypad,"029A");
    std::set<std::string> expected = {"<A^A>^^AvvvA", "<A^A^>^AvvvA", "<A^A^^>AvvvA"}; // CTAD :)
    std::print("\ncomputed:{} expected:{}",computed,expected);
    if (computed.size() == expected.size()) {
      if (std::all_of(computed.begin(), computed.end(), [&expected](std::string const& option) {
        if (not expected.contains(option)) {
          std::print(R"(Generated option "{}" not in expected {})",option,expected);
          return false;
        }
        return true;
      })) {
        std::print(oss,"OK");
      }
      else {
        std::print(oss,"FAILED");
      }
    }
    else {
      if (computed.size() < expected.size()) {
        std::print(oss,"Computed too few");
      }
      else {
        std::print(oss,"Computed too many");
      }
    }

    if (oss.str().size()>0) return oss.str();
    return std::nullopt;
  }

  std::optional<Result> test1(auto& in, Args args) {
    std::optional<Result> result{};
    aoc::raw::Lines answers{};
    std::cout << NL << NL << "test1";
    if (in) {
      auto model = ::parse(in);
      std::ifstream doc_in{aoc::to_working_dir_path("doc.txt")};
      if (doc_in) {
        auto log = test::parse(doc_in);
        using aoc::test::operator<<;
        std::cout << NL << log;
        aoc::grid::Grid keypad({
           "789"
          ,"456"
          ,"123"
          ," 0A"
        });
        aoc::grid::Grid remote({
           " ^A"
          ,"<v>"
        });
        
        // Shoot! We need to examine all possible moves from robot to see what path the next robot can take
        // that is the shortest one!
        bool all_did_pass{true};
        for (LogEntry const& entry : log) {
          auto to_press_1 = to_remote_press_options(keypad,entry.code);
          auto to_press_2 = to_remote_press_options(remote, to_press_1);
          auto to_press_3 = to_remote_press_options(remote, to_press_2);
          std::print("\ncomputed:{}",to_press_3);
          std::cout << NL << T << "expect:" << T << entry.expected_presses;
          auto iter = std::find(to_press_3.begin(), to_press_3.end(), entry.expected_presses);
          if (iter == to_press_3.end()) {
            all_did_pass = false;
          }
          if (not all_did_pass) {
            return "FAILED";
          }
        }
        return "OK";
      }
    }
    return result;
  }

  std::optional<Result> solve_for(std::istream& in,Args args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "test";
    if (args.arg["part"] == "test0") return test0(args);
    else if (args.arg["part"] == "test1") return test1(in,args);
    else {
      return std::format("Sorry, Unknown 'part' \"{}\"",args.arg["part"] );
    }
    return std::nullopt;
  }
}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      aoc::grid::Grid keypad({
         "789"
        ,"456"
        ,"123"
        ," 0A"
      });
      aoc::grid::Grid remote({
         " ^A"
        ,"<v>"
      });

      Integer acc{};
      for (auto const& code : model) {
      }
      if (acc>0) result = std::to_string(acc);
    }
    return result; // 185828 too high
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
  
  // Override by any user input
  for (int i=1;i<argc;++i) {
    user_args.arg["file"] = "example.txt";
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
  
  if (not user_args or user_args.options.contains("-all")) {
    requests.clear();

    std::vector<std::string> parts = {"test", "1", "2"};
    std::vector<std::string> files = {"example.txt", "puzzle.txt"};
    
    for (const auto& [part, file] : aoc::algo::cartesian_product(parts, files)) {
      Args args;
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
      else if (part.starts_with("test")) {
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
