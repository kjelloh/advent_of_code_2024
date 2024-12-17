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
using Result = std::string;

using Registers = std::map<char,int>;
std::ostream& operator<<(std::ostream& os,Registers const& registers) {
  std::cout << "registers:";
  
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

using Statement = std::pair<Op,int>;
std::ostream& operator<<(std::ostream& os,Statement const& statement) {
  std::cout << "statement:";
  std::cout << "{" << statement.first << ":" << to_op_name(statement.first) << "," << statement.second << "}";
  return os;
}
using Memory = std::vector<int>;
std::ostream& operator<<(std::ostream& os,Memory const& memory) {
  std::cout << "Memory:" << memory.size();
  for (int i=0;i<memory.size();++i) {
    if (i>0) std::cout << ',';
    std::cout << memory[i];
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
  int ip() {return m_reg['I'];}
  CPU(Registers reg,int ip,Memory const& mem) : m_reg{reg},m_mem{mem} {
    m_reg['I'] = ip;
  }
  
  int next() {
    auto ip = m_reg['I'];
    ++m_reg['I'];
    return m_mem[ip];
  }
  int eval(int combo) {
    int result{};
    std::cout << "eval(" << combo << ")";
    //    Combo operands 0 through 3 represent literal values 0 through 3.
    //    Combo operand 4 represents the value of register A.
    //    Combo operand 5 represents the value of register B.
    //    Combo operand 6 represents the value of register C.
    //    Combo operand 7 is reserved and will not appear in valid programs.
    switch (combo) {
      case 0:
      case 1:
      case 2:
      case 3:
        result = combo;
        std::cout << " literal ";
        break;

      case 4:
        result = m_reg['A'];
        std::cout << " 'A' ";
        break;
      case 5:
        result = m_reg['B'];
        std::cout << " 'B' ";
        break;
      case 6:
        result = m_reg['C'];
        std::cout << " 'C' ";
        break;

      case 7:
        throw std::runtime_error("Sorry, combo 7 is not valid");break;
    }
    std::cout << " --> " << result;
    return result;
  }
  std::string operator++() {
    std::string result{};
    if (ip() >= m_mem.size()) return "";
    auto op = to_op(next());
    auto x = next();
    std::cout << NL << "execute:" << op << " " << x;
    auto val = eval(x);
    switch (op) {
        //  The adv instruction (opcode 0) performs division. The numerator is the value in the A register. The denominator is found by raising 2 to the power of the instruction's combo operand. (So, an operand of 2 would divide A by 4 (2^2); an operand of 5 would divide A by 2^B.) The result of the division operation is truncated to an integer and then written to the A register.
      case adv: {
        std::cout << NL << to_op_description(op);
        int numerator = m_reg['A'];
        auto denominator = 1 << val;
        auto y = numerator / denominator;
        m_reg['A'] = y;
        std::cout << NL << T << "m_reg['A'] = " << m_reg['A'];
      } break;
        
        //
        //  The bxl instruction (opcode 1) calculates the bitwise XOR of register B and the instruction's literal operand, then stores the result in register B.
      case bxl: {
        std::cout << NL << to_op_description(op);
        std::cout << NL << T << "NOT YET IMPLEMENTED";
      } break;
        //
        //  The bst instruction (opcode 2) calculates the value of its combo operand modulo 8 (thereby keeping only its lowest 3 bits), then writes that value to the B register.
      case bst: {
        std::cout << NL << to_op_description(op);
        auto y = val % 8;
        m_reg['B'] = y;
        std::cout << NL << T << "m_reg['B'] = " << m_reg['B'];
      } break;
        //
        //  The jnz instruction (opcode 3) does nothing if the A register is 0. However, if the A register is not zero, it jumps by setting the instruction pointer to the value of its literal operand; if this instruction jumps, the instruction pointer is not increased by 2 after this instruction.
      case jnz: {
        std::cout << NL << to_op_description(op);
        std::cout << NL << T << "NOT YET IMPLEMENTED";
      } break;
        //
        //  The bxc instruction (opcode 4) calculates the bitwise XOR of register B and register C, then stores the result in register B. (For legacy reasons, this instruction reads an operand but ignores it.)
      case bxc: {
        std::cout << NL << to_op_description(op);
        std::cout << NL << T << "NOT YET IMPLEMENTED";
      } break;
        //
        //  The out instruction (opcode 5) calculates the value of its combo operand modulo 8, then outputs that value. (If a Memory outputs multiple values, they are separated by commas.)
      case out: {
        std::cout << NL << to_op_description(op);
        std::cout << NL << T << "NOT YET IMPLEMENTED";
      } break;
        //
        //  The bdv instruction (opcode 6) works exactly like the adv instruction except that the result is stored in the B register. (The numerator is still read from the A register.)
      case bdv: {
        std::cout << NL << to_op_description(op);
        std::cout << NL << T << "NOT YET IMPLEMENTED";
      } break;
        //
        //  The cdv instruction (opcode 7) works exactly like the adv instruction except that the result is stored in the C register. (The numerator is still read from the A register.)
      case cdv: {
        std::cout << NL << to_op_description(op);
        std::cout << NL << T << "NOT YET IMPLEMENTED";
      } break;
        
      default: {
        std::cout << NL << T << "UNKNOWN OPERATOR - NOP";
      } break;
    }
    return result;
  }
  bool operator==(CPU const& other) const {
    return (m_reg == other.m_reg);
  }
};
std::ostream& operator<<(std::ostream& os,CPU const& cpu) {
  std::cout << "cpu:";
  std::cout << cpu.m_reg;
  return os;
}

struct Computer {
  Memory m_mem;
  CPU m_cpu;
  Computer(Registers reg,Memory const& memory) : m_mem{memory},m_cpu{reg,0,m_mem} {}
  Result run() {
    Result result{};
    std::cout << NL << "run:" << m_cpu << " on " << m_mem;
    while (m_cpu.ip()<m_mem.size()) {
      result += ++m_cpu;
    }
    return result;
  }
  bool operator==(Computer const& other) const = default;
};

std::ostream& operator<<(std::ostream& os,Computer const& pc) {
  std::cout << "pc:";
  std::cout << pc.m_cpu << pc.m_mem;
  return os;
}

struct Model {
  Registers registers{};
  Memory memory{};
};

std::ostream& operator<<(std::ostream& os,Model const& model) {
  std::cout << "model:" << model.registers << model.memory;
  return os;
}

Model parse(auto& in) {
  using namespace aoc::parsing;
  using aoc::raw::operator<<;
  Model result{};
  auto input = Splitter{in};
  auto sections = input.sections();
  if (sections.size()==2) {
    for (int i=0;i<sections.size();++i) {
      std::cout << NL << "------ section " << i << " -------";
      auto const& section = sections[i];
      std::cout << to_raw(section);
    }
  }
  else {
    std::cerr << "Sorry, Expected two sections but got " << sections.size();
  }
  return result;
}

using Args = std::vector<std::string>;

namespace test {

  // Adapt to expected for day puzzle
  struct LogEntry {
    Computer pc;
    std::optional<Result> output{};
    bool operator==(LogEntry const& other) const {
      bool result{true};
      result = result and (pc == other.pc);
      result = result and (output and other.output)?(*output==*other.output):true;
      return result;
    }
  };

  std::ostream& operator<<(std::ostream& os,LogEntry const& entry) {
    std::cout << NL << "log entry:";
    std::cout << entry.pc;
    if (entry.output) std::cout << *entry.output;
    else std::cout << "?";
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
      auto line = lines[i];
      std::cout << NL << T << T << "line[" << i << "]:" << line.size() << " " << std::quoted(line.str());
    }
    return result;
  }

  LogEntries parse1(auto& in) {
    std::cout << NL << T << "test::parse";
    LogEntries result{};
    using namespace aoc::parsing;
    auto input = Splitter{in};
    auto lines = input.lines();
    std::cout << NL << T << lines.size() << " lines";
    for (int i=0;i<lines.size();++i) {
      auto line = lines[i];
      std::cout << NL << T << T << "line[" << i << "]:" << line.size() << " " << std::quoted(line.str());
    }
    return result;
  }


  std::optional<Result> test0(Args args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "test0";
    std::ifstream log_in{aoc::to_working_dir_path("example0.log")};
    if (true) {
      // If register C contains 9, the Memory 2,6 would set register B to 1.
      Registers reg{{'C',9}};
      Memory memory{2,6};
      Computer pc{reg,memory};
      auto output = pc.run();
    }
    else if (log_in) {
      auto model = test::parse0(log_in);
    }
    return result;
  }

  std::optional<Result> test1(auto& in, auto& log_in,Args args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "test1";
    if (in) {
      auto model = ::parse(in);
      if (log_in) {
        auto log = test::parse1(log_in);
      }
    }
    return result;
  }

}

namespace part1 {
  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
    }
    return result;
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
int main(int argc, char *argv[]) {
  Args args{};
  for (int i=0;i<argc;++i) {
    args.push_back(argv[i]);
  }

  Answers answers{};
  std::vector<std::chrono::time_point<std::chrono::system_clock>> exec_times{};
  exec_times.push_back(std::chrono::system_clock::now());
  std::vector<int> states = {0};
  for (auto state : states) {
    switch (state) {
      case 0: {
        answers.push_back({"test0",test::test0(args)});
      } break;
      case 111: {
        auto log_file = aoc::to_working_dir_path("example.log");
        std::ifstream log_in{log_file};
        auto file = aoc::to_working_dir_path("example.txt");
        std::ifstream in{file};
        if (in and log_in) answers.push_back({"Part 1 Test Example vs Log",test::test1(in,log_in,args)});
        else std::cerr << "\nSORRY, no file " << file << " or log_file " << log_file;
      } break;
      case 11: {
        auto file = aoc::to_working_dir_path("example.txt");
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1 Example",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
      } break;
      case 10: {
        auto file = aoc::to_working_dir_path("puzzle.txt");
        std::ifstream in{file};
        if (in) answers.push_back({"Part 1     ",part1::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
      } break;
      case 21: {
        auto file = aoc::to_working_dir_path("example.txt");
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2 Example",part2::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
      } break;
      case 20: {
        auto file = aoc::to_working_dir_path("puzzle.txt");
        std::ifstream in{file};
        if (in) answers.push_back({"Part 2     ",part2::solve_for(in,args)});
        else std::cerr << "\nSORRY, no file " << file;
      } break;
      default:{std::cerr << "\nSORRY, no action for state " << state;} break;
    }
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
   For my input:

   ANSWERS
   ...
      
  */
  return 0;
}
