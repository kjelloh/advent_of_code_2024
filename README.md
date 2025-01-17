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

## day_11
* I got compiler error 'Non-constant-expression cannot be narrowed from type 'int' to 'char' in initializer list' on:
```cpp
std::string indent{remaining_blinks,' '};
```
I must confess I aim at always use the {} initialisation but has yet to fully realise when the compiler will be able to map this to the constructor of choise.

In this case it seems the compiler fails ti map an {int,char} to the std::string(size,char) constructor?

In case case, one fix is no NOT use brace-initialisation by apply the contructor directly as:
```cpp
std::string indent(remaining_blinks,' ');
```
## day 15
* The code:
```cpp
    std::set<Object> pushed{};
    for (auto const& [pos,object] : objects) {
      if (does_overlap(object.aabb, moved)) pushed.insert(object);
    }
```
caused compiler error:
```
/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX15.2.sdk/usr/include/c++/v1/__functional/operations.h:358:16 Invalid operands to binary expression ('const part2::Object' and 'const part2::Object')
```
A very common tale tail but I forgot that this is due to my struct Object lacking an operator<! std::set requires objetcs to be inserted to be compearable with less-than.
## day_24
* I got a refernce to a discarded object without seeing it happen in:
```cpp
auto current = q.front();
```
Because q.front() is a reference to the element on the stack, thus current will also be a reference!
I can't safelly use auto here but must explcitly state the type I want to mitigate the reference to pass through.
```cpp
State current = q.front(); // ensure a copy is made
```