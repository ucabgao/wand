import re
import sys
import time

class Socket:
	def __init__(self):
		self.identifier = ""
		self.parent = ""
		self.state = "Initial"
		self.dup = list()
		self.line = 0

start_time = time.time()

if len(sys.argv) < 2:
	print("Provide file as second argument")
	exit()

socket_list = []
socket_dict = {}
socket_state_dict = {}
socket_dup_dict = {}
fork_list = []

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
			socketObj.state = "Connected"
			socketObj.line = lineNum
			socket_dict[newSocket] = socketObj
			if socket in socket_dict:
				socket_dict[socket].state = "AwaitConnection"
			# socket_state_dict[newSocket] = "connected"
			
			accept_set.add(socket)

			# if socket in socket_list:
			# 	socket_state_dict[socket] = funcName

		socketFuncMatches = re.finditer(r"((bind|listen)\s*\(\s*\w+)", line)
		# print(socketFuncMatches)
		for match in socketFuncMatches:
			# print(match)
			funcName, socket = (match[0].replace(" ","").split("("))

			if funcName == "bind" and socket in socket_dict:
				socket_dict[socket].state = "AwaitConnection"
				# socket_dict[socket].state = "Binding"
				

			if funcName == "listen":
				listen_set.add(socket)

				if socket in socket_dict:
					socket_dict[socket].state = "AwaitConnection"
					# socket_dict[socket].state = "Passive"


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
			fork_list.append(lineNum)


# print(myfile.read())

print("===ALL SOCKETS===")
# print("Socket List:")
for s in socket_dict:
	socket = socket_dict[s]
	print("%s:%s:%s:%s:%d" % (socket.identifier,socket.parent,socket.state,str(socket.dup),socket.line))
# print(socket_dict)
# print("Socket State Dict:")
# print(socket_state_dict)
# print("Socket Dup Dict:")
# print(socket_dup_dict)
print("===MAY BE LISTENING SOCKETS===")
# for s in listen_set.intersection(accept_set):
# 	socket = socket_dict[s]
# 	print("%s:%s:%s:%s:%d" % (socket.identifier,socket.parent,socket.state,str(socket.dup),socket.line))
for s in socket_dict:
	socket = socket_dict[s]
	if socket.state == "AwaitConnection":
		print("%s:%s:%s:%s:%d" % (socket.identifier,socket.parent,socket.state,str(socket.dup),socket.line))

# print(dup2_set)
print("===FORK ON LINE===")
for s in fork_list:
	print(s)

print("===END===")
import time

print("===GENERATED IN %.5f SECOND(S)===" % (time.time() - start_time))

myfile.close()