Apr 1:
* cleaned up test code a tiny bit
* fixed broken tests
* added tests 24-29 (they write and check values differently (like the sample test) and realloc several times) - written by Bahaa
* added comments on tests 24-29 to elaborate on what's being tested
* increase valgrind timeout to 10 seconds since the later tests take more time to complete since they do a bunch of extra runs
* added checks to make sure that files that aren't supposed to be touched aren't touched (like no Overflow when testing fine w/o overflow, and no extraneous ID files for threads that are not launched)
* tester now says all tests pass if they all pass
* fixed a bug where the Overflow and Id files are not removed if a testcase was ran successfully
* tests 26-29: removed uninitialized mutex; pass is now incremented with atomic adds
* added --error-exitcode=2 to Valgrind; testcases will now fail if Valgrind reported memory errors
* change GCC's `-g` option to `-gdwarf-4` due to some weird issues with DWARF data compression when ran on Lectura

Apr 2:
* fixed a bug that prevents the script from correctly detecting if `timeout` or `valgrind` is not installed
* sequential & coarse-grained testcases no longer look for the "Overflow" file
* tests 11-12 removed

Apr 3:
* testcases are now allowed to execute without `valgrind`

Apr 5:
* fixed a typo in test 9