#!/usr/bin/python3
# Timo Barnard 23607165@sun.ac.za
import subprocess
import os
import json
import rich
import hashlib
import inspect
from rich import print
TestCases = {}
compiledTests = []
project_folder : str = "/Files/Code/alan/"
make_command = "make";
abs_cwd = os.path.abspath(os.getcwd());
cwd = os.path.relpath(os.getcwd(),start=project_folder);
def loadCases(fileLocation="tests.json"):
	global TestCases
	cfg_file = os.path.join(project_folder,"tests",fileLocation)
	if(os.path.exists(cfg_file)):
		print("[bold blue]Loading Test Cases")
		with open(cfg_file, "tr", encoding = "UTF-8") as file:
			TestCases = json.load(file)
def saveCases(fileLocation="tests.json"):
	cfg_file = os.path.join(project_folder,"tests","tests.json")
	print("[bold blue]Saving Test Cases")
	with open(cfg_file, "tw", encoding = "UTF-8") as file:
		json.dump(TestCases,file,indent="\t")
def generateCase(test_type:str,file_path:str):
	os.chdir(os.path.join(project_folder,"tests"))
	c = {}
	file_path =	os.path.relpath(file_path,start=project_folder);
	if file_path in TestCases:
		c = TestCases[file_path]
	else:
		TestCases[file_path] = c
	time = os.path.getmtime(os.path.join(project_folder,file_path))
	if not "time" in c:
		c["time"] = time
	if time>c["time"]:
		checked = False;
	if "checked" in c:
		if c["checked"]:
			print("[bold red]Case reviewed by human")
			return
	else:
		c["checked"] = False

	if not test_type in compiledTests: 
		if compile_test(test_type) == 1:
			return
	print("[bold]Generating Case")
	while not (os.path.exists(os.path.join(project_folder,file_path))):
		print(f"Please Type the correct path\[{file_path}]")
		break;
	if not (os.path.exists(os.path.join(project_folder,file_path))):
		return 1
	t = {}	
	print(file_path)
	p = subprocess.run([os.path.join(project_folder,"bin",test_type),
	os.path.join(project_folder,file_path)],capture_output=True, text=True)
	if not (p.returncode == 139):
		t["stderr"] = p.stderr
		t["stdout"] = p.stdout
		t["vouch"] = 1
		c[test_type] = t
		print("Please Check")
		print("[red]Std Err[/red]\n",t["stderr"])
		print("[green]Std_Out[/green]\n",t["stdout"])
	else:
		print("SEG FAULT")
		print(p.stderr)
		print(p.stdout)
def compile_test(test_type:str): 
	os.chdir(os.path.join(project_folder,"src"))
	print(f"Compiling [bold]{test_type}")
	p = subprocess.run([make_command, test_type],capture_output=True, text=True)
	if (p.returncode == 0) and (p.stderr.strip()==""): 
		print(f"[green]Compilation of [bold]{test_type}[/bold] succeeded");
	else:
		if p.returncode == 0:
			print(f"[orange]Compiled [bold]{test_type}[/bold] with warnings")
			print(p.stderr)
		else: 
			print("[red]Compilation of [bold]{test_type}[/bold] failed")
			print(p.stderr)
			print("[red] Exiting")
			return 1
	if not (test_type in compiledTests): 
		compiledTests.append(test_type)
	return 0
test_nr = 0
def testCase(file_path:str,check_types=[]):
	global test_nr, TestCases
	if not (file_path in TestCases):
		pos = []
		for path in TestCases:
			if path.endswith(file_path):
				pos.append(path)
		if len(pos) == 0:
			print(f"[red bold]\"{file_path}\" not found[/red bold]") 
			return 1
		elif len(pos) == 1:
			file_path = pos[0]
		else:
			return 1
	c = TestCases[file_path]
	for test_type in check_types:
		if not test_type in compiledTests: 
			if compile_test(test_type) == 1:
				return
	
	test_nr = test_nr + 1
	print(f"Test [bold]{test_nr}[/bold]:\n\tFilepath:\t\"{file_path}\"")
	for test_type in check_types:
		print(f"\t[bold blue]{test_type.title()}[/bold blue]");
		p = subprocess.run([os.path.join(project_folder,"bin",test_type),
				os.path.join(project_folder,file_path)],capture_output=True,
		text=True)
		if test_type in c:
			t = c[test_type]
			if not (p.stderr==t["stderr"]):
				print(f"[blue]PROGRAM[/blue]\t[red]StdErr[/red]\t{p.stderr}")
				print(f"[green]TEST[/green]\t[red]StdErr[/red]\t",t["stderr"])
			else:
				print("\t\t[green bold]StdErr is Correct[/green bold]")
			if not (p.stdout==t["stdout"]):
				print(f"[blue]PROGRAM[/blue]\t[red]StdOut[/red]\t{p.stdout}")
				print(f"[green]TEST[/green]\t[red]StdOut[/red]\t",t["stdout"])
			else:
				print("\t\t[green bold]StdOut is Correct[/green bold]")
		else:
			print(f"[red]No Check Exists[/red], but here is your output:")
			print("StdErr",p.stderr)
			print("StdOut",p.stdout)
			print("Return Code\t",p.returncode)
def runAllTestCases():
	global TestCases
	for file_path in TestCases:
		testCase(file_path,["testscanner"])
def generateAllTestCases(folder = os.path.join(project_folder,"tests")):
	for root, folders, files in os.walk(folder):
		for file in files:
			if file.endswith("\.alan") :
				generateCase("testscanner",os.path.join(root,file))
def menu():
	global TestCases
	print()
	try:
		while True:
			print("[bold]1. Load Cases[/bold]")
			print("[bold]2. Generate Testcase[/bold]")
			print("[bold]3. Run All Test Cases[/bold]")
			print("[bold]4. Save and Exit[/bold]")
			print("[bold]5. Exit[/bold]")
			try:
				n = int(input("Please Choose an Option: "))
				if (n==1): 
					path = "tests.json";
					path = input(f"What file do you want to open?[Default: {path}]\n");

					loadCases(path)
				if n == 2:
					path = input(f"What file do you want to open?[Relative to {os.getcwd()}]\n");
					generateCase("testscanner", os.path.abspath(path));
				if n == 3:
					runAllTestCases()
				if (n==4):
					path = "tests.json";
					path = input(f"Where do you want to save?[Default: {path}]\n");
					saveCases(path)
			except KeyboardInterrupt:
				print();

			if (n>3):
				break
			print("\n");
		print("[red bold]Goodbye[/red bold]")
	except KeyboardInterrupt:
		print()
		print("[red bold]Can you hear me [italics]Major Tom[/italics][/red bold]")
		
if __name__=="__main__":
	loadCases();
	menu();
