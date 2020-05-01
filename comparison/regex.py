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
	print(json.dumps(jsonDict, indent=8))



start_time = time.time()

if len(sys.argv) < 2:
	print("Provide file as second argument")
	exit()

socket_list = []
socket_dict = {}
socket_state_dict = {}
socket_dup_dict = {}
characteristic_list = []

listen_set = set()
accept_set = set()
dup2_set = set()

lineNum = 0;

# with open('picoc/arturgontijo_directshell.c', 'r') as myfile:
with open(sys.argv[1], 'r') as myfile:
	# data = myfile.read().splitlines()
	for line in myfile:
		lineNum+=1
		line = line.strip()
		# print(p.match(line))
		if line.startswith("//"):
			continue

		socketParentMatches = re.finditer(r"\w+\s*=\s*socket\s*\(", line)
		for match in socketParentMatches:
			socket = match[0].replace(" ","").split("=")[0]

			socketObj = Socket()
			socketObj.identifier = socket
			socketObj.line = lineNum
			socket_dict[socket] = socketObj
			# socket_state_dict[socket] = "initial"

		socketChildMatches = re.finditer(r"\w+\s*=\s*accept\s*\(\s*\w+", line)
		for match in socketChildMatches:
			newSocket, funcName, socket = (re.split("=|\(", match[0].replace(" ","")))
			
			socketObj = Socket()
			socketObj.identifier = newSocket
			socketObj.parent = socket
			socketObj.state = "NotReadingOrWriting"
			socketObj.line = lineNum
			socket_dict[newSocket] = socketObj

			if socket not in socket_dict:
				socketObj = Socket()
				socketObj.identifier = socket
				socketObj.line = lineNum
				socket_dict[socket] = socketObj

			socket_dict[socket].state = "MayBeListening"
			# else:
			# socket_state_dict[newSocket] = "connected"
			
			accept_set.add(socket)

			# if socket in socket_list:
			# 	socket_state_dict[socket] = funcName

		socketFuncMatches = re.finditer(r"((bind|listen|read|recv|recvfrom|write|send|sendto)\s*\(\s*\w+)", line)
		# print(socketFuncMatches)
		for match in socketFuncMatches:
			# print(match)
			funcName, socket = (match[0].replace(" ","").split("("))

			if funcName in ["bind", "listen"]:
				if socket not in socket_dict:
					socketObj = Socket()
					socketObj.identifier = socket
					socketObj.line = lineNum
					socket_dict[socket] = socketObj

				if funcName == "bind" :
					socket_dict[socket].state = "MayBeListening"
					# socket_dict[socket].state = "Binding"
				elif funcName == "listen":
					listen_set.add(socket)
					socket_dict[socket].state = "MayBeListening"
					# socket_dict[socket].state = "Passive"

			elif funcName in ["read", "recv", "recvfrom", "write", "send", "sendto"]:
				if socket not in socket_dict:
					socketObj = Socket()
					socketObj.identifier = socket
					socketObj.parent = "UNKNOWN"
					socketObj.state = "NotReadingOrWriting"
					socketObj.line = lineNum
					socket_dict[socket] = socketObj

				socket_dict[socket].state = "MayBeReadingOrWriting"


			# if socket in socket_list:
			# 	socket_state_dict[socket] = funcName

		dupFuncMatch = re.finditer(r"dup2\s*\(\s*\w+\s*,\s*\w\s*\)", line)
		for match in dupFuncMatch:
			funcName, socket, assignedFD = (re.split("\(|,", match[0].replace(" ","")[:-1]))

			# print(socket)
			assignedFD = int(assignedFD)

			if assignedFD in range(3) and socket in socket_dict:
				# if socket in socket_dup_dict:
				# 	socket_dup_dict[socket].append(assignedFD)
				# else:
				# 	socket_dup_dict[socket] = []
				# 	socket_dup_dict[socket].append(assignedFD)
				socket_dict[socket].dup.append(assignedFD)

				dup2_set.add(socket)

		forkFuncMatch = re.finditer(r"fork\s*\(", line)
		for match in forkFuncMatch:
			# fork_list.append("Line "+str(lineNum)+": "+line)
			characteristic_list.append((Characteristic.Fork,lineNum))

		forkFuncMatch = re.finditer(r"(execl|execlp|execle|execv|execvp|execvpe)\s*\(", line)
		for match in forkFuncMatch:
			# fork_list.append("Line "+str(lineNum)+": "+line)
			characteristic_list.append((Characteristic.Exec,lineNum))


# print(myfile.read())
	print("===JSON OUTPUT===")
	output_json();
	print("===END===")

myfile.close()
