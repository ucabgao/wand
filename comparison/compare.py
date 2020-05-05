# import os
# os.system('./picoc servers/server.c > report/')


import subprocess
import time
import os
import sys
import ast
import re
import json
from datetime import datetime
from pathlib import Path

directory = "../corpus"
# files = ["deadbits_bindshell.c"]
# files = ["server_multiclient_array.c"]
files = ["arturgontijo_directshell.c","bressan3_server.c","deadbits_bindshell.c","dmknght_linuxbind.c",
"fakhrizulkifli_n00bshell.c","geyslan_shellbindtcp.c","m0nad_bindshell.c",
"tutorial_server.c","tutorial_server_multiclient.c","tutorial_server_multiclient_array.c","tutorial_server2.c",
"utsav3_server.c","valbrux_bindshell.c"]
labels = ["===JSON OUTPUT===","===END==="]
# labels= ["===ALL SOCKETS===","===MAY BE LISTENING SOCKETS===","===FORK ON LINE===","===END==="]
output = ""

totalTimePicoc = 0
totalTimeRegex = 0

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

	# with open(outputDir+"diff", "a") as f:
	with open(outputDir+"diff", "a") as f:
		sys.stdout = f
		overallDiff = 0
		diff = 0

		print("*****COMPARING "+file+"*****")

		# outputPicoc = list(map(lambda x: x.decode("utf-8"), stdoutPicoc.splitlines()))
		# outputRegex = list(map(lambda x: x.decode("utf-8"), stdoutRegex.splitlines()))
		decodedPicoc = stdoutPicoc.decode("utf-8")
		decodedRegex = stdoutRegex.decode("utf-8")

		# print(decodedRegex)

		startIndexPicoc = decodedPicoc.index(labels[0]) + len(labels[0])
		endIndexPicoc = decodedPicoc.index(labels[1])
		startIndexRegex = decodedRegex.index(labels[0]) + len(labels[0])
		endIndexRegex = decodedRegex.index(labels[1])

		jsonPicoc = json.loads(stdoutPicoc[startIndexPicoc:endIndexPicoc])
		jsonRegex = json.loads(stdoutRegex[startIndexRegex:endIndexRegex])


		for (k,v), (k2,v2) in zip(jsonPicoc.items(), jsonRegex.items()):
			if k == "var_id" or k == "func_id" or k == "analysis_time":
				continue

			print("===%s===" % k.upper().replace("_"," "))
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

		# for key in jsonRegex:
		# 	print(key)
		# 	if key == "all_sockets" or key == "maybelistening_sockets":
		# 		for socket in jsonRegex[key]:
		# 			print(socket)
			# jsonRegex[key]
		# print(jsonPicoc["all_sockets"])

		# for i in range(len(labels)):
		# 	if i+1 == len(labels):
		# 		break

			# print(labels[i])
			# start = stdoutRegex.decode("utf-8").index(labels[i])
			# end = stdoutRegex.decode("utf-8").index(labels[i+1])
			# print(json.loads(stdoutRegex[start+len(labels[i]):end]))
			# x = stdoutRegex.decode("utf-8").replace("\n","").replace("\r","")
			# print(x)
			# print(stdoutPicoc.decode("utf-8").replace("\n",""))
			# print(json.loads(x))
			# x = json.loads()
			# print(x)

			# if i+1 != len(labels):
			# 	if(i == 0): # ===ALL SOCKETS===
			# 		picocSet = set()
			# 		for x in outputPicoc[outputPicoc.index(labels[i])+1:outputPicoc.index(labels[i+1])]:
			# 			dupPortionStr = x.split(":")[3]
			# 			dupPortionList = ast.literal_eval(dupPortionStr)
			# 			dupPortionList.sort()
			# 			picocSet.add(x.replace(dupPortionStr, str(dupPortionList)))

			# 		regexSet = set()
			# 		for x in outputRegex[outputRegex.index(labels[i])+1:outputRegex.index(labels[i+1])]:
			# 			dupPortionStr = x.split(":")[3]
			# 			dupPortionList = ast.literal_eval(dupPortionStr)
			# 			dupPortionList.sort()
			# 			regexSet.add(x.replace(dupPortionStr, str(dupPortionList)))
			# 	else:
			# 		picocSet = set(outputPicoc[outputPicoc.index(labels[i])+1:outputPicoc.index(labels[i+1])])
			# 		regexSet = set(outputRegex[outputRegex.index(labels[i])+1:outputRegex.index(labels[i+1])])

			# 	print("\t---Difference Picoc\\Regex---")
			# 	setDiff = picocSet.difference(regexSet)
			# 	if not setDiff:
			# 		print("\t\tNIL")
			# 	else:
			# 		diff = 1

			# 	for x in picocSet.difference(regexSet):
			# 		print("\t\t"+x)

			# 	print()

			# 	print("\t---Difference Regex\\Picoc---")
			# 	setDiff = regexSet.difference(picocSet)
			# 	if not setDiff:
			# 		print("\t\tNIL")
			# 	else:
			# 		diff = 1

			# 	for x in regexSet.difference(picocSet):
			# 		print("\t\t"+x)
				
			# print()

		f.close()


	sys.stdout = orig_stdout

	# matches = re.finditer(r"===GENERATED IN (.*) SECOND\(S\)===", stdoutPicoc.decode("utf-8"))
	# for match in matches:
	# 	time = float(match[1])
	# 	totalTimePicoc += time

	# matches = re.finditer(r"===GENERATED IN (.*) SECOND\(S\)===", stdoutRegex.decode("utf-8"))
	# for match in matches:
	# 	time = float(match[1])
	# 	totalTimeRegex += time
	
	totalTimePicoc += jsonPicoc["analysis_time"]
	totalTimeRegex += jsonRegex["analysis_time"]

	if overallDiff:
		print("Diff in %s" % file)
print("Average time taken by PicoC: %.5f" % (totalTimePicoc))
print("Average time taken by Regex: %.5f" % (totalTimeRegex))