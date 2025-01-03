#include "aoc.hpp"

#include <cctype>
#include <iostream>
#include <iomanip> // E.g., std::quoted
#include <string>
#include <string_view>
#include <sstream> // E.g., std::istringstream, std::ostringstream
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
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
#include <tuple>
#include <functional> // E.g., hash for std::tuple
#include <format>
#include <print>

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

std::vector<std::vector<std::string>> generate_combinations(const std::vector<std::vector<std::string>>& segment_options) {
  std::vector<std::vector<std::string>> result{};
//    for (auto const& options : segment_options) {
//      print("\noptions:{}",options);
//    }
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
//        std::print("\ncombination:{}",current_combination);
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


using aoc::grid::Grid;
using aoc::grid::Position;

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

  // Return all options to press the remote to move the robot from key 'first' to key 'second' and press it
  // E.g., 'A' -> '2' on numering keypad returns options '<^A' and '^<A'.
  std::vector<std::string> to_remote_press_options(Grid const& grid,char first,char second) {
//    std::cout << NL << first << second << std::flush;
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
//      std::cout << NL << pos << " " << path;
//      if (++lc > 1000) break;
      if (pos == end) {
        result.push_back(path+'A');
//        std::cout << " ! ";
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

  // Returns all the options to press the remote to have the robot press all keyes in 'keyes'
  std::vector<std::string> to_remote_press_options(Grid const& grid,std::string const& keyes) {
    std::cout << NL << keyes << std::flush;
    std::vector<std::string> result;
    std::vector<std::vector<std::string>> segment_options{};
    for (int i=-1;i<static_cast<int>(keyes.size()-1);++i) {
      segment_options.push_back({});
      auto first = (i<0)?'A':keyes[i];
      auto second = keyes[i+1];
//      std::print("\nfirst:{},second:{}",first,second);
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
//      std::print("\n{}:{} -> {}",ix,keyes_option,press_options);
      std::copy(press_options.begin(), press_options.end(), std::back_insert_iterator(all));
    }
    auto best = std::accumulate(all.begin(), all.end(), std::numeric_limits<std::size_t>::max(),[](auto acc,std::string const& path){
      acc = std::min(acc,path.size());
      return acc;
    });
    std::copy_if(all.begin(), all.end(), std::back_insert_iterator(result), [best](std::string const& path){
      return (path.size()==best);
    });
//    std::print("\n{} : {}",best,result);
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

  int to_num_part(std::string const& code) {
    std::regex pattern(R"((\d+))");
    std::smatch match;
    int result{-1};
    if (std::regex_search(code.begin(), code.end(), match, pattern)) {
      result = std::stoi(match[0]);
    }
    return result;
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
        Integer acc{};
        for (LogEntry const& entry : log) {
          auto code = entry.code;
          auto to_press_1 = to_remote_press_options(keypad,code);
          auto to_press_2 = to_remote_press_options(remote, to_press_1);
          auto to_press_3 = to_remote_press_options(remote, to_press_2);
          std::print("\ncomputed:{}",to_press_3);
          std::print("\nexpect:{}",entry.expected_presses);
//          std::cout << NL << T << "expect:" << T << entry.expected_presses;
          auto iter = std::find(to_press_3.begin(), to_press_3.end(), entry.expected_presses);
          all_did_pass = all_did_pass and (iter != to_press_3.end());
          if (not all_did_pass) break;
          acc += to_num_part(code) * to_press_3.back().size();
        }
        if (not all_did_pass) {
          return "FAILED";
        }
        if (acc == 126384) return "Complexity 126384 OK";
        else return "FAILED";
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
        auto to_press_1 = test::to_remote_press_options(keypad,code);
        auto to_press_2 = test::to_remote_press_options(remote, to_press_1);
        auto to_press_3 = test::to_remote_press_options(remote, to_press_2);
//        std::print("\ncomputed:{}",to_press_3);
        acc += test::to_num_part(code) * to_press_3.back().size();
      }
      if (acc>0) result = std::to_string(acc);
    }
    return result; // 185828 too high
  }
}

namespace part2 {

  using Step = std::pair<char,char>;
  using Path = std::string; // keys to visit / press
  using PathOptions = std::vector<Path>;
  using MoveOptions = PathOptions;
  using PressOptions = PathOptions;
  using MoveOptionsMap = std::map<Step,MoveOptions>;
  using RemotePressOptions = std::vector<PressOptions>;

  MoveOptions to_move_options(Grid const& grid,Position const& start,Position const& end) {
    MoveOptions result{};
    using Path = std::vector<Position>;
    std::queue<Path> q{};
    q.push({start});
    auto manhattan_distance = aoc::grid::to_manhattan_distance(end-start);
//    if (manhattan_distance==1) std::cout << NL << "to_move_options " << start << " " << end;
    while (not q.empty()) {
      auto curr = q.front();

      q.pop();
//      if (manhattan_distance==1) std::cout << NL << T << curr << " " << *grid.at(curr.back());

      if (curr.back() == end) {
        result.push_back(aoc::grid::to_dir_steps(curr));
//        if (manhattan_distance==1) std::cout << curr << " !";
        continue; // Skip longer paths
      }
      for (auto const& neighbour : aoc::grid::to_ortho_neighbours(curr.back())) {
        if (not grid.on_map(neighbour)) continue;
        if (auto iter = std::ranges::find(curr,neighbour);iter!=curr.end()) continue; // No back-track
        if (*grid.at(neighbour) == ' ') continue;
        using aoc::raw::operator+;
        auto next = curr + neighbour;
        if (next.size()>manhattan_distance+1) continue;
        q.push(next);
      }
    }
    return result;
  }

  MoveOptionsMap to_move_options_map(Grid const& grid) {
    MoveOptionsMap result{};
    std::map<Position,char> key_map{};
    auto capture_key = [&key_map](Position const& pos,char ch) {
      if (ch != ' ') key_map[pos] = ch;
    };
    grid.for_each(capture_key);
    for (int i=0;i<key_map.size();++i) {
      for (int j=0;j<key_map.size();++j) {
        using aoc::raw::operator+;
        auto const& [first_pos,first] = *std::next(key_map.begin(),i);
        auto const& [second_pos,second] = *std::next(key_map.begin(),j);
        Step step{first,second};
        result[step] = to_move_options(grid, first_pos, second_pos);
        std::print("\nstep:{} options:{}",step,result[step]);
      }
    }
    return result;
  }

  
  // steps of keys to press keyes starting from 'A'
  aoc::generatator<Step> to_pad_steps(std::string const& keyes) {
    for (int i=0;i<keyes.size();++i) {
      if (i==0) co_yield Step{'A',keyes[0]};
      else co_yield Step{keyes[i-1],keyes[i]};
    }
  }

  // Options to press the remote to have robot press keyes_to_press
  RemotePressOptions to_remote_press_options(std::string const& keyes_to_press,MoveOptionsMap const& move_options) {
    RemotePressOptions result{};
    for (auto const& step : to_pad_steps(keyes_to_press)) {
      result.push_back({});
      using aoc::raw::operator+;
      for (auto keyes : move_options.at(step)) result.back().push_back(keyes + 'A');
    }
    return result;
  }

  using Seen = std::map<std::pair<int,std::string>,Integer>;

  // The number of ways to press the remote to have robot enter all press_options on pad
  Integer to_shortest_possible_sequences_length(RemotePressOptions const& press_options,MoveOptionsMap const& move_options,int robot_stack_height,Seen& seen) {
    static int loop_count{};
    if (loop_count++ % 10000 == 0) std::print("\n{} {} {}",loop_count,robot_stack_height,press_options);
//    auto indent = "\n" + std::string(2*robot_stack_height,' ');
//    std::print("{}{} : to_shortest_possible_sequences_length : {}",indent,robot_stack_height,press_options);
    Integer result{};
    // sequence of options
    auto best_length = std::numeric_limits<Integer>::max();
    for (auto const& round_trip_options : press_options) {
//      std::print("{}{}",indent,round_trip_options);
      // each actual option = keyes to press
      Integer remote_sequence_length{};
      for (auto const& keyes : round_trip_options) {
        if (robot_stack_height>0) {
          // ["<A"]
          // Expand to options to press on remote
          auto state = std::pair{robot_stack_height,keyes};
          if (seen.contains(state)) {
            remote_sequence_length += seen[state];
          }
          else {
            auto press_options = to_remote_press_options(keyes,move_options);
  //          std::print("{}On remote option:{}",indent,press_options);
            // Flatten to actual options
            auto combinations = generate_combinations(press_options);
  //          std::print("{}combinations:{}",indent,combinations);
            auto candidate = to_shortest_possible_sequences_length(combinations, move_options, robot_stack_height-1,seen);
            remote_sequence_length += candidate;
            seen[{robot_stack_height,keyes}] = candidate;
          }
        }
        else {
          remote_sequence_length += keyes.size();
        }
      }
      best_length = std::min(best_length,remote_sequence_length);
    }
    result = best_length; // best total length to press on remote
//    std::print("{}result:{}",indent,result);
    return result;
  }

  Integer to_shortest_possible_sequences_length(std::string const& code,MoveOptionsMap const& move_options,int robot_stack_height) {
    std::cout << NL << aoc::raw::Indent(robot_stack_height*2);
    std::print("\n{} : to_shortest_possible_sequences_length : {}",robot_stack_height,code);
    // expand to what to press on the remote of robot that presses numeric keypad
    RemotePressOptions press_options{};
    press_options.push_back({});
    press_options.back().push_back(code);
    Seen seen{};
    return to_shortest_possible_sequences_length(press_options,move_options, robot_stack_height,seen);
  }


  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part2";
    if (in) {
      auto model = parse(in);
      /*
       
       N+1 layers of keypads numbvered 0..N
       Pad N is the final numerical keypad where the code is to be entered
       Each pad is operated by an 'actuator' (A robot or you)
       
       level
       N     : numeric keypad (where code is to be entered)    Robot N ----> numeric
       N-1   : Remote control pad to control robot N              \---<----- remote N  <----- Robot N-1
       ...   : ...                                             Robot N-2 --> remote N-1 -->------/
       ...   : Robot actuated remotes                             \---<----- remote N-2 <---- Robot N-3
       ...   : ...                                             Robot N-4 --> remote N-3 -->------/
       ...   : ...                                                            ...          ...
       ...   : ...                                               ...          ...
       ...   : ...                                                \---<----- remote 3 <---- Robot 2
       ...   : ...                                             Robot 1 ----> remote 2 ---->------/
       0     : Remote control pad to control Robot 1              \---<----- remote 1 <---- YOU
       
       You enters keys on remote 1 to control robot 1 (out of 2 or 25 robots)
       
       Question is: How many optimal (!) ways are there for you to operate the first robot so that the
                    required code is entered by robot N on the numeric keypad? That is,
                    there are of course many suboptimal ways to operate the remote to have robot N
                    enter the code. But, we want to count only the members in the set of
                    the shortest ways of accomplishing this!
       
       For robot N   : there is only one way to enter the code on the numeric keypad.
       For robot N-1 : there are x0 * x1 * x2 * x4 ways to have robot N enter the code.
                       Where xn is the number of options to move to the n:th key to press.
       
                       Each way to control robot N to press the required keyes,come from
                       all combinations of the options to move between the keyes to press.
       
       So, options to move between keyes on pad  --> options of keyes to press on the remote.
       And, pressing a key on the pad corresponds to pressing 'A' on the remote.
       
       So we have a transformation: keyes to press on the pad --> keyes to press on the remote.
       Where moves between keyes on the pad --> presses of keyes on the remote.
       And then pressing a key on the pad --> pressing key 'A' on the remote.
       Thus, to get what keyes to press on the remote we have to:
       
          * Generate all options to move between the keyes to press on the pad.
            (And these can be pre-calculated for all keyes on the pad(s))
          * transform each such path on the pad to key presses on the remote.
          * Add the key 'A' to press on the remote for each path to a new key on the pad.
      
       For pad N-1 (the remote of robot N) down to pad 1 (the remote pressed by YOU) the
       transformation is between keyes to press on remote N-1 to keyes to press on remote N-2.
       
       The transformation is between levels.
       
       Transformation N: Transform the code to press to the keyes to press on the remote of robot N.
                         We get a complete duplication of the sest of options for each new option
                         to move between each key.
       
       Transformation 1 < i < N : Both the pad to press and the remote are the remote key pad.
                                  The keyes to press on the actuated pad comes from the
                                  keyes to press on the remote generated by the previous transformation.
                                  
       
            * Add a first step : 'A' -> first key to press.
            * For each key-step first to second,
              assemble all the options to press keyes on the remote to have the robot
              travel between keyes first to second.
            * The options to move between each pair of keyes are independent so store them in a set.
            * The order of steps to take between keyes must be retained so store them
              in a list of 'press-to-press' options (move options? path options?)
       
       So when can we prune the otions to the 'best' ones? We want the set of shortest sequence of keyes
       to press on the remote!
       
            * The keyes to press on the remote must assume the controlled robot starts at 'A'.
            * And after having pressed the keyes on the remote to move the robot to the key to press,
              it must then press 'A' on the remote.
            * Thus, the keyes to press on the remote always starts and ends on the 'A'-key.
            * This makes each round-trip in the remote to have the robot åpress a key on the pad
              independent of the other round-trips for the other keyes to press.
            * This should mean we can find the shortest sequence of keyes to press on the remote
              by finding the shortest round-trips for each key tgo press on the pad?
       
       Assume: Best sequence to press on the remote = a combination of the best round-trips
               on the remote for each key to press on the pad.
            
       N: --------------------------------------------------------------------------
       
         numeric keypad (pad N)
         +---+---+---+
         | 7 | 8 | 9 |
         +---+---+---+
         | 4 | 5 | 6 |
         +---+---+---+
         | 1 | 2 | 3 |
         +---+---+---+
             | 0 | A <--- Actuator N (Robot N)
             +---+---+
       
       Actuator N are to press 'keyes' on pad N
       
       Press         0 2 9 A    (pad N)

       Observation: The remote controlled robot starts at 'A'
       
       Visit       A 0 2 9 A    (pad N)
                   | | | | |
                   ---------
                    | | | |   steps A->0, 0->2, 2->9,9->A
                    < | | |
                    | | | |
                    | ^ | |   ...
                    | | | |   options to move between each 'key'
                    | | | |   ...
                    | |^^>|
                    or ^>^|
                    or >^^|
                    | | | |
                    | | | |
                    | | |vvv
                    | | | |
                    A A A A   and then press command 'A' to press the 'key'
      
       
       FUNCTION: path to visit -> steps to take
       FUNCTION: step to take  -> moves to make
       
       Observation: We can represent the options to move between keyes
       as a list with members that are options to move between each key
       
        A->0, 0->2,         2->9      ,9->A    step
          |     |            |           |
         1x     1x          3x           1x    move options count
       [  <     ,^   ,^^> | ^>^| >^^ ,  vvv]   moves list
       |  A      A                 A      A]   press command
       ------------------------

       Observation: The moves AND keypress are accomplished by pressing keyes on the remote.

       Remote to operate robot N (pad N-1 == pad to press on by robot N-1)
       
           +---+---+
           | ^ | A |
       +---+---+---+
       | < | v | > |
       +---+---+---+
       
       FUNCTION: Moves to make (robot N) -> keys to press on pad N-1

       We can transform each list of move sequence options
       into a list of move sequences
       
         [<,^,^^>,vvv]     this
       | [<,^,^>^,vvv]  or this
       | [<,^,>^^,vvv]  or this
       & [A A   A   A] and this
       ---------------

         [<A,^A,^^>A,vvvA]    this 1  --> press "<A^A^^>AvvvA"
       | [<A,^A,^>^A,vvvA] or this 2  --> press "<A^A^>^AvvvA"
       | [<A,^A,>^^A,vvvA] or this 3  --> press "<A^A>^^AvvvA"
       -----------------------------
       count                       3
       
       We can represent the
       to press on remote (options separated by or '|')

       {[<A,^A,^^>A,vvvA] | [<A,^A,^>^A,vvvA] | [<A,^A,>^^A,vvvA]}
       
       These are commands to Robot N to press
       
       These are keys to press on Remote to robot N.
       Or the Pad N-1 pressed on by Robot N-1.
       
       {"<A^A^^>AvvvA" | "<A^A^>^AvvvA" | "<A^A>^^AvvvA]"}
          presses 0   or    presses 1  or    presses 2
                                 |
      N-1 -----------------------|--------------------------------------------------
                                 v
                       sequence options to press on 'remote N' = pad N-1
       
       Remote to robot N (pad N-1)
       
           +---+---+
           | ^ | A <-- Robot N-1 (actuator of remote to next robot N)
       +---+---+---+
       | < | v | > |
       +---+---+---+

      We receive the keys to press on the remote to the next robot (pad actuated by this robot)
              
      Sequences         0                                1                   2
      sub sequences     0    1     2         3           0  1  2    3        0  1  2    3
      Presses           0 1  0 1   0 1 2 3   0 1 2 3     01 01 0123 0123     01 01 0123 0123
                     [[ < A, ^ A , ^ ^ > A , v v v A] | [<A,^A,^>^A,vvvA] | [<A,^A,>^^A,vvvA]]
      
      Compute what to press on remote to have this robot N-1 press each sequence
       
      Move commands    0                                      1     2
      start from A   [[A < A, A ^ A , A ^ ^ > A ,A v v v A] | ... | ...]
                       | | |  | | |   | | | | |  ...
      Steps            -----  -----   ---------
                        | |    | |     | | | |
                      <v< |    | |       ...
                      v<< |    | |
                        | |    | |     Move options for each step
                        >>^    | |
                        >^>    | |
                        | |    < |
                        | |    | >
                        | |    | |
      Presses           A A    A A
                        ----------------------
       
                        Keyes to press on this robot remote
                        <v< A, >>^ A, < A, > A
                         "  A, >^> A, < A, > A
                        v<< A, >>^ A, < A, > A
                         "  A, >^> A, < A, > A
       
       Observation: The round-trip on the remote (start on 'A', then key presses that ends with 'A')
                    all have pre-calculated shortest length independent of other round trips.
      
       Idea: Recurse each round-trip on the remote down the stack of robot actuators!
             Beginning with a step to press a key of the code, we can compute all the best round-trips
             on the robots remote to have the controlled robot press that key.
             The number of ways to do this is the size of the set containing the number of ways to
             move as defined by the step.
             This pattern repeats for the keys to press on the next remote given the key steps we are to
             take on this remote.
       
             N  : Press:  "029A"
             N  : Steps: 'A'->'0','0'->'2','2'->'9','9'->'A'
             N  : Best      "<"  ,   "^"  ,  "^^>" ,  "vvv"       known
             N  : Best       |            ,  "^>^"                known
             N  : Best       |            ,  ">^^"                known
             N  : Press     "<A"
                             |
             N-1: Press     "<A"
             N-1: Steps     'A'->'<','<'->'A'
             N-1: Best       "<v<"    ">^>"
             N-1: Best       "v<<"    ">>^"
             N-1:              |        |
             N-1: Press      "<v<A"   ">^>A"
             N-1: Press      "v<<A"   ">>^A"
                             ----------------
                                |
             N-2: Press      [{"<v<A" or "<v<A"} then {">^>A" or ">>^A"}]
             N-2:                |
             N-2: Steps       'A'->'<','<'->'v','v'->'<','<'->'A'
             N-2: Best           ..      ">"       ..       ..
             N-2:                ..       |        ..       ..
             N-2:              -------  -------  -------  -------
             N-2:                set     set       set      set
             N-2:        list [   0   ,   1     ,   2    ,   3    ]

             N-3: Press  list [ set   ,   set   ,   set  ,  set   ]
                                 s0        s1       s2      s3
             Observation: The number of ways to press a combination of these sets in the eorder
                          defined by the list are the product of all the set sizes!

                          s0 x s1 x s2 x s3 ...
       
         set of optons of key-sequences to press on the remote pad i
                                |
                                |
       i -----------------------|--------------------------------------------------
                                v
       In: set of sequences to press on pad i = sequence of sets of round trips to press on pad 1
       
       i: In  list [ set   ,   set   ,   set  ,  set   , ...] to press on pad i

       Remote to robot i+1 (pad i)
       
           +---+---+
           | ^ | A <-- Robot i
       +---+---+---+
       | < | v | > |
       +---+---+---+
       
               
       list [ set   ,   set   ,   set  ,  set   , ...]          list of set of subsequences to combine to press
               |         |         |       |
               |         |         |       --- "seq","seq",...
               |         |         |             |
               |         |         |        step,step,step...
               |         |         |
               |         |         --- "seq","seq",...
               |         |               |
               |         |         step,step,step...
               |         |
               |         --- "seq","seq",...
               |               |
               |         step,step,step...
               |
               --- "seq","seq",...
                     |
              step,step,step...

       * For each set in list of options of round-trip presses
           * For each sequence in set of round-trips to press
             * add start-step 'A' to first key.
             * for_each step in sequense (starting from 'A')
                 * Look up all options to move between 'first' and 'second' in 'step'
                 * Generate all 'round-trip' 'A' to 'A' on remote i for robot i to press 'second'
                 * Store in set of all round-trips for the step
             * Prune (filter) each set of round-trip-options for each step to keep only shortest ones
               (But if we only use pre-calculated best sequences for each step, then assembling these
                can only get us 'best' round-trips anyhow?)
           
           
       Remote to robot i (pad i-1)

           +---+---+
           | ^ | A <-- Robot i-1
       +---+---+---+
       | < | v | > |
       +---+---+---+

       i: OUT  list [ set   ,   set   ,   set  ,  set   , ...] to press on remote i (pad i-1)
                                  |
                                  |
       ... -----------------------|--------------------------------------------------
                                  v
       In  list [ set   ,   set   ,   set  ,  set   , ...] to press on pad i
                                  |
                      transform to press on remote
                                  |
                                  v
       OUT  list [ set   ,   set   ,   set  ,  set   , ...] to press on remote i (pad i-1)

                                |
                                |
       0 -----------------------|--------------------------------------------------
                                v

       In  list [ set   ,   set   ,   set  ,  set   , ...] to press on pad 0
                   c0        c1        c2      c3     c..
       
       Remote to robot 1 (pad 0)
       
           +---+---+
           | ^ | A |    <-- YOU : Total number of ways to press = product of the sizes of all 'sets'
       +---+---+---+
       | < | v | > |
       +---+---+---+
       
       Hm... We need a way to propagate the 'count' down the stack of robots...

       Suppose we propagate the combination count for each set of round-trip options 'so faar'?
       
                  count     count    count   count   ...
       In  list [ set   ,   set   ,  set  ,  set   , ...] to press on pad 0

                  each step --> in_count * (number of options for step) = out count?

                  A029A   -> one way
       
                   1     1     1     1
         N:       A->0, 0->2, 2->9, 9->A
                  ----------------------
                              "^^>"
                              "^>^"
                              ">^^"
                   1x    1x    3x    1x
                  ----------------------
                   1     1     3     1
                               |
         N-1:                  |
                        --------------
                         3x   3x   3x
                        step step step
                        set  set  set
                        ---  ---  ---
                         |    |    |
                        The count for each of these OUT sets out are 3x (combination increase by step)
                        And the combination increase is the number of round-trips in the set for the step
       
                        
       
                 
       

       OUT  list [ set   ,   set   ,  set  ,  set   , ...] to press on remote i (pad i-1)
                   count     count    count   count   ...

       
       */
      Integer acc{};
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
      MoveOptionsMap move_options{};
      for (auto const& [step,moves] : to_move_options_map(keypad)) move_options[step] = moves;
      for (auto const& [step,moves] : to_move_options_map(remote)) move_options[step] = moves;
      
      int const ROBOT_STACK_HEIGHT{26};
      
      if (false) {
        // test single digit to see how it exapnds
        return std::to_string(to_shortest_possible_sequences_length("029A",move_options,3));
      }
      
      for (auto const& code : model) {
        auto len = to_shortest_possible_sequences_length(code,move_options,ROBOT_STACK_HEIGHT);
        auto num = std::stoi(code.substr(0,3));
        auto complexity = num*len;
        std::print("\n{} -> {}*{}={}",code,len,num,complexity);
        acc += complexity;
      }
      if (acc>0) result = std::to_string(acc);

    }
    return result; // 2381545 too low
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

   >day_21 -all

   ANSWERS
   duration:0ms answer[test example.txt] Sorry, Unknown 'part' "test"
   duration:0ms answer[test puzzle.txt] Sorry, Unknown 'part' "test"
   duration:6077ms answer[part1 example.txt] 126384
   duration:82156ms answer[part1 puzzle.txt] 179444
   duration:0ms answer[part2 example.txt] NO OPERATION
   duration:0ms answer[part2 puzzle.txt] NO OPERATION
   
   */
  return 0;
}
