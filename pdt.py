import subprocess, sys, os

s = input("File you want to test: ")
process_dict = {}
log_dict = {}
args = ['./pdt', 'Tests/{}'.format(s)]
print("-----Program Output-----")
subprocess.call(args)
print("-----Analysis-----")
print("ended")
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
    process_dict[process] = {"exit": exit_status, "children": children, "open_fds": open_fds}
print(process_dict)
# debugging: print("Number of processes: {}".format(num_processes))


