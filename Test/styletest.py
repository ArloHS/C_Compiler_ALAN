#########################################################
# @file    styletest.py
# @brief   Test source files for correct style.
# @author  C Strydom (21166641@sun.ac.za)
# @date    2018-08-13
#########################################################

import os
import re

'''Performs a style check on all c and h files in src directory'''
def test_style():
	is_valid = True
	if not grep("-P", "\sif\(", "Incorrect use of if statements:"):
		is_valid = False
	if not grep("-P", "\sswitch\(", "Incorrect use of switch statements:"):
		is_valid = False
	if not grep("-P", "\swhile\(", "Incorrect use of while loops:"):
		is_valid = False
	if not grep("-P", "\sfor\(", "Incorrect use of for loops:"):
		is_valid = False
	if not grep("", "sizeof ", "Incorrect use of sizeof:"):
		is_valid = False
	if not grep("", "){", "Incorrect use of opening brace:"):
		is_valid = False
	if not grep("", "( ", "Incorrect use of opening parenthesis:"):
		is_valid = False
	if not grep("", " )", "Incorrect use of closing parenthesis:"):
		is_valid = False
	if not grep("-P", "\s$", "The following lines end in, or contain only spaces:"):
		is_valid = False
	return is_valid

'''greps on a particular sequence and displays the appropriate error message with output'''
def grep(flags, text, message):
	is_valid = False
	os.system('grep %s -n "%s" ../src/*.c ../src/*.h > output.temp' % (flags, text))
	os.system('wc -l output.temp > wc.temp')
	f = open("wc.temp", "r")
	if int(f.read(1)) == 0:
		is_valid = True
	else:
		print(message)
		os.system('cat output.temp')
	f.close()
	os.system('rm *.temp')
	return is_valid

if __name__ == "__main__":
	if test_style():
		print("No critical style errors found in source files.")
