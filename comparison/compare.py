import subprocess
import os
import sys
import json
from datetime import datetime
from pathlib import Path

# directory where the corpus is
directory = "../corpus"

# file names of the corpus
files = ["arturgontijo_directshell.c","bressan3_server.c","deadbits_bindshell.c","dmknght_linuxbind.c",
"fakhrizulkifli_n00bshell.c","geyslan_shellbindtcp.c","m0nad_bindshell.c",
"tutorial_server.c","tutorial_server_multiclient.c","tutorial_server_multiclient_array.c","tutorial_server2.c",
"utsav3_server.c","valbrux_bindshell.c"]

# output labels
labels = ["===JSON OUTPUT===","===END==="]

totalTimePicoc = 0
totalTimeRegex = 0

Path("../reports").mkdir(parents=True, exist_ok=True)

for file in files:
	orig_stdout = sys.stdout

	count = 1
	outputDir = ""

	# create file's report directory
	while(count):
		outputDir = "../reports/"+datetime.today().strftime('%Y%m%d')+"_"+str(count)+"_"+file+"/"
		
		if not os.path.exists(outputDir):
		    os.makedirs(outputDir)
		    break;
		else:
			count += 1

	# execute picoc and syntax-based search program with file
	spPicoc = subprocess.Popen(["../picoc", directory+"/"+file], 
	           stdout=subprocess.PIPE, 
	           stderr=subprocess.STDOUT)

	spRegex = subprocess.Popen(["python3.7", "regex.py", directory+"/"+file], 
	           stdout=subprocess.PIPE, 
	           stderr=subprocess.STDOUT)
	
	stdoutPicoc,stderrPicoc = spPicoc.communicate()
	stdoutRegex,stderrRegex = spRegex.communicate()

	# write outputs to respective files
	with open(outputDir+"picoc", 'wb') as myfile:
		myfile.write(stdoutPicoc)
		myfile.close()

	with open(outputDir+"regex", 'wb') as myfile:
		myfile.write(stdoutRegex)
		myfile.close()

	# generate file for output difference
	with open(outputDir+"diff", "a") as f:
		# change stdout to f, print() will write to file
		sys.stdout = f

		overallDiff = 0
		diff = 0

		print("*****COMPARING "+file+"*****")
		decodedPicoc = stdoutPicoc.decode("utf-8")
		decodedRegex = stdoutRegex.decode("utf-8")

		# obtain index for start and end of JSON portion in output
		startIndexPicoc = decodedPicoc.index(labels[0]) + len(labels[0])
		endIndexPicoc = decodedPicoc.index(labels[1])
		startIndexRegex = decodedRegex.index(labels[0]) + len(labels[0])
		endIndexRegex = decodedRegex.index(labels[1])

		jsonPicoc = json.loads(stdoutPicoc[startIndexPicoc:endIndexPicoc])
		jsonRegex = json.loads(stdoutRegex[startIndexRegex:endIndexRegex])

		for (k,v), (k2,v2) in zip(jsonPicoc.items(), jsonRegex.items()):
			# not comparing var identifier, func identifier and analysis time
			# because syntax-based search program does not have var_id and func_id
			if k == "var_id" or k == "func_id" or k == "analysis_time":
				continue

			print("===%s===" % k.upper().replace("_"," "))

			# compare sockets
			if k == "all_sockets" or k == "maybelistening_sockets" :
				if k == "maybelistening_sockets":
					v = [s for s in v if s["state"] == "MayBeListening"]
					v2 = [s for s in v2 if s["state"] == "MayBeListening"]

				for s in v:
					s["dup"].sort()

				for s in v2:
					s["dup"].sort()

				print("\t---Difference Picoc\\Regex---")
				for i in v:
					if i not in v2:
						overallDiff = 1
						diff = 1
						print("\t\t"+str(i))

				if not diff:
					print("\t\tNIL")

				print()
				diff = 0

				print("\t---Difference Regex\\Picoc---")
				for i in v2:
					if i not in v:
						overallDiff = 1
						diff = 1
						print("\t\t"+str(i))

				if not diff:
					print("\t\tNIL")

				print()
				diff = 0

			# compare fork and exec
			else:
				v = set(v)
				v2 = set(v2)

				print("\t---Difference Picoc\\Regex---")
				diffSet = list(v.difference(v2))

				if diffSet:
					overallDiff = 1
					print("\t\t"+str(diffSet))
				else:
					print("\t\tNIL")

				print()

				print("\t---Difference Regex\\Picoc---")
				diffSet = list(v2.difference(v))

				if diffSet:
					overallDiff = 1
					print("\t\t"+str(diffSet))
				else:
					print("\t\tNIL")

				print()

		f.close()


	sys.stdout = orig_stdout
	
	totalTimePicoc += jsonPicoc["analysis_time"]
	totalTimeRegex += jsonRegex["analysis_time"]

	if overallDiff:
		print("Diff in %s" % file)
		
print("Average time taken by PicoC: %.5f" % (totalTimePicoc))
print("Average time taken by Regex: %.5f" % (totalTimeRegex))