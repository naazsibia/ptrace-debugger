import subprocess, sys, os
from typing import TextIO
import re
from pyvis.network import Network
process_dict = {}
log_dict = {}
inode_log_dict = {}
Physics = True
mapping = {}

def generateSegFaultString(info_dict,process):
    s = "{}<br>".format(process)
    s+= "# of children: {}<br>".format(len(info_dict["children"]))
    s+= "# of open File Descriptors: {}<br>".format(len(info_dict["open_fds"]))
    s+= "SEGFAULTED<br>"
    return s

def generateTitleString(info_dict,process):
    s = "{}<br>".format(process)
    s+= "# of children: {}<br>".format(len(info_dict["children"]))
    s+= "# of open File Descriptors: {}<br>".format(len(info_dict["open_fds"]))
    s+= "Exit Status: {}<br>".format(info_dict["exit"])
    return s

def generateGraph():
    counter = 0
    graph = Network(directed = True)
    for process in process_dict:
        info_dict = process_dict[process]
        if info_dict["segfault"]:
            graph.add_node(counter,title = generateSegFaultString(info_dict,process),label = process,physics = Physics,color = "red")
        else:
            graph.add_node(counter,title = generateTitleString(info_dict,process),label = process,physics = Physics,color = "#0080ff")
        mapping[process] = counter
        counter += 1

    for process in process_dict:
        info_dict = process_dict[process]
        for child in info_dict["children"]:
            graph.add_edge(mapping[process],mapping[child],physics = Physics, color = "#0080ff")
    graph.show("test.html")
    return 0 

def traceProgram():
    s = input("Program to run: ")
    args = ['./pdt', 'Tests/{}'.format(s)]
    print("-----Program Output-----")
    subprocess.call(args)
    print("-----Analysis-----")
    print("ended")
    csv_file = open("test.csv")
    line = csv_file.readline()
    num_processes = int(line.split(',')[0].strip())
    num_logs = int(line.split()[1].strip())
    read_processes(csv_file, num_processes)
    read_logs(csv_file, num_logs)
    return 0

def read_processes(csv_file: TextIO, num_processes):
    for i in range(num_processes):
        line = csv_file.readline().split(',')
        process = line[0].strip()
        start_time = line[1].strip()
        end_time = line[2].strip()
        exit_status = int(line[3].strip())
        seg_fault = 1 if int(line[4].strip()) else 0
        num_children = int(line[5].strip())
        num_open_fds = int(line[6].strip())
        children = [child.strip() for child in line[7: 7 + num_children]]
        open_fds = [fd.strip() for fd in line[7 + num_children: 7 + num_children + num_open_fds]]
        process_dict[process] = {"start_time": start_time, "end_time": end_time, "exit": exit_status, "seg_fault": seg_fault,  "children": children, "open_fds": open_fds}    
    return i

def read_logs(csv_file: TextIO, num_logs):
    str_read = ""
    node_from = node_to = None
    for line in csv_file:
        m = re.match(r"(W|R), (\d+), (\d+), (\d+),(\S*)", line)
        if(m):
            if(node_from): # add previous data to dictionary
                log_dict[node_from] = log_dict.get(node_from, {})
                log_dict[node_from][node_to] = log_dict[node_from].get(node_to, [])
                log_dict[node_from][node_to].append((action, str_read, bytes_read))
                inode_log_dict.setdefault(inode, []).append((pid, action, str_read, bytes_read)) 
                node_from = node_to = None
            action = m.group(1)
            pid = m.group(2)
            inode = m.group(3)
            bytes_read = m.group(4)
            str_read = m.group(5)
            node_from, node_to = (pid, inode) if action == 'W' else (inode, pid)
        else: # line from prior process continuing 
            str_read += line
    print(log_dict)
    print(inode_log_dict)


def handleInput():
    if len(sys.argv) == 1:
        sys.stderr.write("{usage} [-p] filename [args]\n")
        return -1
    ListOfArgs = []
    if (sys.argv[1] == '-p'):
        Physics = True
        ListOfArgs = sys.argv[2:]
        if len(ListOfArgs) == 0:
            sys.stderr.write("{usage} [-p] filename [args]\n")
            return -1            
    else:
        ListOfArgs = sys.argv[1:]
        
    filename = ListOfArgs[0]
    ListOfArgs = ListOfArgs[1:]
    args = ['./pdt','Tests/{}'.format(filename)]+ListOfArgs
    subprocess.call(args,stderr=subprocess.DEVNULL, stdout=subprocess.DEVNULL)
    traceProgram()
    generateGraph()
    return 0


traceProgram()
generateGraph()





