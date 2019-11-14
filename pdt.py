import subprocess, sys, os
from pyvis.network import Network
process_dict = {}
log_dict = {}
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
    csv_file = open("test.csv")
    line = csv_file.readline()
    num_processes = int(line.split()[0].strip())
    for i in range(num_processes):
        line = csv_file.readline().split(',')
        process = line[0].strip()
        exit_status = int(line[1].strip())
        seg_fault = int(line[2].strip())
        num_children = int(line[3].strip())
        num_open_fds = int(line[4].strip())
        children = [child.strip() for child in line[5: 5 + num_children]]
        open_fds = [fd.strip() for fd in line[5 + num_children: 5 + num_children + num_open_fds]]
        process_dict[process] = {"exit": exit_status, "children": children, "open_fds": open_fds, "segfault":seg_fault}    
    return 0

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
'''
args = ['./pdt', 'Tests/{}'.format(s)]
print("-----Program Output-----")
subprocess.call(args)
print("-----Analysis-----")
print("ended")
print(process_dict)
# debugging: print("Number of processes: {}".format(num_processes))
'''


