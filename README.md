# advent_of_code_2024
My solutions to 2024 Advent of code

To use the Z-shell scripts.
* call init_day.zsh with the day number and it will copy template code from teh day_0 sub-folder.
* echo the CMake generator name (e.g., 'Xcode') to tool_chain.txt to make day_n/init_tool_chain.zsh to generate the tool chain of choise.
* cd into day sub-folder and execute init_tool_chain.zsh to have CMake generate your tool chain.
* echo your Advent of Code Session Key cookie value into the top-folder file file cookie.txt. This value is used by sub-folder day_n script pull_data.zsh to fetch the inoput data for the day.
* NOTE: pull_data.zsh in day_0 folder will pull data for day 1 (day.txt set to value 1 for test purposes as day_0 has no data from Advent of Code host).
