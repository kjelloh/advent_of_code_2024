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

// Stack based bfs cartesian product
std::vector<std::vector<std::string>> generate_combinations(const std::vector<std::vector<std::string>>& segment_options) {
  std::vector<std::vector<std::string>> result{};
  std::queue<std::vector<std::string>> q;
  q.push({});
  while (!q.empty()) {
    std::vector<std::string> current_combination = q.front();
    q.pop();
    if (current_combination.size() == segment_options.size()) {
      result.push_back(current_combination);
    } else {
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

  std::vector<aoc::raw::Lines> to_examples(aoc::parsing::Sections const& sections) {
    std::vector<aoc::raw::Lines> result{};
    //---------- section 33 ----------
    //    line[0]:4 "029A"
    //    line[1]:4 "980A"
    //    line[2]:4 "179A"
    //    line[3]:4 "456A"
    //    line[4]:4 "379A"
    result.push_back({});
    result.back().append_range(aoc::parsing::to_raw(sections[33]));
    return result;
  }

  // Adapted to expected for day puzzle
  struct Expected {
    aoc::raw::Line code;
    aoc::raw::Line expected_presses;
    bool operator==(Expected const& other) const {
      bool result{true};
      return result;
    }
  };

  std::ostream& operator<<(std::ostream& os,Expected const& entry) {
    std::cout << entry.code << " -- ? --> " << entry.expected_presses;
    return os;
  }

  using Expecteds = aoc::test::Expecteds<Expected>;


  Expecteds to_expecteds(aoc::parsing::Sections const& sections) {
    Expecteds result{};
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
          Expected entry{left,right.trim()};
          result.push_back(entry);
        }
      }
    }
    return result;
  }

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
      if (pos == end) {
        result.push_back(path+'A');
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
      std::copy(press_options.begin(), press_options.end(), std::back_insert_iterator(all));
    }
    auto best = std::accumulate(all.begin(), all.end(), std::numeric_limits<std::size_t>::max(),[](auto acc,std::string const& path){
      acc = std::min(acc,path.size());
      return acc;
    });
    std::copy_if(all.begin(), all.end(), std::back_insert_iterator(result), [best](std::string const& path){
      return (path.size()==best);
    });
    return result;
  }

  bool test0(std::optional<aoc::parsing::Sections> const& opt_sections,Args const& args) {
    bool result{true};
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
    
    std::string code{"029A"};
    auto computed = to_remote_press_options(keypad,code);
    std::set<std::string> expected = {"<A^A>^^AvvvA", "<A^A^>^AvvvA", "<A^A^^>AvvvA"};
    std::print("\ncomputed:{} expected:{}",computed,expected);
    if (computed.size() == expected.size()) {
      if (std::all_of(computed.begin(), computed.end(), [&expected](std::string const& option) {
        if (not expected.contains(option)) {
          std::print(R"(Generated option "{}" not in expected {})",option,expected);
          return false;
        }
        return true;
      })) {
        std::print("{} -> {} (file ignored) OK",code,expected);
      }
      else {
        std::print("FAILED");
        result = false;
      }
    }
    else {
      if (computed.size() < expected.size()) {
        std::print("Computed too few");
      }
      else {
        std::print("Computed too many");
      }
    }
    return result;
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

  bool test1(std::optional<aoc::parsing::Sections> const& opt_sections,Args const& args) {
    if (opt_sections) {
      auto const& sections = *opt_sections;
      auto expecteds = to_expecteds(sections);
      using aoc::test::operator<<;
      std::cout << NL << expecteds;
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
      Integer acc{};
      bool result{true};
      for (Expected const& entry : expecteds) {
        auto code = entry.code;
        auto to_press_1 = to_remote_press_options(keypad,code);
        auto to_press_2 = to_remote_press_options(remote, to_press_1);
        auto to_press_3 = to_remote_press_options(remote, to_press_2);
        std::print("\ncomputed:{}",to_press_3);
        std::print("\nexpect:{}",entry.expected_presses);
        auto iter = std::find(to_press_3.begin(), to_press_3.end(), entry.expected_presses);
        result = result and (iter != to_press_3.end());
        if (not result) break;
        acc += to_num_part(code) * to_press_3.back().size();
      }
      if (result = result and (acc == 126384);result) {
        std::print("\nFive code example -> Complexity 126384 OK");
      }
      if (not result) std::print("\nFAILED");
      return result;
    }
    else std::print("\nNo sections (did you pull_text.zsh and does doc.txt exist with day web page text?)");
    return false;
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

  // flood fill from start and exhaust all hits on end
  MoveOptions to_move_options(Grid const& grid,Position const& start,Position const& end) {
    MoveOptions result{};
    using Path = std::vector<Position>;
    std::queue<Path> q{};
    q.push({start});
    auto manhattan_distance = aoc::grid::to_manhattan_distance(end-start);
    while (not q.empty()) {
      auto curr = q.front();
      q.pop();
      if (curr.back() == end) {
        result.push_back(aoc::grid::to_dir_steps(curr));
        continue; // Skip longer paths
      }
      for (auto const& neighbour : aoc::grid::to_ortho_neighbours(curr.back())) {
        if (not grid.on_map(neighbour)) continue;
        if (auto iter = std::ranges::find(curr,neighbour);iter!=curr.end()) continue; // No back-track
        if (grid.at(neighbour) == ' ') continue;
        using aoc::raw::operator+;
        auto next = curr + neighbour;
        if (next.size()>manhattan_distance+1) continue; // filter out 'detoures' > manhattan distance steps
        q.push(next);
      }
    }
    return result;
  }

  // All options to move optimally between any key pairs on grid (pad)
  MoveOptionsMap to_move_options_map(Grid const& pad) {
    MoveOptionsMap result{};
    std::map<Position,char> key_map{};
    auto capture_key = [&key_map](Position const& pos,char ch) {
      if (ch != ' ') key_map[pos] = ch;
    };
    pad.for_each(capture_key);
    for (int i=0;i<key_map.size();++i) {
      for (int j=0;j<key_map.size();++j) {
        using aoc::raw::operator+;
        auto const& [first_pos,first] = *std::next(key_map.begin(),i);
        auto const& [second_pos,second] = *std::next(key_map.begin(),j);
        Step step{first,second};
        result[step] = to_move_options(pad, first_pos, second_pos);
        std::print("\nstep:{} options:{}",step,result[step]); std::cout << std::flush;
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
  // pad_ix is the index to all robot remotes 0..N-1 + the numeric pad N
  Integer to_shortest_possible_sequences_length(RemotePressOptions const& press_options,MoveOptionsMap const& move_options,int pad_ix,Seen& seen) {
    Integer result{};
    // sequence of options
    auto best_length = std::numeric_limits<Integer>::max();
    for (auto const& round_trip_options : press_options) {
      Integer remote_sequence_length{};
      for (auto const& keyes : round_trip_options) {
        if (pad_ix>0) {
          // Expand to options to press on remote
          auto state = std::pair{pad_ix,keyes};
          if (seen.contains(state)) {
            remote_sequence_length += seen[state];
          }
          else {
            auto press_options = to_remote_press_options(keyes,move_options);
            // 'Flatten' to spelled out options (cartesian product)
            auto combinations = generate_combinations(press_options);
            auto candidate = to_shortest_possible_sequences_length(combinations, move_options, pad_ix-1,seen);
            remote_sequence_length += candidate;
            seen[state] = candidate;
          }
        }
        else {
          // Remote of robot 1 (pad_ix 0) is pressed by YOU
          remote_sequence_length += keyes.size();
        }
      }
      best_length = std::min(best_length,remote_sequence_length);
    }
    result = best_length; // best total length to press on remote
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
    // Start the serach from pad N = robot_stack_height + 1 (the numeric pad)
    return to_shortest_possible_sequences_length(press_options,move_options, robot_stack_height+1,seen);
  }


  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part2";
    if (in) {
      auto model = parse(in);
      /*
       
       N+1 layers of keypads indexed 0..N
       Pad N is the final numerical keypad where the code is to be entered
       The stacking makes pad N-1 is Remote N (pad N is the numeric keypad)
       Each pad is operated by an 'actuator' (a robot or you)
       
       level                                                                 (pad N)
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
                                                                              (pad 0)
       
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

       
       
       In  list [ set   ,   set   ,  set  ,  set   , ...] to press on pad 0


        Press on Pad:      029A
        Visit on Remote:   A->0->2->9->A
        Press on remote       A  A  A  A
       
         N:       A->0, 0->2, 2->9, 9->A
                  ----------------------
                  ...   ...   "^^>" ...    Move options to move key-to-key on remote
                              "^>^"
                              ">^^"
                   |      |     |     |
                   |      |     |     \----------------\
                   |      |     |                      |
                   |      |     \----------\           |
                   |      |                |           |
                   |      \----\           |           |
                   |           |           |           |
                  +'A'        +'A'        +'A'        +'A'      Add 'A' to each move option
                   |           |           |           |        to have robot press the key we moved to
        list [set<keyes>, set<keyes>, set<keyes>, set<keyes> ]
       
        Observation: We now have several ways to pick an option for each key-to-key move.
                     We have to try out each such combination to move and press keys on the remote
       
        Observation: The shortest sequence to press on the remote are all combinations of the shortest
                     sequences to have robot press each key.
              
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
      
      int ROBOT_STACK_HEIGHT{25};
      if (not args.arg.contains("file")) {
        std::cout << NL << "No File?!";
      }
      if (args.arg.at("file") == "example.txt") ROBOT_STACK_HEIGHT=2;
      
      if (false) {
        // test single digit to see how it expands
        return std::to_string(to_shortest_possible_sequences_length("029A",move_options,2));
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
    return result;
  }
}

int main(int argc, char *argv[]) {
  aoc::application app{};
  app.add_to_examples(test::to_examples);
  app.add_test("test0",test::test0);
  app.add_test("test1",test::test1);
  app.add_solve_for("1",part1::solve_for,"example.txt");
  app.add_solve_for("1",part1::solve_for,"puzzle.txt");
  app.add_solve_for("2",part2::solve_for,"example.txt");
  app.add_solve_for("2",part2::solve_for,"puzzle.txt");
  app.run(argc, argv);
  app.print_result();
  /*

   Xcode Debug -O2
   
   For my input:
            
   >day_21 -all
         
   ANSWERS
   duration:2ms answer[part:"test0"] PASSED
   duration:6961ms answer[part:"test1"] PASSED
   duration:5951ms answer[part 1 in:example.txt] 126384
   duration:80721ms answer[part 1 in:puzzle.txt] 179444
   duration:6ms answer[part 2 in:example.txt] 126384
   duration:46ms answer[part 2 in:puzzle.txt] 223285811665866
   
   */

}
