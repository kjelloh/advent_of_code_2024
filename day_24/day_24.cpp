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
      auto wire_vals = test::to_evaluated(model);
      auto z_digits = test::to_bin_digit_string('z',wire_vals);
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

  // sum,carry
  std::pair<std::string, std::string> to_bitwise_added(const std::string& x, const std::string& y) {
      // Ensure x and y are of the same length by padding with leading zeros
      auto len = std::max(x.size(), y.size());
      std::string x_padded = std::string(len - x.size(), '0') + x;
      std::string y_padded = std::string(len - y.size(), '0') + y;
      
      std::string sum(len, '0');      // Resulting sum
      std::string carry(len + 1, '?'); // Carry bits, both 0 at [len] and carry out at [0]
      
      char carry_in = '0'; // Initial carry is zero
      
      // Traverse from LSB to MSB
      for (int i = static_cast<int>(len-1); i >= 0; --i) {
          // Perform bitwise addition for the current bit
          int bit_x = x_padded[i] - '0';
          int bit_y = y_padded[i] - '0';
          int bit_carry_in = carry_in - '0';
          
          int bit_sum = bit_x ^ bit_y ^ bit_carry_in; // XOR for sum
          int bit_carry_out = (bit_x & bit_y) | (bit_x & bit_carry_in) | (bit_y & bit_carry_in); // OR for carry
          
          sum[i] = bit_sum + '0';   // Convert back to char
          carry[i] = bit_carry_out + '0'; // Store carry at i+1 to handle MSB carry-out
          
          carry_in = carry[i]; // Update carry for the next iteration
      }
      
      return {sum, carry};
  }

  struct BitSumResult {
    char sum;
    char carry_out;
  };

  BitSumResult to_added(char x_digit, char y_digit, char c_in_digit) {
    bool x = x_digit == '1';
    bool y = y_digit == '1';
    bool c_in = c_in_digit == '1';
    // Compute the sum and carry-out
    bool sum = x ^ y ^ c_in; // XOR for sum
    bool carry_out = (x & y) | (y & c_in) | (c_in & x); // Carry-out logic
        
    return {sum, carry_out};
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
      auto wire_vals = test::to_evaluated(model);
      auto x_digits = test::to_bin_digit_string('x',wire_vals);
      auto y_digits = test::to_bin_digit_string('y',wire_vals);
      auto z_digits = test::to_bin_digit_string('z',wire_vals);
      
      auto const N = static_cast<int>(z_digits.size()); // N z digits
      
      auto const& [s_digits,carry_digits] = to_bitwise_added(x_digits, y_digits);
      
      std::cout << NL << "carry:" << carry_digits;
      std::cout << NL << "   x:  " << x_digits;
      std::cout << NL << "+  y:  " << y_digits;
      std::cout << NL << "-------" << std::string(N-1,'-');
      std::cout << NL << "=  s:  " << s_digits;
      std::cout << NL << "   z: " << z_digits;

      // Find all digit suming that goes wrong
      // digit[N-1] is the rightmost lowest bit
      std::string diff_string(N-1,' ');
      for (int i=N-2;i>=0;--i) {
        auto c_digit = carry_digits[i+1];
        auto x_digit = x_digits[i];
        auto y_digit = y_digits[i];
        auto s_digit = s_digits[i];
        auto z_digit = z_digits[i+1];
        if (z_digit != s_digit) diff_string[i] = '?';
      }
      std::cout << NL << "diff:  " << diff_string;
      auto diff_count = std::count(diff_string.begin(), diff_string.end(), '?');
      std::cout << NL << "diff count:" << diff_count;
      
      //           4         3         2         1         0
      //carry:100111111110111111111100001111110000000111001?
      //   x:  100010011110101101010110001010110100000011001
      //+  y:  110111100110010010111101100101110001000111001
      //----------------------------------------------------
      //=  s:  011010000101000000010011110000100101001010010
      //   z: 1011010000100111111110011101100101001001010010
      //diff:             ????????      ???    ??
      //diff count:13

      // Now given addition is done low significant to high significant with carry
      // It seems the first error is at bit 11?
      // A carry plus one x-bit should add to 1 but z is 0.
      // So - the carry bit is lost (wired somewhere else)?

      // Find missmatch and try to figure out
      // if it is x,y or carry_in that is in fault?
      for (int i=N-2;i>=0;--i) {
        int bit_number = (N-2)-i;
        auto s_digit = s_digits[i];
        auto z_digit = z_digits[i+1];
        
        if (z_digit != s_digit) {
          
          auto c_in_digit = carry_digits[i+1];
          auto x_digit = x_digits[i];
          auto y_digit = y_digits[i];
          auto c_out_digit = carry_digits[i];
          
          auto to_wire_name = [](char prefix,int bit_number) -> std::string {
            return std::format("{}{:02}",prefix,bit_number); // z00 is i = (N-2)
          };
          
          std::set<std::string> x_and_y_names{};
          for (int i=0;i<x_digits.size();++i) {
            x_and_y_names.insert(to_wire_name('x', i));
            x_and_y_names.insert(to_wire_name('y', i));
          }

          auto wire_name = to_wire_name('z', bit_number);
          if (wire_vals[wire_name]) {
            auto iter = std::find_if(model.gates.begin(), model.gates.end(), [&wire_name](Gate const& gate){
              return gate.output == wire_name;
            });
            int backtrack_level{1};
            if (iter != model.gates.end()) {
              std::cout << NL << NL << std::string(2*backtrack_level,' ') << iter->output << " := " << iter->input1 << " " << iter->op << " " << iter->input2 << " val:" << *wire_vals[wire_name];
              
              //According to wikipedia: For a full adder ow two bits and carry we have
              // to implement z = c_in + x + y
              //           z = x ^ y ^ carry_in
              //   carry_out = x & y + (carry_in & (x ^ y))

              // Two cases:
              // 1) z is an XOR of its inputs
              // 2) z is NOT an XOR of its inputs
              if (iter->op == "XOR") {
                // track case: z = x ^ y ^ carry_in
                
                // From logging we see all XOR for is z done in this way
                // with x,y done by XOR gate one level 'back'
                //
                //    x11        y11    carry_in
                //     |          |         |
                //      ----XOR---          |
                //           |              |
                //           |              |
                //           ?? -- XOR --- ??
                //                   |
                //                  z11
                
                ++backtrack_level;
                bool swap_candidate_found{false};
                // Recurse one level lhs1
                {
                  auto wire_name = iter->input1;
                  auto iter = std::find_if(model.gates.begin(), model.gates.end(), [&wire_name](Gate const& gate){
                    return gate.output == wire_name;
                  });
                  if (iter != model.gates.end()) {
                    std::cout << NL << std::string(2*backtrack_level,' ') << iter->output << " := " << iter->input1 << " " << iter->op << " " << iter->input2 << " val:" << *wire_vals[wire_name];
                  }
                  if ((x_and_y_names.contains(iter->input1) and x_and_y_names.contains(iter->input2))) {
                    if (iter->op == "XOR") {
                      std::cout << " OK ";
                    }
                    else {
                      std::cout << " --> Swap with the XOR variant with same inputs";
                      swap_candidate_found = true;
                    }
                  }
                  else {
                    if (iter->op == "OR") {
                      std::cout << " --> Back track carry in = previous carry_out = x & y + (carry_in & (x ^ y))";
                      swap_candidate_found = true;
                    }
                    else if (iter->op == "XOR") {
                      std::cout << " --> Swap for OR to get carry in = previous carry_out = x & y + (carry_in & (x ^ y))";
                    }
                    else {
                      std::cout << " --> ?? ";
                    }
                  }
                }
                // Recurse one level lhs2
                {
                  auto wire_name = iter->input2;
                  auto iter = std::find_if(model.gates.begin(), model.gates.end(), [&wire_name](Gate const& gate){
                    return gate.output == wire_name;
                  });
                  if (iter != model.gates.end()) {
                    std::cout << NL << std::string(2*backtrack_level,' ') << iter->output << " := " << iter->input1 << " " << iter->op << " " << iter->input2 << " val:" << *wire_vals[wire_name];
                  }
                  
                  if (swap_candidate_found) continue;
                  
                  if ((x_and_y_names.contains(iter->input1) and x_and_y_names.contains(iter->input2))) {
                    if (iter->op == "XOR") {
                      std::cout << " OK ";
                    }
                    else {
                      std::cout << " --> Swap with the XOR variant with same inputs";
                    }
                  }
                  else {
                    if (iter->op == "OR") {
                      std::cout << " --> Back track carry in = previous carry_out = x & y + (carry_in & (x ^ y))";
                    }
                    else if (iter->op == "XOR") {
                      std::cout << " --> Swap for OR to get carry in = previous carry_out = x & y + (carry_in & (x ^ y))";
                    }
                    else {
                      std::cout << " --> ?? ";
                    }
                  }
                }
              }
              else {
                std::cout << " --> SWAP this gate with gate that IS an XOR for x ^ y ^ carry_in";
              }
                            
            }
            else {
              std::cout << " ??";
            }
          }
        }
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
