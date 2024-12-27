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
std::ostream& operator<<(std::ostream& os,Gate const& gate) {
  os << '(' << "setq " << gate.output <<  " (
  
  " << gate.op << " " << gate.input1 << " " << gate.input2 << "))"; // lisp :)
  return os;
}
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

  Model to_evaluated(WireValues const& init_values,Gates const& gates) {
    Model result{init_values,gates};
    auto& [wire_vals,ops] = result;
    std::deque<Gate> q{ops.begin(),ops.end()};
    while (not q.empty()) {
      auto [a,b,out,op] = q.front();
      q.pop_front();
      if (wire_vals[a] and wire_vals[b]) {
        wire_vals[out] = applyGate(*wire_vals[a], *wire_vals[b], op);
      }
      else {
        q.push_back({a,b,out,op}); // retry later
      }
    }
    return result;
  }

  std::string to_bin_digit_string(char id,WireValues const& wire_vals) {
    std::string result{};
    for (const auto& [wire, value] : wire_vals) {
//
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
        auto const& [vals,gates] = to_evaluated(model.init_values,model.gates);
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
      auto [wire_vals,gates] = test::to_evaluated(model.init_values,model.gates);
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

  void rename_wire(std::string const& old_name,std::string const& new_name,Model& model) {
    if (old_name==new_name) return;

    
    std::cout << NL << "rename: " << old_name << " --> " << new_name;
    
    std::for_each(model.gates.begin(), model.gates.end(), [&old_name,&new_name](Gate& target){
      if (target.input1 == old_name) target.input1 = new_name;
      if (target.input2 == old_name) target.input2 = new_name;
      if (target.output == old_name) target.output = new_name;
    });
    // A bit cumbersome to rename a key in an std::map
    // Also costly - so lets see if we can do this without getting too long solve time?
    auto iter = model.init_values.find(old_name);
    if (iter != model.init_values.end()) {
      auto value = iter->second;
      model.init_values.erase(iter);
      model.init_values[new_name] = value;
    }
  }

  Model to_renamed_gates(Model const& model) {
    Model result{model};
    auto& [wire_vals,gates] = result;
    // What if we rename wires as we identify them and see what that gets us?
    for (auto const& gate : model.gates) {
      // not_z = xnn ^ ynn : not_z -> xy_xor_nn
      // any = xnn & ynn : any -> xy_and_nn
      if (    (     (gate.input1.starts_with('x') and gate.input2.starts_with('y'))
                 or (gate.input1.starts_with('y') and gate.input2.starts_with('x'))
              )
          and ( not (gate.output.starts_with('z')))) {

        auto bit_no = gate.input1.substr(1);
        std::string new_name = std::format("_xy_xor_{}",bit_no);
        if (gate.op == "AND") {
          new_name = std::format("_xy_and_{}",bit_no);
        }
        auto old_name = gate.output;
        rename_wire(old_name, new_name, result);
      }

      
//      // jdm := bng OR _xy_and_10  : jdm -> cin_nn
//      if (gate.op == "OR" and gate.input2.starts_with("_xy_and")) {
//        auto old_name = gate.output;
//        auto bit_no = gate.input2.substr(gate.input2.size()-2);
//        std::string new_name = std::format("_cin_{}",bit_no);
//        rename_wire(old_name, new_name, model);
//      }
//            
//      // _cin_10 := bng OR _xy_and_10 : bng -> cin_and_10
//      if (gate.output.starts_with("_cin_") and gate.input2.starts_with("_xy_and_")) {
//        auto old_name = gate.input1;
//        auto bit_no = gate.input2.substr(gate.input2.size()-2);
//        std::string new_name = std::format("_cin_and_{}",bit_no);
//        rename_wire(old_name, new_name, model);
//      }
//            
//      // z17 := _xy_and_17 OR ffg : ffg -> cin_and_nn
//      if (gate.op == "OR" and gate.input1.starts_with("_xy_and_")) {
//        auto old_name = gate.input2;
//        auto bit_no = gate.input1.substr(gate.input1.size()-2);
//        std::string new_name = std::format("_cin_and_{}",bit_no);
//        rename_wire(old_name, new_name, model);
//      }
//      // z12 := _xy_xor_12 XOR sfm  : sfm -> _cin_12
//      if (gate.output.starts_with('z') and gate.input1.starts_with("_xy_xor")) {
//        auto old_name = gate.input2;
//        auto bit_no = gate.input1.substr(gate.input1.size()-2);
//        std::string new_name = std::format("_cin_{}",bit_no);
//        rename_wire(old_name, new_name, model);
//      }
    }
    return result;
  }

  //           z = x ^ y ^ carry_in
  //   carry_out = x & y + (carry_in & (x ^ y))
  //           dc = x & y   direct carry
  //           rc = cin & either_xy  re-carry
  //            c = dc + rc
  enum eGate  {
    eGate_Undefined
    ,eGate_either_xy // xnn XOR ynn
    ,eGate_direct_carry // xnn AND ynn
    ,eGate_re_carry // cin & either_xy
    ,egate_carry // eGate_direct_carry OR eGate_re_carry
    ,egate_s // eGate_either_xy XOR egate_carry
    ,eGate_Unknown
  };

  std::string to_string(eGate id) {
      switch (id) {
          case eGate_Undefined:   return "eGate_Undefined";
          case eGate_either_xy:   return "eGate_either_xy";
          case eGate_direct_carry: return "eGate_direct_carry";
          case eGate_re_carry:    return "eGate_re_carry";
          case egate_carry:       return "egate_carry";
          case egate_s:           return "egate_s";
          case eGate_Unknown:     return "eGate_Unknown";
          default:                return "Unknown eGate";
      }
  }

  std::ostream& operator<<(std::ostream& os,eGate id) {
    os << to_string(id);
    return os;
  }

  std::pair<std::string,std::string> to_prefix_and_bit(std::string const& wire_name) {
    std::pair<std::string,std::string> result{wire_name,""};
    std::regex pattern(R"((\S+?)(\d*)$)");
    std::smatch match;
    if (std::regex_match(wire_name.begin(), wire_name.end(), match, pattern)) {
      result.first = match[1];
      result.second = match[2];
    }
    return result;
  }

  bool has_prefixed_in_wire(std::string const& prefix,Gate const& gate) {
    return (gate.input1.starts_with(prefix) or gate.input2.starts_with(prefix));
  }

  std::string to_prefix(std::string const& wire_name) {
    return to_prefix_and_bit(wire_name).first;
  }

  std::string to_bit(std::string const& wire_name) {
    return to_prefix_and_bit(wire_name).second;
  }

  bool is_gate(std::string const& op,std::string const& in_a,std::string const& in_b,Gate const& gate) {
    bool is_op = gate.op == op;
    bool has_also_in_a = is_op & has_prefixed_in_wire(to_prefix(in_a),gate);
    bool has_also_in_b = has_also_in_a & has_prefixed_in_wire(to_prefix(in_b),gate);
    return has_also_in_b;
  }

  Gate const& to_gate(std::string const& output_name,Gates const& gates) {
    auto iter = std::find_if(gates.begin(), gates.end(), [&output_name](Gate const& gate){
      return (gate.output == output_name);
    });
    if (iter != gates.end()) return *iter;
    else throw std::runtime_error(std::format("Sorry, to_gate() failed for output_name {}",output_name));
  }

  bool is_xy_wire(std::string const& wire_name) {
    auto [prefix,bit] = to_prefix_and_bit(wire_name);
    return (prefix == "x" or prefix == "y");
  }

  bool has_xy_input(Gate const& gate) {
    return is_xy_wire(gate.input1) and is_xy_wire(gate.input2);
  }

  eGate to_gate_id(Gate const& gate,Gates const gates) {
//    std::cout << NL << NL << "to_gate_id:" << gate;
    
    eGate result{eGate_Undefined};
    using aoc::raw::operator++;

    for (auto id=aoc::raw::advance(eGate_Undefined,1);id<eGate_Unknown;++id) {
//      std::cout << NL << "id:" << id << gate;
      
      switch (id) {
        case eGate_either_xy: {
          if (is_gate("XOR","x","y",gate)) result = id;
        } break;
        case eGate_direct_carry: {
          if (is_gate("AND","x","y",gate)) result = id;
        } break;
        case eGate_re_carry: {
          // cin & either_xy  re-carry
          if (not has_xy_input(gate) and to_gate_id(to_gate(gate.input2,gates),gates) == eGate_either_xy) result = id;
        } break;
        case egate_carry: {
          //,egate_carry // eGate_direct_carry OR eGate_re_carry
          // NOTE: Don't check input1 as this will result in infinite recursion back through bits
          if (not has_xy_input(gate) and (to_gate_id(to_gate(gate.input1,gates),gates) == eGate_direct_carry)) result = id;
        } break;
        case egate_s: {
          //,egate_s // eGate_either_xy XOR egate_carry
          // None of the above
          if (not has_xy_input(gate) and gate.op == "XOR" and to_gate_id(to_gate(gate.input1,gates), gates) == eGate_either_xy) result = id;
        } break;
        default: break;
      }
    }
    return result;
  }

  int to_identified_gates(Gates const& gates) {
    int result{};
    for (auto const& gate : gates) {
      auto gate_id = to_gate_id(gate, gates);
      auto const& [prefix,bit] = to_prefix_and_bit(gate.output);
      std::cout << NL << T << "bit:" << bit << " " << gate_id << ":" << gate;
      if (gate_id != eGate_Undefined or gate_id != eGate_Undefined) {
        ++result;
        std::cout << " " << result;
      }
    }
    return result;
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

      auto const _in_model = parse(in);
      auto const init_model = test::to_evaluated(_in_model.init_values,_in_model.gates);
      if (args.options.contains("-parse_only")) return "-parse_only";
      if (args.options.contains("-to_dot")) {
        std::vector<std::string> dot{};
        dot.push_back("digraph G {");
        for (auto const& [a,b,u,op] : init_model.gates) {
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
      
      if (args.options.contains("-to_ids")) {
        auto ids = to_identified_gates(init_model.gates);
        return std::format("-to_ids: {} out of {}",ids,init_model.gates.size());
      }
      
      std::set<std::string> x_and_y_names{};
      std::for_each(init_model.gates.begin(), init_model.gates.end(), [&x_and_y_names](Gate const& gate){
        if (gate.input1.starts_with('x') or gate.input1.starts_with('y')) x_and_y_names.insert(gate.input1);
        if (gate.input2.starts_with('x') or gate.input2.starts_with('y')) x_and_y_names.insert(gate.input2);
      });

      
      //According to wikipedia: For a full adder ow two bits and carry we have
      // to implement z = c_in + x + y
      //           z = x ^ y ^ carry_in
      //   carry_out = x & y + (carry_in & (x ^ y))
      // IDEA: Break down these expressions into two-input gates and name them.
      //       xnn ^ ynn                : xy_xor_nn
      //       xy_xor_nn ^ cin_nn       : znn
      //       xnn & ynn                : xy_and_nn
      //       cin_nn & xy_xor_nn       : cin_and_nn
      //       xy_and_nn or cin_and_nn  : cout_nn -> cin_pp (pp = nn+1)

      
      using Swap = std::pair<std::string,std::string>;
      using Swaps = std::vector<Swap>;
      Swaps found_swaps{};

      struct State {
        Swaps swaps; // applied swaps so far
        Model model;
        int bit_no; // Bit to process
      };
      std::deque<State> q{};
      auto start = State{{},init_model,0};
      q.push_front(start);
      Integer best{std::numeric_limits<int>::max()}; // best number of faulty z so far
      while (not q.empty()) {
        State current = q.front(); // ensure a copy is made
        q.pop_front();

        using aoc::raw::operator<<;
        std::cout << NL << NL << "Process bit:" << current.bit_no << " swaps:" << current.swaps;
                  
        auto& [applied_swaps,model,last_bit_no] = current;
        auto& [wire_vals,gates] = model;
        
        model = to_renamed_gates(model);
        
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
        
        if (diff_count==0) {
          found_swaps = current.swaps;
          break;
        }
        if (diff_count<best) {
          best = diff_count;
          found_swaps=current.swaps;
          using aoc::raw::operator<<;
          std::cout << NL << "new best:" << best << " for swaps:" << current.swaps;
        }
                            
        auto to_wire_name = [](std::string const& prefix,int bit_number) -> std::string {
          return std::format("{}{:02}",prefix,bit_number); // z00 is i = (N-2)
        };
                  
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

        // i = digit string index N-2 ... 0
        // bit_number               0 ... N-2
        int bit_number = current.bit_no;
        auto i = (N-2) - bit_number;
        auto s_digit = s_digits[i];
        auto z_digit = z_digits[i+1];
        
        if (z_digit == s_digit) {
          // try next z-bit
          q.push_front(State{current.swaps,current.model,bit_number+1});
        }
        else {
          int backtrack_level{1};

          std::cout << NL << NL; // new z-bit

          // For a full bit adder we expect to find gates that takes x and why for current bit number
          if (true) {
            auto iter = gates.begin();
            auto x_wire_name = to_wire_name("x", bit_number);
            auto y_wire_name = to_wire_name("y", bit_number);

            while (iter != gates.end()) {
              iter = std::find_if(iter, gates.end(), [&x_wire_name,&y_wire_name](Gate const& gate) {
                return     (gate.input1==x_wire_name and gate.input2==y_wire_name)
                        or (gate.input1==y_wire_name and gate.input2==x_wire_name);
              });
              if (iter != gates.end()) {
                std::cout << NL << std::string(2*backtrack_level,' ') << "candidate:" << iter->output << " := " << iter->input1 << " " << iter->op << " " << iter->input2;
                ++iter;
              }
            }
          }

          
          auto c_in_digit = carry_digits[i+1];
          auto x_digit = x_digits[i];
          auto y_digit = y_digits[i];
          auto c_out_digit = carry_digits[i];
          
          auto wire_name = to_wire_name("z", bit_number);
          if (wire_vals[wire_name]) {
            auto iter = std::find_if(gates.begin(), gates.end(), [&wire_name](Gate const& gate){
              return gate.output == wire_name;
            });
            if (iter != gates.end()) {
              std::cout << NL << std::string(2*backtrack_level,' ') << iter->output << " := " << iter->input1 << " " << iter->op << " " << iter->input2 << " val:" << *wire_vals[wire_name];
              
              //z11 := _xy_and_11 XOR jdm val:0
              //    _xy_and_11 := y11 AND x11 val:0 --> Swap with the XOR variant with same inputs
              //    jdm := bng OR _xy_and_10 val:0
              if (iter->op == "XOR") {
                if (iter->input1.starts_with("_xy_and_")) {
                  // This must be _xy_xor_ so swap
                  auto bit_digits = iter->input1.substr(iter->input1.size()-2);
                  std::string output_other = std::format("_xy_xor_{}",bit_digits);
                  auto iter_other = std::find_if(gates.begin(),gates.end(),[&output_other](Gate const& gate){
                    return gate.output == output_other;
                  });
                  if (iter_other != gates.end()) {
                    // Shoot! What does it even mean to 'swap two gates?
                    // I want z11 := _xy_and_11 XOR jdm to be z11 := _xy_xor_11 XOR jdm
                    // How do I 'swap' to make this happen?
                    // Ok, I want to replace the 'gate' with output _xy_and_11 with the gate '_xy_xor_11'
                    // ...
                    std::cout << NL << "swap " << iter->input1 << " with " << output_other << ". HOW?";
                  }
                }
              }

              
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

                // to implement z = c_in + x + y
                //           z = x ^ y ^ carry_in
                //   carry_out = x & y + (carry_in & (x ^ y))

                //    x11        y11    carry_in 11 -      x10       y10         x10        y10
                //     |          |         |        |      |          |          |          |             carry_in 10
                //      ->--XOR--<-         v        ^       -- AND ---           ---- XOR ---                |
                //           |              |        |           |                      |                     |
                //           v              |        |           |                      |                     |
                //           ?? -- XOR --- ??        |           |                       ---------- AND -------
                //                   |               |           |                                   |
                //                  z11              ??          --------------- OR ------------------
                //                                   |                            |
                //                                   |                           carry_out 10
                //                                   |                                |
                //                                   ------------------------------<--
                                
                ++backtrack_level;
                bool swap_candidate_found{false};
                // Recurse one level lhs1
                {
                  auto wire_name = iter->input1;
                  auto iter = std::find_if(gates.begin(), gates.end(), [&wire_name](Gate const& gate){
                    return gate.output == wire_name;
                  });
                  if (iter != gates.end()) {
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
                  auto iter = std::find_if(gates.begin(), gates.end(), [&wire_name](Gate const& gate){
                    return gate.output == wire_name;
                  });
                  if (iter != gates.end()) {
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


            
          } // if z wire
          
          
          std::cout << NL << "q:" << q.size();
          if (true) {
            std::cout << NL << "BREAK after first faulty bit - for test" << std::flush;
            break;
          }
        }
      }
      using aoc::raw::operator<<;
      std::cout << NL << "FOUND swaps:" << found_swaps;
      if (found_swaps.size()==4) {
        std::cout << NL << "FOUND 4 pairs to swap OK!";
        std::vector<std::string> swap_names{};
        for (auto const& [left,right]: found_swaps) {
          swap_names.push_back(left);
          swap_names.push_back(right);
        }
        std::ranges::sort(swap_names);
        for (auto const& [ix,name] : aoc::views::enumerate(swap_names)) {
          if (ix>0) response << ',';
          response << name;
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
