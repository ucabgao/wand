# import os
# os.system('./picoc servers/server.c > report/')


import subprocess
import time
import os
import sys
import ast
from datetime import datetime
from pathlib import Path

directory = "../corpus"
files = ["arturgontijo_directshell.c","deadbits_bindshell.c",
"m0nad_bindshell.c","server.c",
"server2.c",
"utsav3_server.c"]
labels= ["===ALL SOCKETS===","===MAY BE LISTENING SOCKETS===","===FORK ON LINE===","===END==="]
output = ""

Path("../reports").mkdir(parents=True, exist_ok=True)

for file in files:
	orig_stdout = sys.stdout
	count = 1
	outputDir = ""

	while(count):
		outputDir = "../reports/"+datetime.today().strftime('%Y%m%d')+"_"+str(count)+"_"+file+"/"
		
		if not os.path.exists(outputDir):
		    os.makedirs(outputDir)
		    break;
		else:
			count += 1


	# start_time = time.time()
	spPicoc = subprocess.Popen(["../picoc", directory+"/"+file], 
	           stdout=subprocess.PIPE, 
	           stderr=subprocess.STDOUT)
	# print("===GENERATED IN %s SECONDS===" % (time.time() - start_time))
	# start_time = time.time()
	spRegex = subprocess.Popen(["python3.7", "regex.py", directory+"/"+file], 
	           stdout=subprocess.PIPE, 
	           stderr=subprocess.STDOUT)
	# print("===GENERATED IN %s SECONDS===" % (time.time() - start_time))
	
	stdoutPicoc,stderrPicoc = spPicoc.communicate()
	stdoutRegex,stderrRegex = spRegex.communicate()

	with open(outputDir+"picoc", 'wb') as myfile:
		myfile.write(stdoutPicoc)
		myfile.close()

	with open(outputDir+"regex", 'wb') as myfile:
		myfile.write(stdoutRegex)
		myfile.close()

	with open(outputDir+"diff", "a") as f:
		sys.stdout = f
		diff = 0
		
		print("*****COMPARING "+file+"*****")

		outputPicoc = list(map(lambda x: x.decode("utf-8"), stdoutPicoc.splitlines()))
		outputRegex = list(map(lambda x: x.decode("utf-8"), stdoutRegex.splitlines()))

		for i in range(len(labels)):
			print(labels[i])
			if i+1 != len(labels):
				if(i == 0): # ===ALL SOCKETS===
					picocSet = set()
					for x in outputPicoc[outputPicoc.index(labels[i])+1:outputPicoc.index(labels[i+1])]:
						dupPortionStr = x.split(":")[3]
						dupPortionList = ast.literal_eval(dupPortionStr)
						dupPortionList.sort()
						picocSet.add(x.replace(dupPortionStr, str(dupPortionList)))

					regexSet = set()
					for x in outputRegex[outputRegex.index(labels[i])+1:outputRegex.index(labels[i+1])]:
						dupPortionStr = x.split(":")[3]
						dupPortionList = ast.literal_eval(dupPortionStr)
						dupPortionList.sort()
						regexSet.add(x.replace(dupPortionStr, str(dupPortionList)))
				else:
					picocSet = set(outputPicoc[outputPicoc.index(labels[i])+1:outputPicoc.index(labels[i+1])])
					regexSet = set(outputRegex[outputRegex.index(labels[i])+1:outputRegex.index(labels[i+1])])

				print("\t---Difference Picoc\\Regex---")
				setDiff = picocSet.difference(regexSet)
				if not setDiff:
					print("\t\tNIL")
				else:
					diff = 1

				for x in picocSet.difference(regexSet):
					print("\t\t"+x)

				print()

				print("\t---Difference Regex\\Picoc---")
				setDiff = regexSet.difference(picocSet)
				if not setDiff:
					print("\t\tNIL")
				else:
					diff = 1

				for x in regexSet.difference(picocSet):
					print("\t\t"+x)
				
			print()

		f.close()

	sys.stdout = orig_stdout
	
	if diff:
		print("Diff in %s" % file)