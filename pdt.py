import subprocess, sys, os
from typing import TextIO
import re
from pyvis.network import Network
import plotly
import pandas as pd
import plotly.graph_objs as go
from plotly.offline import init_notebook_mode, iplot
import string


#generates the hoverable title string for a segfaulted process
def generateSegFaultString(info_dict,process):
    s = "{}<br>".format(process)
    s+= "# of children: {}<br>".format(len(info_dict["children"]))
    s+= "# of open File Descriptors: {}<br>".format(len(info_dict["open_fds"]))
    s+= "SEGFAULTED<br>"
    return s
#generates the hoverable title string for a normal process
def generateTitleString(info_dict,process):
    s = "{}<br>".format(process)
    s+= "# of children: {}<br>".format(len(info_dict["children"]))
    s+= "# of open File Descriptors: {}<br>".format(len(info_dict["open_fds"]))
    s+= "Exit Status: {}<br>".format(info_dict["exit"])
    return s

#generates the hoverable title string for a pipe
def generateInodeString(lst,inode):
    s = "{}<br>".format(inode)
    for (process, mode, string, bytesWritten) in lst:
        if mode == "W":
            s+= "{} wrote {} with {} bytes<br>".format(process,string,bytesWritten)
        else:
            s+= "{} read {} with {} bytes<br>".format(process,string,bytesWritten)
    s+= "Total number of entries: {}<br>".format(len(lst))
    return s

#generates the process graph
def generateGraph():
    counter = 0
    graph = Network(directed = True)
    #Add mapping for every process
    for process in process_dict:
        info_dict = process_dict[process]
        if info_dict["seg_fault"]:
            graph.add_node(counter,title = generateSegFaultString(info_dict,process),label = process,physics = Physics,color = "red")
        else:
            graph.add_node(counter,title = generateTitleString(info_dict,process),label = process,physics = Physics,color = "#0080ff")
        mapping[process] = counter
        counter += 1

    #Add connections between processes
    for process in process_dict:
        info_dict = process_dict[process]
        for child in info_dict["children"]:
            graph.add_edge(mapping[process],mapping[child],physics = Physics, color = "#0080ff")

    #Add file descriptor nodes
    for inode in inode_log_dict:
        graph.add_node(counter,title = generateInodeString(inode_log_dict[inode],inode),label = inode,Physics = Physics, color = "#FFA500", shape = "diamond")
        mapping[inode] = counter
        counter += 1


     #Add connections between file descriptors and nodes
    strings = {}
    modes = {}
    for process in log_dict:
        for inode in log_dict[process]:
            s = "{},{}<br>".format(process,inode)
            modes[(process,inode)] = {}
            modes[(process,inode)] = {"W":0,"R":0}
            for (mode, stringWritten, bytesWritten) in log_dict[process][inode]:
                if mode == "W":
                    s+= "wrote {} with {} bytes<br>".format(stringWritten,bytesWritten)
                    modes[(process,inode)]["W"] = 1
                else:
                    s+= "read {} with {} bytes<br>".format(stringWritten,bytesWritten)
                    modes[(process,inode)]["R"] = 1
            strings[(process,inode)] = s 

    #Add connections between pipes and processes
    for (process,inode) in strings:
        info_dict = modes[(process,inode)]
        if info_dict["W"] and info_dict["R"]:
            graph.add_edge(mapping[process],mapping[inode],physics = Physics, color = "#0080ff", title = strings[(process,inode)])
            graph.add_edge(mapping[inode], mapping[process],physics = Physics, color = "#0080ff")
        elif info_dict["W"]:
            graph.add_edge(mapping[process],mapping[inode],physics = Physics, color = "#0080ff", title = strings[(process,inode)])
        else:
            graph.add_edge(mapping[inode],mapping[process],physics = Physics, color = "#0080ff", title = strings[(process,inode)])
    
    #Add open file descriptors
    for process in process_dict:
        info_dict = process_dict[process]
        for (inode,pipe) in info_dict["open_fds"]:
            if mapping.get(inode,-1) == -1:
                graph.add_node(counter,title = generateInodeString([],inode),label = inode,Physics = Physics, color = "#FFA500", shape = "diamond")
                mapping[inode] = counter
                counter += 1
            if pipe == "1":
                graph.add_edge(mapping[process],mapping[inode],physics = Physics, color = "red")
            else:
                graph.add_edge(mapping[inode],mapping[process],physics = Physics, color = "red")

    #Saves the graph to an html file
    graph.show("{}.html".format(program_name))
    return 0 

def traceProgram(program: str, args_input: list, process_dict, log_dict, inode_log_dict):
    args = ['./pdt', program]
    args.extend(args_input)
    print(args)
    print("-----Program Output-----")
    subprocess.call(args)
    print("-----Analysis-----")
    print("ended") 
    with open("test.csv", encoding="utf8", errors='ignore') as csv_file: 
        line = csv_file.readline()
        num_processes = int(line.split(',')[0].strip())
        num_logs = int(line.split()[1].strip())
        read_processes(csv_file, num_processes, process_dict)
        read_logs(csv_file, num_logs, log_dict, inode_log_dict)
    return process_dict, log_dict

def read_processes(csv_file: TextIO, num_processes, process_dict: dict):
    for i in range(num_processes):
        line = csv_file.readline().split(',')
        process = line[0].strip()
        start_time = float(line[1].strip())
        end_time = float(line[2].strip())
        exit_status = int(line[3].strip())
        seg_fault = 1 if int(line[4].strip()) else 0
        num_children = int(line[5].strip())
        num_open_fds = int(line[6].strip())
        num_fds = int(line[7].strip())
        children = [child.strip() for child in line[8: 8 + num_children]]
        open_fds = [tuple(fd.strip()[1:-1].split()) for fd in line[8 + num_children: 8 + num_children + num_open_fds]]
        process_dict[process] = {"start_time": start_time, "end_time": end_time, "exit": exit_status, "seg_fault": seg_fault,  "children": children, "open_fds": open_fds, "num_fds": num_fds}    
    return process_dict

def read_logs(csv_file: TextIO, num_logs, log_dict: dict, inode_log_dict: dict):
    str_read = ""
    pid =inode = None
    for line in csv_file:
        #m = re.match(r"(W|R), (\d+), (\d+), (\d+),(\S*)", line)

        # 02000000

        m = re.match(r"(W|R), (\d+), (\d+), (\d+), ([a-fA-F0-9-:]+$)\n", line)
        if(m):
            if(pid): # add previous data to dictionary
                # use is_ascii for str_read - non-printable characters accept for \n and \t
                # check to see that byte == something between 32 and 126, or 9, 12, 13  (tab, newline, carriadge return)
                
                try:
                    s = bytes.fromhex(str_read[:]).decode('utf-8')
                    s1 = "".join([c for c in s if c in string.printable])
                except ValueError:
                    s1 = ""
                if s1.strip() == "":
                   s1 = str_read
                add_data_to_log(pid, inode, (action, s1, bytes_read), log_dict, inode_log_dict) 
                pid = inode = None
            action = m.group(1)
            pid = m.group(2)
            inode = m.group(3)
            bytes_read = m.group(4)
            str_read = m.group(5)
            str_read = str_read.replace(":", " ")
        else: # line from prior process continuing 
            str_read += line
    if(pid):
        try:
            s = bytes.fromhex(str_read[:]).decode('utf-8')
            s1 = "".join([c for c in s if c in string.printable])
        except ValueError:
            s1 = ""
        if s1.strip() == "":
            s1 = str_read
        
        add_data_to_log(pid, inode, (action, s1, bytes_read), log_dict, inode_log_dict)    
    return log_dict

def add_data_to_log(pid: int, inode: int, data: tuple, log_dict: dict, inode_log_dict: dict):
    log_dict[pid] = log_dict.get(pid, {})
    log_dict[pid][inode] = log_dict[pid].get(inode, [])
    log_dict[pid][inode].append(data)
    inode_log_dict.setdefault(inode, []).append((pid, ) + data) 
    print("Data is |{}|".format(data))



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
    args = ['./pdt','{}'.format(filename)]+ListOfArgs
    subprocess.call(args,stderr=subprocess.DEVNULL, stdout=subprocess.DEVNULL)
    process_dict = {}
    log_dict = {}
    traceProgram(filename, ListOfArgs, process_dict, log_dict, inode_log_dict)
    generateGraph()
    return 0 


# Disable
def blockPrint():
    sys.stdout = open(os.devnull, 'w')

# Restore
def enablePrint():
    sys.stdout = sys.__stdout__

def generate_gannt_chart():
    blockPrint()
    init_notebook_mode(connected=True)
    enablePrint()
    df_list = []
    process_list = [(process, process_dict[process]["start_time"], process_dict[process]["end_time"]) for process in process_dict]
    for process in process_list:
        df_list.append(dict(Task = process[0], Start = process[1], Finish = process[2]))
    df = pd.DataFrame(df_list)
    df['duration'] = df["Finish"] - df["Start"]
    df.sort_values(by=['Start'], ascending=False)
    df['Task'] = df['Task'].astype(str)
    fig = go.Figure(
        layout = {
            'barmode': 'stack',
            'xaxis': {'automargin': True},
            'yaxis': {'automargin': True, 'autorange': 'reversed'}}
    )
    for process, process_df in df.groupby('Task'):
        fig.add_bar(x=process_df.duration,
                    y=process_df.Task,
                    base=process_df.Start,
                    orientation='h',
                    showlegend=False,
                    name=process,
                    hovertext="exit status: {}".format(process_dict[process]["exit"])
                    )
    plotly.offline.plot(fig, filename='{}_gannt_chart.html'.format(program_name), auto_open=False)

def generateErrorLog():
    hasSegfault = False
    hasopenFD = False
    #Checking for Segfaults and Uno:
    for process in process_dict:
        info_dict = process_dict[process]
        if info_dict["seg_fault"]:
            hasSegfault = True
        if len(info_dict["open_fds"]) != 0:
            hasopenFD = True
    
    if hasSegfault or hasopenFD:
        file = open("error.log","w+")
        if hasSegfault:
            file.write("A process segfaulted.")
        elif hasopenFD:
            file.write("Unclosed file descriptors detected:\n")
            for process in process_dict:
                info_dict = process_dict[process]
                for (inode,pipe) in info_dict["open_fds"]:
                    write_string = ''
                    if pipe == "1": #Write end
                        write_string = "(PIPE=WRITE,PID= %s,INODE = %s)\n" %(process,inode)
                    else: #Read end
                        write_string = "(PIPE=READ,PID= %s,INODE = %s)\n" %(process,inode)
                    file.write(write_string)
        file.close()
    
if __name__ == '__main__':
    process_dict = {}
    log_dict = {}
    inode_log_dict = {}
    Physics = False
    mapping = {}
    program_name = sys.argv[1]
    traceProgram(program_name, sys.argv[2:], process_dict, log_dict, inode_log_dict)
    generateGraph()
    generateErrorLog()
    generate_gannt_chart()
  





