# advent_of_code_2024
My solutions to 2024 Advent of code

To use the Z-shell scripts.
* call init_day.zsh with the day number and it will copy template code from teh day_0 sub-folder.
* echo the CMake generator name (e.g., 'Xcode') to tool_chain.txt to make day_n/init_tool_chain.zsh to generate the tool chain of choise.
* cd into day sub-folder and execute init_tool_chain.zsh to have CMake generate your tool chain.
* echo your Advent of Code Session Key cookie value into the top-folder file file cookie.txt. This value is used by sub-folder day_n script pull_data.zsh to fetch the inoput data for the day.
* NOTE: pull_data.zsh in day_0 folder will pull data for day 1 (day.txt set to value 1 for test purposes as day_0 has no data from Advent of Code host).

## Traps i fell into solving AoC 2024

### day_2

* In part 2 I first failed to realise that I need to try to remove both the left and right 'level' value for a pair that would cause an UNSAFE report.
* In part 2 I missed to initialise a boolean return value 'result. This caused the return value to always be true and e v e r y report entry was SAFE in my part 2 code!

```cpp
  bool can_be_made_safe(std::vector<Result> report) {
    bool result; // Undefined behaviour = uninitialised!
    ...
    for (int i=0;i<report.size();++i) {
      auto candidate = report;
      candidate.erase(candidate.begin() + i);
      ...
      if (is_safe(candidate)) {
        result = true;
        break;
      }
    }
    return result;
  }

```
### day_3

* In XCode, I missed to have a return statement in a parse() function and got a mysterious 'Thread 1: EXC_BREAKPOINT (code=1, subcode=0x100002694)' at the std::getline(...) call!
```
 Model parse(std::istream& in) {
    std::cout << "\n<BEGIN parse>";
    Model result{};
    std::string line{};
    // StringParser mul_parser("mul(");
    while (std::getline(in,line)) { <== BREAK HERE...
      /*
      if (auto result = mul_parser.parse(line)) {
        auto [thing,tail] = *result;
        std::cout << "\nmul_parser --> [thing:" << thing << ",tail:" << tail << "]";
      }
       */
    }
    // MISSING return statement here
  }
```
NOTE: XCode did NOT warn or signal in any way about the missing return statement.