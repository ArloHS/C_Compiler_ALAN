#!/bin/bash

cd src
make clean
make testscanner
cd ..

pass=0
i=0

prefix="testcases/"

echo
echo -e "\033[0;46mRunning Scanner tests: \033[0m"
echo
for TESTCASE in testcases/*.alan
do
    test=${TESTCASE#"$prefix"}
	./bin/testscanner $TESTCASE &> ${TESTCASE%.alan}.out
    if cmp --silent -- ${TESTCASE%.alan}.out ${TESTCASE%.alan}.suggested; then
      pass=$(($pass + 1))
    else
      echo -e "\033[0;31mFailed ${test}.\033[0m"
    fi
    i=$(($i + 1))
done

if (($pass == $i)); then
    echo -e "\033[0;32mPassed all tests.\033[0m"
fi

percentage=$((100*$pass/$i))
echo
echo -e "\033[0;46mTesting complete:\033[0m"
echo
echo -e "\033[0;36mTests Run: $i"
echo -e "Tests Passed: ($pass/$i)"
echo -e "Percentage: $percentage%\033[0m"
echo
if (($pass != $i)); then
    echo -e "\033[0;33mCheck the suggested output in the testcases folder for the tests you failed.\033[0m"
fi
echo