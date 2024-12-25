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
using aoc::Args;

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = aoc::raw::Line;

using WireValues = std::map<std::string,std::optional<bool>>;
struct Gate {
  std::string input1;
  std::string input2;
  std::string output;
  std::string op; // "AND", "OR", "XOR"
  auto operator<=>(const Gate&) const = default;
};
using Gates = std::vector<Gate>;

struct Model {
  WireValues init_values{};
  Gates gates{};
};

Model parse(auto& in) {
  using namespace aoc::parsing;
  Model result{};
  auto sections = Splitter{in}.sections();
  for (auto const& [sx,section] : aoc::views::enumerate(sections)) {
    std::cout << NL << "---------- section " << sx << " ----------";
    for (auto const& [lx,line] : aoc::views::enumerate(section)) {
      std::cout << NL << T << T << "line[" << lx << "]:" << line.size() << " " << std::quoted(line.str());
      if (sx == 0) {
        auto const& [caption,value] = line.split(':');
        std::cout << " --> " << to_raw(caption) << " " << to_raw(value);
        result.init_values[caption] = (value.trim().str() == "1")?true:false;
      }
      else if (sx==1) {
        auto tokens = line.splits(' ');
        using aoc::raw::operator<<;
        std::cout << " " << to_raw(tokens);
        result.gates.push_back({tokens[0],tokens[2],tokens[4],tokens[1]});
        if (tokens[0].str().starts_with('z')) {
          std::cout << NL << T << "GATE has z-input!"; // Never happens for my input
        }
      }
      else {
        std::cerr << NL << "Sorry, Parse ERROR: More than two section is unexpected";
      }
    }
  }
  return result;
}

namespace test {

  std::optional<bool> applyGate(int val1, int val2, const std::string& op) {
      if (op == "AND") return val1 and val2;
      if (op == "OR") return val1 or val2;
      if (op == "XOR") return val1 xor val2;
      throw std::invalid_argument("Unknown operation");
  }

  WireValues to_evaluated(Model model) {
    WireValues result{};
    auto const& [wire_vals,ops] = model;
    WireValues has_val{model.init_values};
    std::deque<Gate> q{ops.begin(),ops.end()};
    while (not q.empty()) {
      auto [a,b,out,op] = q.front();
      q.pop_front();
      if (has_val[a] and has_val[b]) {
        has_val[out] = applyGate(*has_val[a], *has_val[b], op);
      }
      else {
        q.push_back({a,b,out,op}); // retry later
      }
    }
    result = has_val;
    return result;
  }

  std::string to_bin_digit_string(char id,WireValues const& wire_vals) {
    std::string result{};
    for (const auto& [wire, value] : wire_vals) {
      std::cout << NL << wire << ": " << *value;
      if (wire.starts_with(id)) result.push_back(*value?'1':'0');
    }
    std::reverse(result.begin(), result.end()); // requires wire_vals alphabetically sorted
    return result;
  }

  auto to_int(std::string bin_digits) {
    std::bitset<64> bits{bin_digits};
    return bits.to_ulong();;
  }

  // Adapt to expected for day puzzle
  struct LogEntry {
    bool operator==(LogEntry const& other) const {
      bool result{true};
      return result;
    }
  };

  std::ostream& operator<<(std::ostream& os,LogEntry const& entry) {
    return os;
  }

  using LogEntries = aoc::test::LogEntries<LogEntry>;

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
    return {};
  }

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::ostringstream response{};
    std::cout << NL << NL << "test";
    if (in) {
      auto model = parse(in);
      auto doc = parse_doc(args);
      auto example_lines = to_example(doc);
      if (args.options.contains("-to_example")) {
        auto example_file = aoc::to_working_dir_path("example.txt");
        if (aoc::raw::write_to_file(example_file, example_lines)) {
          response << "Created " << example_file;
        }
        else {
          response << "Sorry, failed to create file " << example_file;
        }
        return response.str();
      }
      else {
        auto vals = to_evaluated(model);
        auto z_digits = to_bin_digit_string('z',vals);
        std::cout << NL << "zs:" << z_digits.size() << " " << z_digits;
        auto z = to_int(z_digits);
        std::cout << " --> decimal:" << z;
        response << z;
      }
    }
    if (response.str().size()>0) return response.str();
    else return std::nullopt;
  }

}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::ostringstream response{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      auto vals = test::to_evaluated(model);
      auto z_digits = test::to_bin_digit_string('z',vals);
      std::cout << NL << "zs:" << z_digits.size() << " " << z_digits;
      auto z = test::to_int(z_digits);
      std::cout << " --> decimal:" << z;
      response << z;
    }
    if (response.str().size()>0) return response.str();
    else return std::nullopt;
  }
}

namespace part2 {

  using Swaps = std::vector<std::pair<int, int>>;

  // Function to calculate binary addition
  bool checkSum(const std::vector<int>& x, const std::vector<int>& y, const std::vector<int>& z) {
    int carry = 0;
    for (std::size_t i = 0; i < x.size(); ++i) {
      int sum = x[i] + y[i] + carry;
      carry = sum / 2; // Carry for the next bit
      if ((sum % 2) != z[i]) {
        return false; // Early exit if mismatch
      }
    }
    return carry == 0; // Ensure no leftover carry
  }

  // Main function to find swaps
  Swaps findZSwaps(const std::vector<int>& x, const std::vector<int>& y, const std::vector<int>& z) {

    auto n = z.size();
    
    // Generate combinations of 8 wires from z
    std::vector<int> indices(n);
    std::iota(indices.begin(), indices.end(), 0); // Fill indices with 0, 1, ..., n-1
    
    // Try all combinations of 8 wires
    std::vector<int> selectionMask(n, 0);
    std::fill(selectionMask.end() - 8, selectionMask.end(), 1); // Mask for choosing 8 wires
    
    do {
      std::vector<int> selectedWires;
      for (int i = 0; i < n; ++i) {
        if (selectionMask[i] == 1) {
          selectedWires.push_back(indices[i]);
        }
      }
      
      // Permute the selected 8 wires
      do {
        // Divide into four pairs of wires
        Swaps swaps = {
          {selectedWires[0], selectedWires[1]},
          {selectedWires[2], selectedWires[3]},
          {selectedWires[4], selectedWires[5]},
          {selectedWires[6], selectedWires[7]}
        };
                
      } while (std::next_permutation(selectedWires.begin(), selectedWires.end()));
      
    } while (std::next_permutation(selectionMask.begin(), selectionMask.end()));
    
    return {}; // Return empty vector if no solution is found
  }

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::ostringstream response{};
    std::cout << NL << NL << "part2";
    if (in) {
      
      // 20241224 - Other solvers sais the structure is a ripple carry adder?
      //            https://en.wikipedia.org/wiki/Adder_(electronics)
      
      // 20241224 - Viewing the graph in GraphViz I get some ideas
      // Idea: 1. Find the x,y and z valus in the graph
      //       2. Backtrack the z bits from high to low.
      //       3. For addition x+y for this bit we have have some options.
      //          carry x y z carry
      //              0 0 0 0 0
      //              0 0 1 1 0
      //              0 1 0 1 0
      //              0 1 1 1 1
      //              1 0 0 0 1
      //              1 0 1 0 1
      //              1 1 0 0 1
      //              1 1 1 1 1
      //
      //        4. If the adder for current bit produces the wrong z-bit, then
      //           is 0 but should be 1
      //
      //          carry x y z carry
      //              0 0 1 1 0
      //              0 1 0 1 0
      //              0 1 1 1 1
      //              1 1 1 1 1
      //
      //           The fix is to change the x-bit, the y-bit or the carry-bit
      //           All will flip the z-bit to be correct
      //
      //           In fact, this is true wether z is wrong as 0 or 1.
      //           Question is, what swap flips x,y or carry in?
      //                        ,and what swaps are safe (do not break ok z-bits?
      //
      //            According to wikipedia: For a full adder ow two bits and carry we have
      //            z = x ^ y ^ in
      //            out = x & y + (in & (x ^y))
      
      // Idea: Follow each z-bit back to root initial values and record the operations on the way back?
      //       Well, problem is that if this is an adder the carry bit will lead all the way back to x0,y0??
      //       But for z_n it should not be far to x_n and y_n?!
      
      // 241225 Reduce the serach space
      //        1. Only swap to NOT change the wire values we allready have checked as ok
      //        2. Perhaps we can even swap only wires that affect the output of only the z-bit we are checking (none of the others)
      //        3. swp only so that current wrong z-bit flips (no need to swap with no effect)

      auto model = parse(in);
      if (args.options.contains("-parse_only")) return "-parse_only";
      if (args.options.contains("-to_dot")) {
        std::vector<std::string> dot{};
        dot.push_back("digraph G {");
        for (auto const& [a,b,u,op] : model.gates) {
          dot.push_back(std::format(R"({} -> {} [label="{}"];)",a,u,op));
          dot.push_back(std::format(R"({} -> {} [label="{}"];)",b,u,op));
        }
        dot.push_back("}");
        auto file = aoc::to_working_dir_path("graph.dot");
        std::ofstream out{file};
        // copy dot (std::vector<string>>) to out
        std::copy(dot.begin(),dot.end(),std::ostream_iterator<std::string>(out,NL));
        return std::string{"-to_dot, created dot file "} + file.string();
      }
      

    }
    if (response.str().size()>0) return response.str();
    else return std::nullopt;
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
