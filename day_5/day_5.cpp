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

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    Result acc{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      print(model);
      using CandidateRefs = std::vector<Updates::const_iterator>;
      std::pair<CandidateRefs,CandidateRefs> candidate{};
      for (auto iter=model.updates.begin();iter != model.updates.end();++iter) {
        candidate.first.push_back(iter);
      }
      // Apply each rule to each in candidate.first and keep in candidate.second if applies
      for (auto const& rule : model.rules) {
        print(NL);
        print(rule);
        
        // try current candidates
        for (auto cand_iter : candidate.first) {
          std::map<PageNo,Update::const_iterator> p2i{};
          for (auto page_iter=cand_iter->begin();page_iter != cand_iter->end();++page_iter) {
            p2i[*page_iter] = page_iter;
          }
          if (not p2i.contains(rule.first) or not p2i.contains(rule.second)) {
            print(NL);
            print(T);
            print(" OK (don't apply) on ");
            print(*cand_iter);
            candidate.second.push_back(cand_iter); // keep
          }
          else if (p2i[rule.first] < p2i[rule.second]) {
            print(NL);
            print(T);
            print(" OK on ");
            print(*cand_iter);
            candidate.second.push_back(cand_iter); // keep
          }
          else {
            print(NL);
            print(T);
            print(" FAILED on ");
            print(*cand_iter);
          }
        }
        candidate.first = candidate.second;
        candidate.second.clear();
      }
      print(NL);
      for (auto const& iter : candidate.first) {
        print(NL); print(T);
        auto middle = (*iter)[iter->size()/2];
        print(*iter);
        std::cout << " --> middle:" << middle;
        acc += middle;
        std::cout << " --> acc:" << acc;
      }
      result = acc;
    }
    return result;
  }
}

namespace part2 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    Result acc{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      print(model);
      using CandidateRefs = std::vector<Updates::const_iterator>;
      std::pair<CandidateRefs,CandidateRefs> candidate{};
      for (auto iter=model.updates.begin();iter != model.updates.end();++iter) {
        candidate.first.push_back(iter);
      }
      // Apply each rule to each in candidate.first and keep in candidate.second if applies
      for (auto const& rule : model.rules) {
        print(NL);
        print(rule);
        
        // try current candidates
        for (auto cand_iter : candidate.first) {
          std::map<PageNo,Update::const_iterator> p2i{};
          for (auto page_iter=cand_iter->begin();page_iter != cand_iter->end();++page_iter) {
            p2i[*page_iter] = page_iter;
          }
          if (not p2i.contains(rule.first) or not p2i.contains(rule.second)) {
            print(NL);
            print(T);
            print(" OK (don't apply) on ");
            print(*cand_iter);
            candidate.second.push_back(cand_iter); // keep
          }
          else if (p2i[rule.first] < p2i[rule.second]) {
            print(NL);
            print(T);
            print(" OK on ");
            print(*cand_iter);
            candidate.second.push_back(cand_iter); // keep
          }
          else {
            print(NL);
            print(T);
            print(" FAILED on ");
            print(*cand_iter);
          }
        }
        candidate.first = candidate.second;
        candidate.second.clear();
      }
      std::cout << NL << "FIX FAILED UPDATES";
      Updates updates_to_fix{};
      for (auto iter=model.updates.begin();iter != model.updates.end();++iter) {
        if (std::find(candidate.first.begin(),candidate.first.end(),iter) == candidate.first.end()) {
          // Not in valid candidates
          updates_to_fix.push_back(*iter);
        }
      }

      bool all_rules_pass{true};
      int updates_count{};
      do {
        all_rules_pass = true;
        updates_count=0;
        for (auto iter=updates_to_fix.begin();iter != updates_to_fix.end();++iter) {
          ++updates_count;
          if (std::find(candidate.first.begin(),candidate.first.end(),iter) == candidate.first.end()) {
            Update update_to_fix = *iter;
//            print(NL);print(NL);
//            std::cout << "TO FIX ";
//            print(update_to_fix);
            // Find rules that fails and use them to fix candidate
            for (auto const& rule : model.rules) {
//              print(NL);
//              print(rule);

              std::map<PageNo,Update::iterator> p2i{};
              for (auto page_iter=update_to_fix.begin();page_iter != update_to_fix.end();++page_iter) {
                p2i[*page_iter] = page_iter;
              }
              if (not p2i.contains(rule.first) or not p2i.contains(rule.second)) {
//                print(NL);
//                print(T);
//                print(" NO FIX - (don't apply) on ");
//                print(update_to_fix);
              }
              else if (p2i[rule.first] < p2i[rule.second]) {
//                print(NL);
//                print(T);
//                print(" NO FIX - OK on ");
//                print(update_to_fix);
              }
              else {
                all_rules_pass = false;
                print(NL);
                print(T);
                print(" FIX - FAILED on ");
                print(update_to_fix);
                // Swap the pages in the update as defined by the failed rule
                std::swap(*p2i[rule.first],*p2i[rule.second]);
                print(" FIXED = ");
                print(update_to_fix);
                *iter = update_to_fix;
                goto restart;
              }
            }
          }
        }
        restart:
        std::cout << NL << "updates_count:" << updates_count;
      } while (!all_rules_pass);
      auto const& fixed_updates = updates_to_fix;
      for (auto const& fixed_update : fixed_updates) {
        print(NL); print(NL);
        print(fixed_update);
        auto middle = fixed_update[fixed_update.size()/2];
        std::cout << " --> middle:" << middle;
        acc += middle;
        std::cout << " --> acc:" << acc;
      }
      result = acc;
    }
    return result;
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
  std::vector<int> states = {3};
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
   ...
  */
  return 0;
}
