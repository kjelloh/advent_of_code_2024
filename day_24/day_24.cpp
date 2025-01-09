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

template <typename T>
struct std::formatter<std::optional<T>> : std::formatter<std::string> {
  template<class FmtContext>
  FmtContext::iterator format(std::optional<T> const& ot, FmtContext& ctx) const {
    if (ot) std::format_to(ctx.out(),"{}",*ot);
    else std::format_to(ctx.out(),"nullopt");
    return ctx.out();
  }
};

struct Gate {
  std::string input1;
  std::string input2;
  std::string output;
  std::string op; // "AND", "OR", "XOR"
  auto operator<=>(const Gate&) const = default;
};

template <>
struct std::formatter<Gate> : std::formatter<std::string> {
  template<class FmtContext>
  FmtContext::iterator format(Gate const& gate, FmtContext& ctx) const {
    std::format_to(ctx.out(),"(setq {} ({} {} {}))",gate.output,gate.op,gate.input1,gate.input2);
    return ctx.out();
  }
};

std::ostream& operator<<(std::ostream& os,Gate const& gate) {
  os << std::format("{}",gate);
  return os;
}

using Gates = std::vector<Gate>;

struct Model {
  WireValues wire_values{};
  Gates gates{};
};

template <>
struct std::formatter<Model> : std::formatter<std::string> {
  template<class FmtContext>
  FmtContext::iterator format(Model const& model, FmtContext& ctx) const {
    std::format_to(ctx.out(),"\n{}\n{}",model.wire_values,model.gates);
    return ctx.out();
  }
};

std::ostream& operator<<(std::ostream& os,Model const& model) {
  os << NL << "Model:";
  os << NL << std::format("{}",model);
  return os;
}

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
        result.wire_values[caption] = (value.trim().str() == "1")?true:false;
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

  auto to_wire_name(std::string const& prefix,int bit_number) {
    return std::format("{}{:02}",prefix,bit_number); // z00 is i = (N-2)
  };

  std::string to_bin_digit_string(char id,WireValues const& wire_vals) {
    std::string result{};
    int i=0;
    while (true) {
      std::string prefix = std::string(1,id);
      auto wire_name = to_wire_name(prefix, i);
      if (not wire_vals.contains(wire_name) or not wire_vals.at(wire_name)) break;
      result.push_back(*wire_vals.at(wire_name)?'1':'0');
      ++i;
    }
    std::reverse(result.begin(), result.end()); // MSB at index 0
    return result;
  }

  bool applyGate(bool val1, bool val2, const std::string& op) {
      if (op == "AND") return val1 and val2;
      if (op == "OR") return val1 or val2;
      if (op == "XOR") return val1 xor val2;
      throw std::invalid_argument("Unknown operation");
  }

  Model to_evaluated(WireValues const& INIT_VALUES, Model const& model) {
    std::cout << NL << "to_evaluated()";
    std::cout << NL << T << "x:" << to_bin_digit_string('x', model.wire_values) << std::flush;
    std::cout << NL << T << "y:" << to_bin_digit_string('y', model.wire_values) << std::flush;

    Model result{INIT_VALUES,model.gates};
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

  std::string adder_eval(std::string const& x,std::string const& y,Gates const& gates) {
    std::cout << NL << "adder_eval";
    std::cout << NL << T << x;
    std::cout << NL << T << y;
    auto const N = x.size();
    WireValues wire_vals{};
    for (int bit = 0;bit<x.size();++bit) {
      auto x_wire_name = to_wire_name("x", bit);
      wire_vals[x_wire_name] = x[N-1-bit]=='1'?true:false; // x[size-1] = bit 0
      auto y_wire_name = to_wire_name("y", bit);
      wire_vals[y_wire_name] = y[N-1-bit]=='1'?true:false;
    }
    Model model{wire_vals,gates};
    auto evaluated_model = to_evaluated(wire_vals, model);
    return to_bin_digit_string('z', evaluated_model.wire_values);
  }

  auto to_int(std::string bin_digits) {
    std::bitset<64> bits{bin_digits};
    return bits.to_ulong();;
  }

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

  std::vector<aoc::raw::Lines> to_examples(aoc::parsing::Sections const& sections) {
    std::vector<aoc::raw::Lines> result{};

    result.push_back({});
    result.back().append_range(aoc::parsing::to_raw(sections[17]));
    result.back().push_back("");
    result.back().append_range(aoc::parsing::to_raw(sections[18]));

    result.push_back({});
    result.back().append_range(aoc::parsing::to_raw(sections[25]));
    result.back().push_back("");
    result.back().append_range(aoc::parsing::to_raw(sections[26]));

    return result;
  }

  bool test0(std::optional<aoc::parsing::Sections> const& sections,Args args) {
    // This function is called by aoc::application if registered with add_test(test::test0)
    // Extract test data from provided sections from the day web page text.
    // See zsh-script pull_text.zsh for support to fetch advent of code day web page text to doc.txt
    std::cout << NL << "test0";
    if (sections) {
      std::cout << NL << T << "sections ok";
      auto examples = to_examples(*sections);
      if (examples.size()>0) {
        std::cout << NL << T << "examples ok";
        auto example_in = aoc::test::to_example_in(examples[0]);
        auto example_model = parse(example_in);
        std::cout << NL << std::format("\n{}",example_model);
        
        auto const INIT_VALUES = example_model.wire_values;
        auto const& [vals,gates] = to_evaluated(INIT_VALUES,example_model);
        auto z_digits = to_bin_digit_string('z',vals);
        std::cout << NL << "zs:" << z_digits.size() << " " << z_digits;
        auto z = to_int(z_digits);
        std::cout << " --> decimal:" << z;
        return z==4;
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
    std::ostringstream response{};
    std::cout << NL << NL << "test";
    if (in) {
      auto model = parse(in);
      auto const INIT_VALUES = model.wire_values;
      auto const& [vals,gates] = to_evaluated(INIT_VALUES,model);
      auto z_digits = to_bin_digit_string('z',vals);
      std::cout << NL << "zs:" << z_digits.size() << " " << z_digits;
      auto z = to_int(z_digits);
      std::cout << " --> decimal:" << z;
      response << z;
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
      auto INIT_VALUES = model.wire_values;
      auto [wire_vals,gates] = test::to_evaluated(INIT_VALUES,model);
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

  std::vector<aoc::raw::Lines> to_examples(aoc::parsing::Sections const& sections) {
    std::vector<aoc::raw::Lines> result{};

    result.push_back({});
    result.back().append_range(aoc::parsing::to_raw(sections[17]));
    result.back().push_back("");
    result.back().append_range(aoc::parsing::to_raw(sections[18]));

    result.push_back({});
    result.back().append_range(aoc::parsing::to_raw(sections[25]));
    result.back().push_back("");
    result.back().append_range(aoc::parsing::to_raw(sections[26]));

    result.push_back({});
    result.back().append_range(aoc::parsing::to_raw(sections[44]));
    result.back().push_back("");
    result.back().append_range(aoc::parsing::to_raw(sections[45]));
    return result;
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
    // Web: 01011010000101000000010011110000100101001010010
    // sum:   011010000101000000010011110000100101001010010
    //   z:  1011010000100111111110011101100101001001010010
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
      if (result != eGate_Undefined) break;
    }
    return result;
  }

  bool is_xy_gate(Gate const& gate) {
    return (is_xy_wire(gate.input1) and is_xy_wire(gate.input2));
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

  // Recursive Pritty Print
  void pritty_print(std::string const& out_wire_name,int level,Gates const& gates,int depth = 4) {
    auto gate = to_gate(out_wire_name, gates);
    std::cout << NL << aoc::raw::Indent(2*level) << gate << " " << to_gate_id(gate, gates);
    if (depth == 0) return;
    if (not is_xy_wire(gate.input1)) pritty_print(gate.input1, level+1, gates,depth-1);
    if (not is_xy_wire(gate.input2)) pritty_print(gate.input2, level+1, gates,depth-1);
  }

  void print_higher(int bit,Model const& model,int depth = 2) {
    auto z_wire = test::to_wire_name("z", bit);
    if (bit > 45 or depth <= 0) return;
    print_higher(bit+1, model,depth-1);
    // unwind
    pritty_print(z_wire, bit, model.gates);
  }

  using Swap = std::pair<std::string,std::string>;
  using Swaps = std::vector<Swap>;

  Model to_swapped(WireValues const& INIT_VALUES, Model const& model,Swap const& swap) {
    Model result{model};
    auto [lhs,rhs] = swap;
    auto lhs_iter = std::find_if(result.gates.begin(), result.gates.end(), [&lhs](Gate const& gate){
      return (gate.output == lhs);
    });
    auto rhs_iter = std::find_if(result.gates.begin(), result.gates.end(), [&rhs](Gate const& gate){
      return (gate.output == rhs);
    });
    if (lhs_iter != result.gates.end() and rhs_iter != result.gates.end()) {
      std::cout << NL << "swap " << *lhs_iter << " <--> " << *rhs_iter;
      std::swap(rhs_iter->output,lhs_iter->output);
      std::cout << NL << "swapped " << *lhs_iter << "  " << *rhs_iter;
    }
    else {
      throw std::runtime_error(std::format(R"(Sorry, failed to swap {} {}. failed to find both in gates)",lhs,rhs));
    }
    result = test::to_evaluated(INIT_VALUES,result);
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
      auto const INIT_VALUES = _in_model.wire_values;
      auto const init_model = test::to_evaluated(INIT_VALUES,_in_model);
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

      // Identify gates
      if (args.options.contains("-to_ids"))
      {
        auto ids = to_identified_gates(init_model.gates);
        return std::format("-to_ids: {} out of {}",ids,init_model.gates.size());
      }
      
      // Pritty Print
      if (args.options.contains("-pp")) {
        if (args.arg.contains("3")) {
          auto bit = args.arg.at("3");
          std::cout << NL << "Pritty Print for z " << bit;
          auto z_gate = to_gate(std::format("z{}",bit), init_model.gates);
          pritty_print(z_gate.output,0,init_model.gates);
        }
        else {
          response << "Sorry, Please provide bit number as 4:th argument";
        }
        return "-pp";
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
                  
        auto& [applied_swaps,model,current_z_bit] = current;
        auto& [wire_vals,gates] = model;

        auto x_digits = test::to_bin_digit_string('x',wire_vals);
        auto y_digits = test::to_bin_digit_string('y',wire_vals);
        auto z_digits = test::to_bin_digit_string('z',wire_vals);
        auto const N = static_cast<int>(z_digits.size()); // N z digits

        struct AdderResult {
          std::string z_digits{};
          std::string s_digits{};
          std::string carry_digits{};
        };
        
        std::pair<AdderResult,AdderResult> odd_even_result{};
        
        {
          std::string x_digits = std::string(y_digits.size(),'0');
          auto const& [s_digits,carry_digits] = to_bitwise_added(x_digits, y_digits);
          auto z_digits = test::adder_eval(x_digits, y_digits, model.gates);
          
          std::cout << NL << " cin: " << carry_digits;
          std::cout << NL << "   x:  " << x_digits;
          std::cout << NL << "+  y:  " << y_digits;
          std::cout << NL << "-------" << std::string(N-1,'-');
          std::cout << NL << "=  s:  " << s_digits;
          std::cout << NL << "   z: " << z_digits;

          odd_even_result.first.z_digits = z_digits;
          odd_even_result.first.s_digits = s_digits;
          odd_even_result.first.carry_digits = carry_digits;
        }
        
        // Find all digit summing that goes wrong
        // digit[N-1] is the rightmost lowest bit
        std::string first_diff_string(N-1,' ');
        for (int i=N-2;i>=0;--i) {
          auto first_s_digit = odd_even_result.first.s_digits[i];
          auto first_z_digit = odd_even_result.first.z_digits[i+1];
          if (first_z_digit != first_s_digit) first_diff_string[i] = '?';
        }
        std::cout << NL << "   d:  " << first_diff_string;
        auto first_diff_count = std::count(first_diff_string.begin(), first_diff_string.end(), '?');
        std::cout << NL << " d count:" << first_diff_count;
        
        {
          auto i = (N-2) - current_z_bit;

          std::string mod_x_digits = std::string(x_digits.size(),'1');
                              
          auto const& [mod_s_digits,mod_carry_digits] = to_bitwise_added(mod_x_digits, y_digits);
          auto mod_z_digits = test::adder_eval(mod_x_digits, y_digits, model.gates);
          
          std::cout << NL << " cin: " << mod_carry_digits;
          std::cout << NL << "  x':  " << mod_x_digits;
          std::cout << NL << "+  y:  " << y_digits;
          std::cout << NL << "-------" << std::string(N-1,'-');
          std::cout << NL << "= s':  " << mod_s_digits;
          std::cout << NL << "  z': " << mod_z_digits;

          
          odd_even_result.second.z_digits = mod_z_digits;
          odd_even_result.second.s_digits = mod_s_digits;
          odd_even_result.second.carry_digits = mod_carry_digits;
                    
        }
        
        // Find all digit summing that goes wrong
        // digit[N-1] is the rightmost lowest bit
        std::string second_diff_string(N-1,' ');
        for (int i=N-2;i>=0;--i) {
          auto second_s_digit = odd_even_result.second.s_digits[i];
          auto second_z_digit = odd_even_result.second.z_digits[i+1];
          if (second_z_digit != second_s_digit) second_diff_string[i] = '?';
        }
        std::cout << NL << "  d':  " << second_diff_string;
        auto second_diff_count = std::count(second_diff_string.begin(), second_diff_string.end(), '?');
        std::cout << NL << "d' count:" << second_diff_count;
        
        if (first_diff_count==0 and second_diff_count==0) {
          found_swaps = current.swaps;
          break;
        }
        if (first_diff_count==second_diff_count and first_diff_count<best) {
          best = first_diff_count;
          applied_swaps=current.swaps;
          using aoc::raw::operator<<;
          std::cout << NL << "new best:" << best << " for swaps:" << current.swaps;
        }
                                              
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
        auto first_s_digit = odd_even_result.first.s_digits[i];
        auto first_z_digit = odd_even_result.first.z_digits[i+1];
        auto second_s_digit = odd_even_result.second.s_digits[i];
        auto second_z_digit = odd_even_result.second.z_digits[i+1];

        
        if (first_z_digit == first_s_digit and second_z_digit == second_s_digit) {
          // OK. try next z-bit
          q.push_front(State{current.swaps,current.model,bit_number+1});
          found_swaps = current.swaps;
        }
        else {
          int backtrack_level{1};
          
          std::cout << NL << NL; // new z-bit

          // For a full bit adder we expect to find gates that takes x and why for current bit number
          if (true) {
            auto iter = gates.begin();
            auto x_wire_name = test::to_wire_name("x", bit_number);
            auto y_wire_name = test::to_wire_name("y", bit_number);

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

          auto wire_name = test::to_wire_name("z", bit_number);
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
                // Recurse one level lhs2
                {
                  auto wire_name = iter->input2;
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

              } // if z is "XOR"
              else {
                std::cout << " --> SWAP this gate with gate that IS an XOR for x ^ y ^ carry_in";
              }
                            
            } // if iter -> z-wire
            else {
              std::cout << " ??";
            }
         
            print_higher(current_z_bit+1, model);
            pritty_print(wire_name, 0, gates);
            std::cout << NL << "Please enter two wires to swap:";
            std::string input{};
            std::getline(std::cin, input);
            auto [lhs,rhs] = aoc::parsing::Splitter(input).split(' ');
            Swap swap{lhs,rhs};
            auto modified_model = to_swapped(INIT_VALUES,model,swap);
            auto modified_swaps = current.swaps;
            modified_swaps.push_back(swap);
            State next{modified_swaps,modified_model,current_z_bit+1};
            q.push_front(next);

          } // if z wire
          std::cout << NL << "q:" << q.size();
          if (false) {
            std::cout << NL << "BREAK after first faulty bit - for test" << std::flush;
            break;
          }
        }
      } // while q
      using aoc::raw::operator<<;
      std::cout << NL << "Found swaps:" << found_swaps; // Found swaps:[{ "qjj","gjc"},{ "z17","wmp"},{ "z26","gvm"},{ "z39","qsb"}]
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
    if (response.str().size()>0) return response.str();
    else return std::nullopt;
  }
}

int main(int argc, char *argv[]) {
  aoc::application app{};
  app.add_to_examples(test::to_examples);
  app.add_test("test0", test::test0);
  app.add_solve_for("test",test::solve_for,"example.txt");
  app.add_solve_for("1",part1::solve_for,"example.txt");
  app.add_solve_for("1",part1::solve_for,"example1.txt");
  app.add_solve_for("1",part1::solve_for,"puzzle.txt");
  app.add_solve_for("2",part2::solve_for,"puzzle.txt");
  app.run(argc, argv);
  app.print_result();
  /*

   Xcode Debug -O2

   >day_24 -all
   
   For my input:
            
   NOTE: User must inspect and enter swaps to fix each faulty z-bit found by program
   NOTE: My Manual input swaps [{ "w"},{ "z17","wmp"},{ "z26","gvm"},{ "z39","qsb"}]
   NOTE: Execution time includes user interaction.
   
   ANSWERS
   duration:0ms answer[part:"test0"] PASSED
   duration:0ms answer[part test in:example.txt] 4
   duration:0ms answer[part 1 in:example.txt] 4
   duration:1ms answer[part 1 in:example1.txt] 2024
   duration:19ms answer[part 1 in:puzzle.txt] 49520947122770
   duration:91254ms answer[part 2 in:puzzle.txt] gjc,gvm,qjj,qsb,wmp,z17,z26,z39
   */

}
