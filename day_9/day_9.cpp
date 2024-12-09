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

namespace parsing {
  class Splitter {
  public:
    Splitter(std::string const& s) : m_s{s} {}
    Splitter(std::istream& is) : m_s{
       std::istreambuf_iterator<char>(is)
      ,std::istreambuf_iterator<char>()
    } {};
    std::vector<Splitter> lines() const {
      std::vector<Splitter> result{};
      std::istringstream is{m_s};
      std::string line{};
      while (std::getline(is,line)) result.push_back(line);
      return result;
    }
    std::pair<Splitter,Splitter> split(char ch) const {
      std::size_t pos = m_s.find(ch);
      if (pos == std::string::npos) {
        return {m_s, std::string{}};
      }
      return {m_s.substr(0, pos), m_s.substr(pos + 1)};
    }
    std::vector<Splitter> splits(char sep) const {
      std::vector<Splitter> result{};
      std::istringstream is{m_s};
      std::string line{};
      while (std::getline(is, line, sep)) {
        result.push_back(line);
      }
      return result;
    }
    Splitter trim() {
        auto start = std::find_if_not(m_s.begin(), m_s.end(), ::isspace);
        auto end = std::find_if_not(m_s.rbegin(), m_s.rend(), ::isspace).base();
        return std::string(start, end);
    }
    std::vector<Splitter> groups(std::string const& regexPattern) {
        std::vector<Splitter> result;
        std::regex pattern(regexPattern);
        std::smatch matches;

        if (std::regex_search(m_s, matches, pattern)) {
            // Iterate over the captured groups (start from 1, as 0 is the whole match)
            for (size_t i = 1; i < matches.size(); ++i) {
                result.push_back(matches[i].str());
            }
        }
        return result;
    }

    std::string const& str() const {return m_s;}
    operator std::string() const {return m_s;}
    auto size() const {return m_s.size();}
  private:
    std::string m_s{};
  };
}

using DiskMap = std::map<int,std::pair<int,int>>;
using Model = DiskMap;

Model parse(auto& in) {
  using namespace parsing;
  Model result{};
  auto input = Splitter{in}.trim();
  std::cout << "input:" << input.size() << " " << std::quoted(input.str());
  for (int i=0;i<input.size();i+=2) {
    std::cout << NL << T << "i:" << i;
    int id = i/2;
    int b = input.str()[i]-'0';
    int f = ((i+1)<input.size())?input.str()[i+1]-'0':0;
    std::cout << " file:" << id << " count:" << b << " free:" << f;
    result[id] = {b,f};
  }
  return result;
}

std::ostream& operator<<(std::ostream& os,DiskMap const& disk_map) {
  for (auto const& [id,pair] : disk_map) {
    os << NL << T << "map[" << id << "] = {count:" << pair.first << ",free:" << pair.second << "}";
  }
  return os;
}

using Args = std::vector<std::string>;

struct Block {
  int id{}; // file id = 0..n or -1 for free block
  int count{};
};
using Blocks = std::vector<Block>;

Blocks to_blocks(Model const& model) {
  Blocks blocks{};
  for (auto const& [id,p] : model) {
    auto const& [count,free] = p;
    blocks.push_back({id,count});
    blocks.push_back({-1,free});
  }
  return blocks;
}

std::ostream& operator<<(std::ostream& os,Block const& block) {
  os << "{id:" << block.id << ",count:" << block.count << "}";
  return os;
}

std::ostream& operator<<(std::ostream& os,Blocks const& blocks) {
  os << "blocks:";
  for (auto const& block : blocks) {
    os << NL << T << block;
  }
  return os;
}

auto to_checksum(Blocks const& compressed){
  Result acc{};
  int pos{};
  for (int i=0;i<compressed.size();++i) {
    auto const& b = compressed[i];
    for (int j=0;j<b.count;++j) {
      if (b.id>=0) {
        std::cout << NL << T << pos << " * " << b.id;
        std::cout << " = " << pos * b.id;
        acc += pos * b.id;
        std::cout << " acc:" << acc;
      }
      ++pos;
    }
  }
  return acc;
};

class Compressor {
private:
  Blocks compressed;
  bool m_is_part_2{};
public:
  
  Compressor(Blocks const& blocks,bool is_part_2)
    :  compressed{blocks}
      ,m_is_part_2(is_part_2) {}

  // part 1
  auto done(auto lix,auto rix){
    return (lix>=rix);
  };

  auto done(auto lix,auto rix,bool is_part_2){
    // rix past the first empty
    auto iter = std::find_if(compressed.begin(),compressed.end(),[](Block const& block){
      return block.id<0; // first empty
    });
    return (iter > compressed.begin()+rix);
//    return (rix < 1); Crude but works
  };
  
  auto valid_lix(auto lix,auto rix) {
    return (lix < rix);
  };

  bool left_fits_right(auto lix, auto rix) {
    auto candidate = compressed[lix];
    return (candidate.id < 0 and candidate.count >= compressed[rix].count);
  }

  
  // part 1 and 2
  auto next_free_segment(auto lix, auto rix) {
    lix = (m_is_part_2)?0:lix;
    while (    valid_lix(lix,rix)
           and (   (compressed[lix].id >= 0)
                or (m_is_part_2 and not left_fits_right(lix,rix))
               )
           ) {
      // std::cout << NL << T << "skipped lix:" << lix << "left:" << compressed[lix];
      ++lix;
    }
    return lix;
  };
  
  // part 1
  auto next_file_segment(auto lix, auto rix){
    while (lix < rix and compressed[rix].id < 0) {
      std::cout << NL << T << "skipped right:" << compressed[rix];
      --rix;
    }
    return rix;
  };
      
  // part 1
  // TODO: Refactor into a part 1 and 2 version?
  auto operator()() {
    auto lix = decltype(compressed.size()){};
    auto rix = compressed.size()-1;
    
    while (true) {
      std::cout << NL << "processing [lix:" << lix << ",rix:" << rix << "] = left:" << compressed[lix] << " right:" << compressed[rix];
      
      lix = next_free_segment(lix,rix);
      rix = next_file_segment(lix, rix);
      
      if (done(lix,rix)) break;
      
      std::cout << NL << T << "at[lix:" << lix << ",rix:" << rix << "] = left:" << compressed[lix] << " right:" << compressed[rix];
      
      // Move data in right to free in left(s)
      auto to_move = std::min(compressed[lix].count,compressed[rix].count);
      
      auto diff = compressed[lix].count - compressed[rix].count;

      if (diff > 0) {
        // Spare room in left free block
        // Split to accomodate for new left over free space
        auto to_move = compressed[rix].count; // empty right data
        Block new_free{-1,diff};
        std::cout << NL << T << compressed[lix] << " <-- " << to_move << " blocks <-- new left:" << new_free << " <-- " << compressed[rix];
        compressed[lix].id = compressed[rix].id;
        compressed[lix].count = to_move;
        compressed[rix].id = -1; // now free
        compressed.insert(compressed.begin()+lix+1,new_free); // spare
        // NOTE: Extended vector invalidates rix (all right of lix right shifted)
        //       But rix will now then just ref to prev in list ok
      }
      else if (diff < 0) {
        // right data > left free
        // split right block by inserting left over data before right block
        auto to_move = compressed[lix].count; // fill left free
        std::cout << NL << T << compressed[lix] << " <-- " << to_move << " blocks <-- right:" << compressed[rix];
        compressed[lix].id = compressed[rix].id;
        compressed[lix].count = to_move;
        compressed[rix].count -= to_move;
      }
      else {
        // same size
        std::cout << NL << T << compressed[lix] << " <-- " << to_move << " blocks <-- " << compressed[rix];
        compressed[lix].id = compressed[rix].id;
        compressed[lix].count = compressed[rix].count;
        compressed[rix].id = -1; // now free
      }
    }
    return compressed;
  };
  
  // part 2
  // TODO: Refactor into a part 1 and 2 version?
  auto operator()(bool is_part_2){
    auto lix = decltype(compressed.size()){};
    auto rix = compressed.size()-1;
    
    while (true) {
            
      if (done(lix,rix,m_is_part_2)) break;
      
      std::cout << NL << "processing [lix:" << lix << ",rix:" << rix << "] = left:" << compressed[lix] << " right:" << compressed[rix];

      rix = next_file_segment(lix, rix);
      lix = next_free_segment(lix, rix);
            
      if (not valid_lix(lix,rix)) {
        std::cout << NL << T << "No room to move " << compressed[rix];
        --rix;
        continue; // try to move next rix
      }

      if (done(lix,rix,m_is_part_2)) break;

      std::cout << NL << T << "at[lix:" << lix << ",rix:" << rix << "] = left:" << compressed[lix] << " right:" << compressed[rix];
      
      // Move data in right to free in left(s)
      auto to_move = std::min(compressed[lix].count,compressed[rix].count);
      auto diff = compressed[lix].count - compressed[rix].count;

      if (diff > 0) {
        // Spare room in left free block
        // Split to accomodate left over free space
        auto to_move = compressed[rix].count; // empty right data
        Block new_free{-1,diff};
        std::cout << NL << T << compressed[lix] << " <-- " << to_move << " blocks <-- new left:" << new_free << " <-- " << compressed[rix];
        compressed[lix].id = compressed[rix].id;
        std::cout << " compressed[lix].id = " << compressed[lix].id;
        compressed[lix].count = to_move;
        compressed[rix].id = -1; // now free
        compressed.insert(compressed.begin()+lix+1,new_free); // spare
        ++rix; // compensate for extended vector
      }
      else if (diff < 0) {
        // file blocks does not fit in free
      }
      else {
        // same size
        std::cout << NL << T << compressed[lix] << " <-- " << to_move << " blocks <-- " << compressed[rix];
        compressed[lix].id = compressed[rix].id;
        compressed[lix].count = compressed[rix].count;
        compressed[rix].id = -1; // now free
      }
    }
    return compressed;
  };

};

namespace part1 {

  std::optional<Result> solve_for(std::istream& in,Args const& args) {
    std::optional<Result> result{};
    std::cout << NL << NL << "part1";
    if (in) {
      auto model = parse(in);
      std::cout << NL << NL << "<model>" << model;
      auto blocks = to_blocks(model);
      std::cout << NL << blocks;
      auto compressed = Compressor{blocks,false}();
      result = to_checksum(compressed);
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
      std::cout << NL << NL << "<model>" << model;
      auto blocks = to_blocks(model);
      std::cout << NL << blocks;
      auto compressed = Compressor{blocks,true}(true);
      std::cout << NL << "compressed:" << compressed;
      result = to_checksum(compressed);
    }
    //
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
//  std::vector<int> states = {0};
//  std::vector<int> states = {0,1};
//  std::vector<int> states = {2};
//  std::vector<int> states = {2,3};
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
   duration:1ms answer[Part 1 Example] 1928
   duration:266ms answer[Part 1     ] 6390180901651
   duration:0ms answer[Part 2 Example] 2858
   duration:695ms answer[Part 2     ] 6412390114238
   
  */
  return 0;
}
