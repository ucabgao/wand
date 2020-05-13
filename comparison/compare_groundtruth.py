import subprocess
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

for file in files:
	# execute picoc with file
	spPicoc = subprocess.Popen(["../picoc", directory+"/"+file], 
	           stdout=subprocess.PIPE, 
	           stderr=subprocess.STDOUT)
	
	stdoutPicoc,stderrPicoc = spPicoc.communicate()

	decodedPicoc = stdoutPicoc.decode("utf-8")

	# obtain index for start and end of JSON portion in output
	startIndexPicoc = decodedPicoc.index(labels[0]) + len(labels[0])
	endIndexPicoc = decodedPicoc.index(labels[1])

	jsonPicoc = json.loads(stdoutPicoc[startIndexPicoc:endIndexPicoc])

	# load groundtruth JSON string
	with open(directory+"/groundtruth/"+file[:-2], "r") as f:
		fileString = f.read()
		startIndex = fileString.index(labels[0]) + len(labels[0])
		endIndex = fileString.index(labels[1])

		jsonGroundtruth = json.loads(fileString[startIndex:endIndex])

		f.close()

	diff = 0

	for (k,v), (k2,v2) in zip(jsonPicoc.items(), jsonGroundtruth.items()):
		# not comparing analysis time
		if k == "analysis_time":
			continue

		# compare sockets
		if k == "all_sockets" or k == "maybelistening_sockets" :
			if k == "maybelistening_sockets":
				v = [s for s in v if s["state"] == "MayBeListening"]
				v2 = [s for s in v2 if s["state"] == "MayBeListening"]

			for s in v:
				s["dup"].sort()

			for s in v2:
				s["dup"].sort()

			for i in v:
				if i not in v2:
					diff = 1
					break

			for i in v2:
				if i not in v:
					diff = 1
					break

		# compare fork, exec, var_id, func_id
		else:
			v = set(v)
			v2 = set(v2)

			diffSet = list(v.difference(v2))

			if diffSet:
					diff = 1
					break

			diffSet = list(v2.difference(v))

			if diffSet:
					diff = 1
					break

	if diff:
		print("Diff in %s" % file)