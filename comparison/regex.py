import re
import sys
import time
import enum
import json


class Socket:
	def __init__(self):
		self.identifier = ""
		self.parent = ""
		self.state = "NotListening"
		self.dup = list()
		self.line = 0


class Characteristic(enum.Enum):
    Fork = 1
    Exec = 2


def output_json():
	jsonDict = {"all_sockets":[], "maybelistening_sockets":[], "fork":[], "exec":[],
				"var_id":[], "func_id":[], "analysis_time":0}

	for s in socket_dict:
		socket = socket_dict[s]

		# all sockets
		jsonDict["all_sockets"].append(socket.__dict__)

		# maybelistening sockets
		if socket.state == "MayBeListening":
			jsonDict["maybelistening_sockets"].append(socket.__dict__)

	# fork
	for s in [y for (x,y) in characteristic_list if x == Characteristic.Fork]:
		jsonDict["fork"].append(s)

	# exec
	for s in [y for (x,y) in characteristic_list if x == Characteristic.Exec]:
		jsonDict["exec"].append(s)

	# analysis time
	jsonDict["analysis_time"] = time.time() - start_time

	# print json string
	print(json.dumps(jsonDict, indent=8))


start_time = time.time()

if len(sys.argv) < 2:
	print("Provide file as second argument")
	exit()

socket_dict = {}
characteristic_list = []
lineNum = 0;

with open(sys.argv[1], 'r') as myfile:
	for line in myfile:
		lineNum+=1
		line = line.strip()

		# if it is a comment
		if line.startswith("//"):
			continue

		# identifying packet sockets
		socketParentMatches = re.finditer(r"\w+\s*=\s*socket\s*\(", line)
		for match in socketParentMatches:
			# get identifier of assignment statement
			socket = match[0].replace(" ","").split("=")[0]

			# create socket object to store parent socket's info and add to socket dictionary
			socketObj = Socket()
			socketObj.identifier = socket
			socketObj.line = lineNum
			socket_dict[socket] = socketObj

		# identifying child sockets
		socketChildMatches = re.finditer(r"\w+\s*=\s*accept\s*\(\s*\w+", line)
		for match in socketChildMatches:
			# get child identifier, function name and parent identifier
			newSocket, funcName, socket = (re.split("=|\(", match[0].replace(" ","")))
			
			# create socket object to store child socket's info and add to socket dictionary
			socketObj = Socket()
			socketObj.identifier = newSocket
			socketObj.parent = socket
			socketObj.state = "NotReadingOrWriting"
			socketObj.line = lineNum
			socket_dict[newSocket] = socketObj

			# if the parent socket was not seen before, create socket object and add to socket dictionary
			if socket not in socket_dict:
				socketObj = Socket()
				socketObj.identifier = socket
				socketObj.line = lineNum
				socket_dict[socket] = socketObj

			# change parent socket state to MayBeListening
			socket_dict[socket].state = "MayBeListening"

		# identifying socket functions to change states
		socketFuncMatches = re.finditer(r"((bind|listen|read|recv|recvfrom|write|send|sendto)\s*\(\s*\w+)", line)
		for match in socketFuncMatches:
			# get function name and identifier of socket affected
			funcName, socket = (match[0].replace(" ","").split("("))

			# functions pertaining to parent sockets
			if funcName in ["bind", "listen"]:
				if socket not in socket_dict:
					socketObj = Socket()
					socketObj.identifier = socket
					socketObj.line = lineNum
					socket_dict[socket] = socketObj

				socket_dict[socket].state = "MayBeListening"

			# functions pertaining to child sockets
			elif funcName in ["read", "recv", "recvfrom", "write", "send", "sendto"]:
				# if child socket not seen before, parent is UNKNOWN
				if socket not in socket_dict:
					socketObj = Socket()
					socketObj.identifier = socket
					socketObj.parent = "UNKNOWN"
					socketObj.state = "NotReadingOrWriting"
					socketObj.line = lineNum
					socket_dict[socket] = socketObj

				socket_dict[socket].state = "MayBeReadingOrWriting"

		# identifying dup2 functions
		dupFuncMatch = re.finditer(r"dup2\s*\(\s*\w+\s*,\s*\w\s*\)", line)
		for match in dupFuncMatch:
			# get socket identifier and fd
			funcName, socket, assignedFD = (re.split("\(|,", match[0].replace(" ","")[:-1]))
			assignedFD = int(assignedFD)

			# only interested in 0, 1, 2
			if assignedFD in range(3) and socket in socket_dict:
				socket_dict[socket].dup.append(assignedFD)

		# identifying fork functions
		forkFuncMatch = re.finditer(r"fork\s*\(", line)
		for match in forkFuncMatch:
			characteristic_list.append((Characteristic.Fork,lineNum))

		# identifying exec functions
		execFuncMatch = re.finditer(r"(execl|execlp|execle|execv|execvp|execvpe)\s*\(", line)
		for match in execFuncMatch:
			characteristic_list.append((Characteristic.Exec,lineNum))


	print("===JSON OUTPUT===")
	output_json();
	print("===END===")

myfile.close()