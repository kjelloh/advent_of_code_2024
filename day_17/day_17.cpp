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
#include <print>

using aoc::raw::NL;
using aoc::raw::T;
using aoc::raw::NT;

using Integer = int64_t; // 16 bit int: 3.27 x 10^4, 32 bit int: 2.14 x 10^9, 64 bit int: 9.22 x 10^18
using Result = std::string;

using Registers = std::map<char,Integer>;
std::ostream& operator<<(std::ostream& os,Registers const& registers) {
  os << " registers:";
  int count{};
  for (auto const& [l,x] : registers) {
    if (count++>0) os << ',';
    os << " reg['" << l << "'] : " << x;
  }
  
  return os;
}

enum Op {
  op_unknown
  ,adv = 0
  ,bxl = 1
  ,bst = 2
  ,jnz = 3
  ,bxc = 4
  ,out = 5
  ,bdv = 6
  ,cdv = 7
  ,op_undefined
};

Op to_op(int op) {return static_cast<Op>(op);}

std::string to_op_name(Op op) {
//  The eight instructions are as follows:
//
  switch (op) {

//  The adv instruction (opcode 0) performs division. The numerator is the value in the A register. The denominator is found by raising 2 to the power of the instruction's combo operand. (So, an operand of 2 would divide A by 4 (2^2); an operand of 5 would divide A by 2^B.) The result of the division operation is truncated to an integer and then written to the A register.
    case adv: return "adv";break;

//
//  The bxl instruction (opcode 1) calculates the bitwise XOR of register B and the instruction's literal operand, then stores the result in register B.
    case bxl: return "bxl";break;
//
//  The bst instruction (opcode 2) calculates the value of its combo operand modulo 8 (thereby keeping only its lowest 3 bits), then writes that value to the B register.
    case bst: return "bst";break;
//
//  The jnz instruction (opcode 3) does nothing if the A register is 0. However, if the A register is not zero, it jumps by setting the instruction pointer to the value of its literal operand; if this instruction jumps, the instruction pointer is not increased by 2 after this instruction.
    case jnz: return "jnz";break;
//
//  The bxc instruction (opcode 4) calculates the bitwise XOR of register B and register C, then stores the result in register B. (For legacy reasons, this instruction reads an operand but ignores it.)
    case bxc: return "bxc";break;
//
//  The out instruction (opcode 5) calculates the value of its combo operand modulo 8, then outputs that value. (If a Memory outputs multiple values, they are separated by commas.)
    case out: return "out";break;
//
//  The bdv instruction (opcode 6) works exactly like the adv instruction except that the result is stored in the B register. (The numerator is still read from the A register.)
    case bdv: return "bdv";break;
//
//  The cdv instruction (opcode 7) works exactly like the adv instruction except that the result is stored in the C register. (The numerator is still read from the A register.)
    case cdv: return "cdv";break;

    default: return "?";break;
  }
}

std::string to_combo_source(int literal) {
  Integer result{};
//    std::cout << " eval(" << combo << ")";
  //    Combo operands 0 through 3 represent literal values 0 through 3.
  //    Combo operand 4 represents the value of register A.
  //    Combo operand 5 represents the value of register B.
  //    Combo operand 6 represents the value of register C.
  //    Combo operand 7 is reserved and will not appear in valid programs.
  switch (literal) {
    case 0:
    case 1:
    case 2:
    case 3: return std::to_string(literal);
    case 4: return "A";
    case 5: return "B";
    case 6: return "C";
    case 7: return "?";
  }
}

std::string to_statement_string(Op const op,int literal) {
  std::ostringstream oss{};
  switch (op) {

//  The adv instruction (opcode 0) performs division. The numerator is the value in the A register. The denominator is found by raising 2 to the power of the instruction's combo operand. (So, an operand of 2 would divide A by 4 (2^2); an operand of 5 would divide A by 2^B.) The result of the division operation is truncated to an integer and then written to the A register.
    case adv: oss << "A = A >> " << to_combo_source(literal);break;

//
//  The bxl instruction (opcode 1) calculates the bitwise XOR of register B and the instruction's literal operand, then stores the result in register B.
    case bxl: oss << "B = B xor " << literal;break;
//
//  The bst instruction (opcode 2) calculates the value of its combo operand modulo 8 (thereby keeping only its lowest 3 bits), then writes that value to the B register.
    case bst: oss << "B = " << to_combo_source(literal) << " % 8";break;
//
//  The jnz instruction (opcode 3) does nothing if the A register is 0. However, if the A register is not zero, it jumps by setting the instruction pointer to the value of its literal operand; if this instruction jumps, the instruction pointer is not increased by 2 after this instruction.
    case jnz: oss << "jnz A " << literal;break;
//
//  The bxc instruction (opcode 4) calculates the bitwise XOR of register B and register C, then stores the result in register B. (For legacy reasons, this instruction reads an operand but ignores it.)
    case bxc: oss << "B = B xor C";break;
//
//  The out instruction (opcode 5) calculates the value of its combo operand modulo 8, then outputs that value. (If a Memory outputs multiple values, they are separated by commas.)
    case out: oss << "out " << to_combo_source(literal) << " % 8";break;
//
//  The bdv instruction (opcode 6) works exactly like the adv instruction except that the result is stored in the B register. (The numerator is still read from the A register.)
    case bdv: oss << "B = A >> " << to_combo_source(literal);break;
//
//  The cdv instruction (opcode 7) works exactly like the adv instruction except that the result is stored in the C register. (The numerator is still read from the A register.)
    case cdv: oss << "C = A >> " << to_combo_source(literal);break;

    default: oss << "?";break;
  }
  return std::format("{} : {}",to_op_name(op),oss.str());
}


using Memory = std::vector<int>;
std::ostream& operator<<(std::ostream& os,Memory const& memory) {
  os << " memory:" << memory.size() << " ";
  for (int i=0;i<memory.size();++i) {
    if (i>0) os << ',';
    os << memory[i];
  }
  return os;
}

std::string to_op_description(Op op) {
//  The eight instructions are as follows:
//
  switch (op) {

//  The adv instruction (opcode 0) performs division. The numerator is the value in the A register. The denominator is found by raising 2 to the power of the instruction's combo operand. (So, an operand of 2 would divide A by 4 (2^2); an operand of 5 would divide A by 2^B.) The result of the division operation is truncated to an integer and then written to the A register.
    case adv: return "The adv instruction (opcode 0) performs division. The numerator is the value in the A register. The denominator is found by raising 2 to the power of the instruction's combo operand. (So, an operand of 2 would divide A by 4 (2^2); an operand of 5 would divide A by 2^B.) The result of the division operation is truncated to an integer and then written to the A register";break;

//
//  The bxl instruction (opcode 1) calculates the bitwise XOR of register B and the instruction's literal operand, then stores the result in register B.
    case bxl: return "The bxl instruction (opcode 1) calculates the bitwise XOR of register B and the instruction's literal operand, then stores the result in register B";break;
//
//  The bst instruction (opcode 2) calculates the value of its combo operand modulo 8 (thereby keeping only its lowest 3 bits), then writes that value to the B register.
    case bst: return "The bst instruction (opcode 2) calculates the value of its combo operand modulo 8 (thereby keeping only its lowest 3 bits), then writes that value to the B register";break;
//
//  The jnz instruction (opcode 3) does nothing if the A register is 0. However, if the A register is not zero, it jumps by setting the instruction pointer to the value of its literal operand; if this instruction jumps, the instruction pointer is not increased by 2 after this instruction.
    case jnz: return "The jnz instruction (opcode 3) does nothing if the A register is 0. However, if the A register is not zero, it jumps by setting the instruction pointer to the value of its literal operand; if this instruction jumps, the instruction pointer is not increased by 2 after this instruction";break;
//
//  The bxc instruction (opcode 4) calculates the bitwise XOR of register B and register C, then stores the result in register B. (For legacy reasons, this instruction reads an operand but ignores it.)
    case bxc: return "The bxc instruction (opcode 4) calculates the bitwise XOR of register B and register C, then stores the result in register B. (For legacy reasons, this instruction reads an operand but ignores it.)";break;
//
//  The out instruction (opcode 5) calculates the value of its combo operand modulo 8, then outputs that value. (If a Memory outputs multiple values, they are separated by commas.)
    case out: return "The out instruction (opcode 5) calculates the value of its combo operand modulo 8, then outputs that value. (If a Memory outputs multiple values, they are separated by commas.)";break;
//
//  The bdv instruction (opcode 6) works exactly like the adv instruction except that the result is stored in the B register. (The numerator is still read from the A register.)
    case bdv: return "The bdv instruction (opcode 6) works exactly like the adv instruction except that the result is stored in the B register. (The numerator is still read from the A register.)";break;
//
//  The cdv instruction (opcode 7) works exactly like the adv instruction except that the result is stored in the C register. (The numerator is still read from the A register.)
    case cdv: return "The cdv instruction (opcode 7) works exactly like the adv instruction except that the result is stored in the C register. (The numerator is still read from the A register.)";break;

    default: return "?";break;
  }
}

struct CPU {
  Registers m_reg{};
  Memory m_mem{};
  Integer ip() {return m_reg['I'];}
  int m_loop_index{0};
  CPU(Registers reg,int ip,Memory const& mem) : m_reg{reg},m_mem{mem} {
    m_reg['I'] = ip;
  }
  
  int next() {
    auto ip = m_reg['I'];
    ++m_reg['I'];
    return m_mem[ip];
  }
  Integer combo(int literal) {
    Integer result{};
//    std::cout << " eval(" << combo << ")";
    //    Combo operands 0 through 3 represent literal values 0 through 3.
    //    Combo operand 4 represents the value of register A.
    //    Combo operand 5 represents the value of register B.
    //    Combo operand 6 represents the value of register C.
    //    Combo operand 7 is reserved and will not appear in valid programs.
    switch (literal) {
      case 0:
      case 1:
      case 2:
      case 3:
        result = literal;
        break;

      case 4:
        result = m_reg['A'];
        break;
      case 5:
        result = m_reg['B'];
        break;
      case 6:
        result = m_reg['C'];
        break;

      case 7:
        result = -1; // signal invalid
    }
    return result;
  }
  std::string operator++() {
    constexpr int N = 32;
    std::string result{};
    if (ip() >= m_mem.size()) return "";
    bool repl{false};
    if (repl) {
      std::print("\n\nn:{}",(8 + m_loop_index));
      std::print("\n   A:{: >32b} : {}",m_reg['A'],m_reg['A']);
      std::print("\n   B:{: >32b} : {}",m_reg['B'],m_reg['B']);
      std::print("\n   C:{: >32b} : {}",m_reg['C'],m_reg['C']);
      std::print("\n    >");
      std::string input{};
      if (std::getline(std::cin, input)) {
        auto tokens = aoc::parsing::Splitter(input).splits(' ');
        if (tokens.size()==3) {
          m_reg['A'] = std::stoi(tokens[0]);
          m_reg['B'] = std::stoi(tokens[1]);
          m_reg['C'] = std::stoi(tokens[2]);
        }
      }
    }
    auto op = to_op(next());
    auto literal = next();
    std::print("\n{}:{}",m_reg['I']-2,to_statement_string(op, literal));
    switch (op) {
        //  The adv instruction (opcode 0) performs division. The numerator is the value in the A register. The denominator is found by raising 2 to the power of the instruction's combo operand. (So, an operand of 2 would divide A by 4 (2^2); an operand of 5 would divide A by 2^B.) The result of the division operation is truncated to an integer and then written to the A register.
      case adv: {
        auto numerator = m_reg['A'];
        auto y = numerator >> combo(literal);
        std::print(" = {:b} >> {:b} = {:b}",numerator,combo(literal),y);
        m_reg['A'] = y;
      } break;
        
        //
        //  The bxl instruction (opcode 1) calculates the bitwise XOR of register B and the instruction's literal operand, then stores the result in register B.
      case bxl: {
        auto y = m_reg['B'] xor literal;
        std::print(" = {:b} xor {:b} = {:b}",m_reg['B'],literal,y);
        m_reg['B'] = y;
      } break;
        //
        //  The bst instruction (opcode 2) calculates the value of its combo operand modulo 8 (thereby keeping only its lowest 3 bits), then writes that value to the B register.
      case bst: {
        auto y = combo(literal) % 8;
        std::print(" = {:b} % 8 = {:0>3b}",combo(literal),y);
        m_reg['B'] = y;
      } break;
        //
        //  The jnz instruction (opcode 3) does nothing if the A register is 0. However, if the A register is not zero, it jumps by setting the instruction pointer to the value of its literal operand; if this instruction jumps, the instruction pointer is not increased by 2 after this instruction.
      case jnz: {
        if (m_reg['A'] > 0) {
          m_reg['I'] = literal;
          std::print(" A = {} != 0, ip = {}",m_reg['A'],literal);
          --m_loop_index; // 0..-8 for my input
        }
        else {
        }
      } break;
        //
        //  The bxc instruction (opcode 4) calculates the bitwise XOR of register B and register C, then stores the result in register B. (For legacy reasons, this instruction reads an operand but ignores it.)
      case bxc: {
        auto y = m_reg['B'] xor m_reg['C'];
        std::print(" = {:b} xor {:b} = {:b}",m_reg['B'],m_reg['C'],y);

        m_reg['B'] = y;
      } break;
        //
        //  The out instruction (opcode 5) calculates the value of its combo operand modulo 8, then outputs that value. (If a Memory outputs multiple values, they are separated by commas.)
      case out: {
        auto y = combo(literal) % 8;
        std::print(" = {:b} % 8 = {:0>3b} ---> {}",combo(literal),y,y);
        result.push_back('0'+y);
      } break;
        //
        //  The bdv instruction (opcode 6) works exactly like the adv instruction except that the result is stored in the B register. (The numerator is still read from the A register.)
      case bdv: {
        auto numerator = m_reg['A'];
        auto y = numerator >> combo(literal);
        std::print(" = {:b} >> {} = {:b}",numerator,combo(literal),y);
        m_reg['B'] = y;
      } break;
        //
        //  The cdv instruction (opcode 7) works exactly like the adv instruction except that the result is stored in the C register. (The numerator is still read from the A register.)
      case cdv: {
        auto numerator = m_reg['A'];
        auto y = numerator >> combo(literal);
        std::print(" = {:b} >> {} = {:b}",numerator,combo(literal),y);
        m_reg['C'] = y;
      } break;
        
      default: {
        std::print(" UNKNOWN OPERATOR - NOP");
      } break;
    }
    return result;
  }
  bool operator==(CPU const& other) const {
    // Eqhaul if all 'our' keys are in other and have the same value
    return std::all_of(m_reg.begin(), m_reg.end(),[&other](auto const& entry){
      if (entry.first == 'I') return true; // Ignore Register 'I'
      if (other.m_reg.contains(entry.first)) {
        return (other.m_reg.at(entry.first) == entry.second);
      }
      return false;
    });
  }
};
std::ostream& operator<<(std::ostream& os,CPU const& cpu) {
  os << "cpu:";
  os << cpu.m_reg;
  return os;
}

struct Computer {
  Memory m_mem;
  CPU m_cpu;
  Computer(Registers reg,Memory const& memory) : m_mem{memory},m_cpu{reg,0,m_mem} {}
  Result run(std::string const& arg={}) {
    Result result{};
    bool pritty_print = arg == "pp";
    std::cout << NL << "run:" << m_cpu << " on " << m_mem;
    if (pritty_print) std::cout << " 'pritty print'";
    while (m_cpu.ip()<m_mem.size()) {
      if (auto output = ++m_cpu;output.size()>0) {
        if (result.size()>0) result.push_back(',');
        result += output;
      }
    }
    return result;
  }
  bool operator==(Computer const& other) const {
    return m_cpu == other.m_cpu;
  }
};

std::ostream& operator<<(std::ostream& os,Computer const& pc) {
  os << "pc:";
  os << pc.m_cpu << pc.m_mem;
  return os;
}

struct Model {
  Registers registers{};
  Memory memory{};
};

struct Program {
  Memory memory;
};

template <>
struct std::formatter<Program> : std::formatter<std::string> {
  using Base = std::formatter<std::string>;

  template<class FmtContext>
  FmtContext::iterator format(const Program& program, FmtContext& ctx) const {
//  auto format(const Program& program, auto& ctx) const {
    auto const& vec = program.memory;
    auto paired_view = std::views::iota(decltype(vec.size() / 2){}, vec.size() / 2)
            | std::views::transform([&vec](auto i) {
              return std::make_tuple(static_cast<Op>(vec[2 * i]), vec[2 * i + 1]);  // Pair consecutive elements
            });
    std::format_to(ctx.out(),"\n<Program>");
    for (auto const& [ip,statement] : aoc::views::enumerate(paired_view)) {
      auto const& [op,literal] = statement;
      std::format_to(ctx.out(),"\n{} : {}",ip,to_statement_string(op,literal));
    }
    return ctx.out();
  }
};

void print_program(Program const& program) {
  std::print(std::cout,"{}",program);
}

std::ostream& operator<<(std::ostream& os,Model const& model) {
  os << "model:" << model.registers << model.memory;
  return os;
}

Model parse(auto& in) {
  using namespace aoc::parsing;
  using aoc::raw::operator<<;
  Model result{};
  auto input = Splitter{in};
  auto sections = input.sections();
  if (sections.size()==2) {
    Registers registers{};
    Memory memory{};
    for (int i=0;i<sections.size();++i) {
      std::cout << NL << "------ section " << i << " -------";
      std::cout << to_raw(sections[i]) << std::flush;
      if (i==0) {
        for (auto const& line : sections[0]) {
          auto const& [left,right] = line.split(':');
          char name = left.str().back();
          auto val = std::stoi(right.trim().str());
          registers[name] = val;
        }
      }
      else {
        auto line = sections[1][0];
        auto const& [left,right] = line.split(':');
        auto program = right.trim().str();
        for (int i=0;i<program.size();++i) {
          if (i%2==0) memory.push_back(program[i]-'0');
        }
      }
    }
    result = Model{registers,memory};
  }
  else {
    std::cerr << "Sorry, Expected two sections but got " << sections.size();
  }
  return result;
}

struct Args {
  std::map<std::string,std::string> arg{};
  std::set<std::string> options{};
  operator bool() const {
    return (arg.size()>0) or (options.size()>0);
  }
};

namespace test {

  // Adapt to expected for day puzzle
  struct LogEntry {
    Computer before;
    std::optional<Computer> after{};
    std::optional<Result> output{};
    bool operator==(LogEntry const& other) const {
      bool result{true};
      result = result and (after and other.after)?*after == *other.after:true;
      result = result and (output and other.output)?(*output==*other.output):true;
      return result;
    }
  };

  std::ostream& operator<<(std::ostream& os,LogEntry const& entry) {
    os << " log entry:";
    os << NL << T << " before:" << entry.before;
    os << NL << T << " after:";
    if (entry.after) os << *entry.after;
    else os << "?";
    os << NL << T << " output:";
    if (entry.output) os << std::quoted(*entry.output);
    else os << "?";
    return os;
  }

  using LogEntries = aoc::test::LogEntries<LogEntry>;

  LogEntries parse0(auto& in) {
    std::cout << NL << T << "test::parse";
    LogEntries result{};
    using namespace aoc::parsing;
    auto input = Splitter{in};
    auto lines = input.lines();
    std::cout << NL << T << lines.size() << " lines";
    for (int i=0;i<lines.size();++i) {
      auto raw_line = to_raw(lines[i]);
      std::cout << NL << T << T << "line[" << i << "]:" << raw_line.size() << " " << std::quoted(raw_line);
      //        If register C contains 9, the program 2,6 would set register B to 1.
      //        If register A contains 10, the program 5,0,5,1,5,4 would output 0,1,2.
      //        If register A contains 2024, the program 0,1,5,4,3,0 would output 4,2,5,6,7,7,7,7,3,1,0 and leave 0 in register A.
      //        If register B contains 29, the program 1,7 would set register B to 26.
      //        If register B contains 2024 and register C contains 43690, the program 4,0 would set register B to 44354

      Registers reg_before{};
      {
        std::regex pattern(R"(register ([A-Z]) contains (\d+))");
        // Create a regex iterator to iterate over all matches
        auto words_begin = std::sregex_iterator(raw_line.begin(), raw_line.end(), pattern);
        auto words_end = std::sregex_iterator();

        // Iterate through the matches and print results
        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
          std::smatch match = *i;
          std::cout << NL << "Match found:";
          std::cout << NL << "Register: " << match[1];
          std::cout << NL << "Value: " << match[2];
          reg_before[match[1].str().front()] = std::stoi(match[2].str());
        }
      }
      Memory memory{};
      {
        std::regex pattern(R"(the program ([\d,]+))");
        std::smatch match{};
        if (std::regex_search(raw_line,match,pattern)) {
          std::string const program = match[1];
          std::cout << NL << "program:" << std::quoted(program);
          for (int i=0;i<program.size();++i) {
            if (i%2==0) memory.push_back(program[i]-'0');
          }
          using ::operator<<; // Bring in for alias Memory (will not trigger ADL to look in global namespace)
          std::cout << NL << "memory:" << memory;
        }
      }
      Registers reg_after{};
      {
        std::regex pattern(R"(would set register ([A-Z]) to (\d+))");
        std::smatch match{};
        if (std::regex_search(raw_line,match,pattern)) {
          std::cout << NL << "Match found:";
          std::cout << NL << "Register: " << match[1];
          std::cout << NL << "Value: " << match[2];
          reg_after[match[1].str().front()] = std::stoi(match[2].str());
        }
      }
      {
        std::regex pattern(R"(leave (\d+) in register ([A-Z]))");
        std::smatch match{};
        if (std::regex_search(raw_line,match,pattern)) {
          std::cout << NL << "Match found:";
          std::cout << NL << "Register: " << match[2];
          std::cout << NL << "Value: " << match[1];
          reg_after[match[2].str().front()] = std::stoi(match[1].str());
        }
      }
      Result output{};
      {
        std::regex pattern(R"(would output ([\d,]+))");
        std::smatch match{};
        if (std::regex_search(raw_line,match,pattern)) {
          output = match[1].str();
          std::cout << NL << "output:" << std::quoted(output);
        }
      }
      Computer before{reg_before,memory};
      Computer after{reg_after,memory};
      LogEntry entry{before};
      if (reg_after.size()>0) entry.after = Computer{reg_after,memory};
      if (output.size()>0) entry.output = output;
      result.push_back(entry);
    }
//    using aoc::test::operator<<;
//    std::cout << NL << "test::parse0:" << result.size() << result;
    return result;
  }

  LogEntry parse1(auto& in,auto& log_in) {
    std::cout << NL << T << "test::parse";
    using namespace aoc::parsing;
    auto model = ::parse(in);
    Computer computer{model.registers,model.memory};
    auto log_input = Splitter{log_in}.trim();
    LogEntry entry{computer,{},to_raw(log_input)};
    return entry;
  }


  std::optional<Result> test0(Args args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "test0";
    std::ifstream log_in{aoc::to_working_dir_path("example0.log")};
    if (false) {
      // If register C contains 9, the Memory 2,6 would set register B to 1.
      Registers reg{{'C',9}};
      Memory memory{2,6};
      Computer pc{reg,memory};
      auto output = pc.run();
    }
    else if (log_in) {
      auto log = test::parse0(log_in);
      bool failed{};
      int count{};
      for (auto const& logged : log) {
        std::cout << NL << "--------------------";
        std::cout << NL << "processing:" << logged;
        Computer pc{logged.before};
        auto output = pc.run();
        LogEntry computed{logged.before,pc,output};
        std::cout << NL << "--------------------";
        std::cout << computed;
        if ((logged.after and computed.after) and  (*logged.after != *computed.after)) {
          std::cout << NL << "logged.after DIFFERS from computed.after - FAILED";
          failed = true;
          break;
        }
        if ((logged.output and computed.output) and (*logged.output != *computed.output)) {
          std::cout << NL << "logged.output DIFFERS from computed.output - FAILED";
          failed = true;
          break;
        }
        std::cout << NL << "logged == computed OK";
      }
      if (failed) result = "FAILED";
      else result = "passed";
    }
    return result;
  }

  std::optional<Result> solve_for(auto& in,Args args) {
    std::optional<Result> result;
    auto part = args.arg["part"];
    if (part == "test0") return test0(args);
    return result;
  }
}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      Computer pc{model.registers,model.memory};
      auto output = pc.run();
      result = output;
    }
    return result;
  }
}

namespace part2 {

  struct State {
    std::size_t digit_ix;
    Integer out_a;
  };
  Integer solve_for(State const& state,Memory const& output,auto const& f) {
    // For end digit A[end] must be 0..7, B and C will come from A
    //  --> one of these A's must create the output output[end]
    // For digit end-1, A[end-1] must be one of A[end]*8 + 0..7
    //  --> one of these A's must create output[end-1]
    
    auto [digit_ix,out_a] = state;
    Integer best{std::numeric_limits<Integer>::max()};


    std::print("\n{}ix:{} in_a:{} target:{}",std::string(2*digit_ix,' '),digit_ix,out_a,output[digit_ix]);

    // For any output[i] the possible A's we have to try is A[i+1]*8 + 0..7
    for (int i=0;i<=7;++i) {
      auto in_a = (out_a << 3) + i;
      if (f(in_a) == output[digit_ix]) {
        if (digit_ix == 0) {
          std::cout << NL << "CANDIDATE A:" << in_a;
          return in_a;
        }
        // recurse
        best = std::min(best,solve_for({digit_ix-1,in_a},output,f));
      }
    }
    return best;
  }

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part2";
    if (in) {
      auto model = parse(in);
      print_program(Program{model.memory});
      Computer pc{model.registers,model.memory};
      auto f = [&program = model.memory](Integer a) -> int {
        Registers registers{};
        registers['A'] = a;
        CPU cpu{registers,0,program};
        while (true) {
          auto output = ++cpu;
          if (output.size()>0) return ('0' - output.back());
        }
      };
      auto a = solve_for({model.memory.size()-1},model.memory,f);
      if (a<std::numeric_limits<Integer>::max()) {
        result = std::to_string(a);
      }
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

    std::vector<std::string> parts = {"test0", "1", "2"};
    std::vector<std::string> files = {"example.txt", "puzzle.txt"};
    
    for (const auto& [part, file] : aoc::algo::cartesian_product(parts, files)) {
      Args args;
      args.arg["part"] = part;
      args.arg["file"] = file;
      requests.push_back(args);
    }
  }

  std::cout << NL << "Arguments:";
  for (auto const& request : requests) {
    auto const& [arg,options] = request;
    using aoc::raw::operator<<;
    std::cout << NL << "  {key,value} : " << arg;
    std::cout << NL << "  options     : " << options;
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
   ./day_07 -all
      
   ANSWERS
   duration:6ms answer[test0 example.txt] passed
   duration:5ms answer[test0 puzzle.txt] passed
   duration:0ms answer[part1 example.txt] 4,6,3,5,6,3,5,2,1,0
   duration:1ms answer[part1 puzzle.txt] 2,1,3,0,5,2,3,7,1
   duration:0ms answer[part2 example.txt] NO OPERATION
   duration:1ms answer[part2 puzzle.txt] NO OPERATION

   */
  return 0;
}
