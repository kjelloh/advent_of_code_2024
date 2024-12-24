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
using Result = aoc::raw::Line;

using WireValues = std::map<std::string,std::optional<bool>>;
struct Operation {
  std::string input1;
  std::string input2;
  std::string output;
  std::string op; // "AND", "OR", "XOR"
  auto operator<=>(const Operation&) const = default;
};
using Operations = std::vector<Operation>;

struct Model {
  WireValues init_values{};
  Operations ops{};
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
        result.ops.push_back({tokens[0],tokens[2],tokens[4],tokens[1]});
      }
      else {
        std::cerr << NL << "Sorry, Parse ERROR: More than two section is unexpected";
      }
    }
  }
  return result;
}

struct Args {
  std::map<std::string,std::string> arg{};
  std::set<std::string> options{};
};

namespace test {

  std::optional<bool> applyOperation(int val1, int val2, const std::string& op) {
      if (op == "AND") return val1 and val2;
      if (op == "OR") return val1 or val2;
      if (op == "XOR") return val1 xor val2;
      throw std::invalid_argument("Unknown operation");
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

  void create_example_file(aoc::raw::Lines const& lines) {
    auto example_file = aoc::to_working_dir_path("example.txt");
    std::ofstream out{example_file};
    if (out) {
      for (auto const& [lx,line] : aoc::views::enumerate(lines)) {
        if (lx>0) out << NL;
        out << line;
      }
      std::cout << NL << "Created " << example_file;
    }
    else {
      std::cerr << NL << "Sorry, failed to create file " << example_file;
    }
  }

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "test";
    if (in) {
      auto model = parse(in);
      auto doc = parse_doc(args);
      auto example = to_example(doc);
      if (args.options.contains("-to_example")) {
        create_example_file(example);
      }
      else {
        auto ops = model.ops;
        auto wire_vals = model.init_values;
        {
          WireValues has_val{};
          for (auto const& [wire,val] : wire_vals) {
            if (val) has_val[wire] = val;
          }
          std::deque<Operation> q{ops.begin(),ops.end()};
          while (not q.empty()) {
            auto [a,b,out,op] = q.front();
            q.pop_front();
            if (has_val[a] and has_val[b]) {
              has_val[out] = applyOperation(*has_val[a], *has_val[b], op);
            }
            else {
              q.push_back({a,b,out,op});
            }
          }
          wire_vals = has_val;
        }

        std::string z_digits{};
        for (const auto& [wire, value] : wire_vals) {
          std::cout << NL << wire << ": " << *value;
          if (wire.starts_with('z')) z_digits.push_back(*value?'1':'0');
        }
        std::reverse(z_digits.begin(), z_digits.end());
        std::cout << NL << "zs:" << z_digits.size() << " " << z_digits;
        std::bitset<64> bits{z_digits};
        auto z = bits.to_ulong();;
        std::cout << " --> decimal:" << z;
        result = std::to_string(z);
        
        
        // zs:0011111101000
        //    0011111101000
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

  // Function to apply swaps based on pairs of indices
  void applySwaps(std::vector<int>& z,Swaps const& swaps) {
    for (const auto& [i, j] : swaps) {
      std::swap(z[i], z[j]);
    }
  }

  // Undo swaps based on pairs of indices
  void undoSwaps(std::vector<int>& z,Swaps const& swaps) {
    applySwaps(z, swaps); // Swapping twice undoes the swap
  }

  std::tuple<std::vector<int>,std::vector<int>,std::vector<int>> to_xyz(WireValues const& wv) {
    return {{},{},{}};
  }

  // Main function to find swaps
  Swaps findZSwaps(WireValues const& wire_vals) {

    auto [x,y,z] = to_xyz(wire_vals);
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
        
        // Apply swaps
        applySwaps(z, swaps);
        
        // Check if the sum constraint is satisfied
        if (checkSum(x, y, z)) {
          return swaps; // Return the valid swaps
        }
        
        // Undo swaps
        undoSwaps(z, swaps);
        
      } while (std::next_permutation(selectedWires.begin(), selectedWires.end()));
      
    } while (std::next_permutation(selectionMask.begin(), selectionMask.end()));
    
    return {}; // Return empty vector if no solution is found
  }

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part2";
    if (in) {
      auto model = parse(in);
      
      std::vector<std::string> index2wire{}; // Mapping index to wire
      std::map<std::string,int> wire2index{}; // mapping wire to index
      std::vector<int> ix{}; // index to x-wires in order
      std::vector<int> iy{}; // index to y-wires in order
      std::vector<int> iz{}; // index to z-wires in order
      std::vector<int> iall{}; // index to all wires in order
      std::vector<int> vals{}; // vals[index] = value for wire with index

      auto wire_vals = model.init_values;
      for (auto const& [in1,in2,out,op] : model.ops) {
        // Ensure there is an entry in the map
        // std::map will order them alphabetically (string key order)
        wire_vals[in1];
        wire_vals[in2];
        wire_vals[out];
      }

      for (auto const& [wire,val] : wire_vals) {
        if (not wire2index.contains(wire)) {
          int index = static_cast<int>(index2wire.size());
          index2wire.push_back(wire);
          wire2index[wire] = index;
          iall.push_back(index);
          
          if (wire.starts_with('x')) {
            ix.push_back(index);
          }
          else if (wire.starts_with('y')) {
            iy.push_back(index);
          }
          else if (wire.starts_with('z')) {
            iz.push_back(index);
          }
        }
      }
      
      // Transform each op to the index world
      struct IndexedOp {
        int in1_ix;
        int in2_ix;
        int out_ix;
        std::array<int,4> cached{}; // 00,01,10,11 to value
        int operator()(bool in1_val,bool in2_val) {
          return cached[2*in1_val+in2_val];
        }
      };

      // We need to be able to evaluate op[w]({u,v})  where u,v,w are indecies
      // We need to map each op to an index w
      std::map<Operation,int> op2index{};
      std::vector<Operation> index2op{};
      std::vector<IndexedOp> iops{};
      
      auto to_cached = [](std::string const& op) -> std::array<int,4> {
        if (op == "AND") {
          //      00 01 10 11
          return {0  ,0 ,0 ,1};
        }
        else if (op == "OR") {
          //      00 01 10 11
          return {0 ,1 ,1 ,1};
        }
        else if (op == "XOR") {
          //      00 01 10 11
          return {0 ,1 ,1 ,0};
        }
      };

      for (auto const& op : model.ops) {
        if (op2index.contains(op)) continue;
        auto index = static_cast<int>(index2op.size());
        index2op.push_back(op);
        op2index[op] = index;
        // Create the indexed op
        auto const& [input1,input2,output,operation] = op;
        auto in1 = wire2index[input1];
        auto in2 = wire2index[input2];
        auto out = wire2index[output];
        auto cached = to_cached(operation);
        IndexedOp iop{in1,in2,out,cached};
        iops.push_back(iop);
      }
      
      if (true) {
        // Log
        for (const auto& [name, index] : wire2index) {
          std::cout << NL << name << " -> " << index;
        }
        
        std::cout << NL << "\nX-Wires (ix): ";
        for (int index : ix) std::cout << index << " ";
        std::cout << "\nY-Wires (iy): ";
        for (int index : iy) std::cout << index << " ";
        std::cout << "\nZ-Wires (iz): ";
        for (int index : iz) std::cout << index << " ";
      }
            
      // eval network of gates
      std::vector<Operation> ordered_ops{};
      {
        auto const& ops = model.ops;
        WireValues has_val{};
        for (auto const& [wire,val] : wire_vals) {
          if (val) has_val[wire] = val;
        }
        std::deque<Operation> q{ops.begin(),ops.end()};
        while (not q.empty()) {
          auto current = q.front();
          auto const& [a,b,out,op] = current;
          q.pop_front();
          if (has_val[a] and has_val[b]) {
            has_val[out] = test::applyOperation(*has_val[a], *has_val[b], op);
            ordered_ops.push_back(current);
            auto u = wire2index[a];
            auto v = wire2index[b];
            auto w = wire2index[out];
            vals[u] = *has_val[a];
            vals[v] = *has_val[b];
            vals[w] = *has_val[out];
            auto op_ix = op2index[current];
            iops[op_ix] = {u,v,w,to_cached(op)};
          }
          else {
            q.push_back({a,b,out,op});
          }
        }
      }
      
      // vals[wire_index] are now the wire value of wire_index = wire2index[wire]
      // iops[op_index] are now the gate operation in the index world of op_index = op2index[operation]
      
      // Now I need to keep track of the index to the operations with a z output!
                  
      // find 4 pairs of z:s to swap fro x+y=z
      while (true) {
        break;
      }
      
      Swaps swaps{};
      if (!swaps.empty()) {
        std::cout << "Found swaps:" << std::endl;
        for (const auto& [i, j] : swaps) {
          std::cout << NL << "Swap z[" << i << "] with z[" << j << "]";
        }
      } else {
        std::cout << NL << "No valid swaps found!";
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
  
  user_args.arg["part"] = "test";
  user_args.arg["file"] = "doc.txt";

  // Override by any user input
  for (int i=1;i<argc;++i) {
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
  
  if (user_args.options.contains("-all")) {
    requests.clear();
    
    for (int i=0;i<4;++i) {
      Args args{};
      std::string part{};
      std::string file{};
      part = (i/2==0)?"1":"2";
      file = (i%2==0)?"example.txt":"puzzle.txt";
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
