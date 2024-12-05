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

auto const NL = "\n";
auto const T = "\t";
auto const NT = "\n\t";

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = Integer;
using PageNo = int;
using PageOrderingRule = std::pair<PageNo,PageNo>;
using PageOrderingRules = std::vector<PageOrderingRule>;
using Update = std::vector<PageNo>;
using Updates = std::vector<Update>;
struct Model {
  PageOrderingRules rules{};
  Updates updates{};
};

Model parse(auto& in) {
  std::cout << "\n<BEGIN parse>";
  Model result{};
  std::string line{};
  int state{0};
  while (std::getline(in,line)) {
    std::cout << "\nLine:" << std::quoted(line);
    if (line.size()==0) {
      state=1;
      continue;
    }
    switch (state) {
      case 0: {
        PageOrderingRule rule{};
        std::istringstream is{line};
        char sep{};
        is >> rule.first >> sep >> rule.second;
        std::cout << " --> " << rule.first << " " << rule.second;
        result.rules.push_back(rule);
      } break;
      case 1: {
        result.updates.push_back({});
        std::istringstream is{line};
        std::string token{};
        while (std::getline(is, token, ',')) {
          result.updates.back().push_back({});
          std::cout << " -> " << token;
          result.updates.back().back() = std::stoi(token);
        }
      } break;
      default: {
        std::cerr << NL << "Unknown state:" << state;
      }
    }
  }
  std::cout << "\n<END parse>";
  return result;
}

using Args = std::vector<std::string>;

void print(std::string s) {
  std::cout << s;
}
void print(PageOrderingRule const& rule) {
  std::cout << rule.first << " then " << rule.second;
}
void print(PageOrderingRules const& rules) {
  for (auto const& rule : rules) {
    print(NL);
    print(rule);
  }
}
void print(Update const& update) {
  for (auto const& n : update) std::cout << " " << n;
}
void print(Updates const& updates) {
  for (auto const& update : updates) {
    print(NL);
    print (update);
  }
}
void print(Model const& model) {
  print(NL);
  print(model.rules);
  print(NL);
  print(model.updates);
}

class LessThan {
public:
  LessThan(PageOrderingRules const& rules) : m_rules{std::move(rules)} {};
  bool operator()(PageNo a,PageNo b) const {
    return std::any_of(m_rules.begin(),m_rules.end(),[first=a,second=b](auto const& rule){
      // returns true if rule macth (a to be considered less_than b
      // otherwise 'ignore' as in return false.
      return (first == rule.first and second == rule.second);
    });
  }
private:
  PageOrderingRules m_rules;
};

namespace common {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    Result acc{};
    if (in) {
      auto model = parse(in);
      print(model);
      Updates fixed_updates{model.updates};
      for (auto& fixed_update : fixed_updates) {
        print(NL);print(fixed_update);
        std::sort(fixed_update.begin(),fixed_update.end(),LessThan{model.rules});
        std::cout << " sorted --> ";
        print(fixed_update);
      }
      // Now we can identify valid and unvalid updates by
      // comparing if the sorted update differs or not
      std::vector<std::pair<Update,Update>> paired_updates{};
      for (int i=0;i<model.updates.size();++i) {
        paired_updates.push_back({model.updates[i],fixed_updates[i]});
      }
      // valid updates accumulator lambda
      auto acc_valid = [](auto acc,auto const& entry){
        if (entry.first == entry.second) {
          auto middle = entry.second[entry.second.size()/2];
          acc += middle;
          print(NL);
          print(entry.second);
          std::cout << " --> middle:" << middle << " acc:" << acc;
        }
        return acc;
      };
      // invalid updates acc lambda
      auto acc_invalid = [](auto acc,auto const& entry){
        if (entry.first != entry.second) {
          auto middle = entry.second[entry.second.size()/2];
          acc += middle;
          print(NL);
          print(entry.second);
          std::cout << " --> middle:" << middle << " acc:" << acc;
        }
        return acc;
      };
      
      // Choose lamda based on args
      if (args.size()>0 and args[0]=="invalid") {
        // part_2 option
        acc = std::accumulate(paired_updates.begin(),paired_updates.end(),acc,acc_invalid);
      }
      else {
        // default part_1
        acc = std::accumulate(paired_updates.begin(),paired_updates.end(),acc,acc_valid);
      }
      result = acc;
    }
    return result;
  }
}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    return common::solve_for(in, Args{"valid"});
  }
}

namespace part2 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    return common::solve_for(in, Args{"invalid"});
  }
}

using Answers = std::vector<std::pair<std::string,std::optional<Result>>>;
int main(int argc, char *argv[])
{
  Args args{};
  for (int i=0;i<argc;++i) {
    args.push_back(argv[i]);
  }
  Answers answers{};
  std::vector<std::chrono::time_point<std::chrono::system_clock>> exec_times{};
  exec_times.push_back(std::chrono::system_clock::now());
  std::vector<int> states = {0,1,2,3};
  for (auto state : states) {
    switch (state) {
      case 0: {
        std::filesystem::path file{"../../example.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1 Example",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 1: {
        std::filesystem::path file{"../../puzzle.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1     ",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 2: {
        std::filesystem::path file{"../../example.txt"};
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2 Example",part2::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
        exec_times.push_back(std::chrono::system_clock::now());
      } break;
      case 3: {
        std::filesystem::path file{"../../puzzle.txt"};
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
   duration:0ms answer[Part 1 Example] 143
   duration:389ms answer[Part 1     ] 4814
   duration:0ms answer[Part 2 Example] 123
   duration:376ms answer[Part 2     ] 5448
  */
  return 0;
}
