#! /bin/bash
# This grading script contains parts from Russ' 252 grading script

# Make sure that you've installed the "timeout" tool - 
# to catch infinite loops and deadlocks
if [[ $(which timeout 2>/dev/null) = "" ]]
then
    echo "The command 'timeout' is not installed on this system, the grading script will not work."
    exit 1
fi

valgrind_path=$(which valgrind 2>/dev/null)
if [[ valgrind_path = "" ]]
then
    echo "The command 'valgrind' is not installed on this system, the grading script will not work."
    echo "(Alternatively, you may change this script to disable valgrind altogether)."
    exit 1
fi

# Compile myMalloc and myMalloc-helper
make myMalloc.o myMalloc-helper.o

if [ $? -ne 0 ]
then
    echo "make failed. See GCC error messages above."
    exit 1
fi

# Run testcases
fail_list=""
for testcase in $(ls test-*.c)
do
    # Compile and run testcases here    
    base=$(echo $testcase | cut -f1 -d'.')

    gcc -pthread -gdwarf-4 -o $base myMalloc.o myMalloc-helper.o $testcase
    if [ $? -ne 0 ]
    then
        echo "*** $(testcase) failed to compile. ***" 1>&2
        echo
        fail_list="$fail_list $testcase"
        continue
    fi

    # Increase this duration if the tester is killing the process prematurely.
    timeout 10 valgrind --error-exitcode=2 -q ./$base
    
    exit_code=$?

    if [ $exit_code -ne 0 ]
    then
        fail_list="$fail_list $testcase"
        if [ $exit_code -eq 124 ]
        then
            echo "*** $(testcase) timed out ***"
        fi

    fi
    
    rm -f ./Overflow ./Id-*

    # Comment this line out if you would like to keep the executable to debug
    rm -f ./$base
done

echo "==============================================================================="
if [ -n "$fail_list" ]; then
    echo "FAILED TESTCASES: "
    for tc in $fail_list
    do
        echo " *  $tc"
    done
else
    echo "*** ALL TESTS PASSED ***"
fi
