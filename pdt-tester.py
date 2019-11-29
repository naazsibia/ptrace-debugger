from pdt import traceProgram
from n_ary_tree import NAryTree

expected_process_dict = {}
expected_log_dict = {}

actual_process_dict = {}
actual_log_dict = {}

def generate_tree(process_dict: dict) -> NAryTree:
    process_list = list(process_dict.keys())
    if(process_list == []):
        print(process_list, process_dict)
        return None
    root = min(process_list)
    root_node = NAryTree(root)
    processes_added = [root]
    i = 0
    while len(processes_added) != len(process_list) and i < len(processes_added):
         curr_proc = processes_added[i]
         add_data_for_children(curr_proc, process_dict, root_node, processes_added)
         i += 1
    return root_node

def add_data_for_children(process: str, process_dict: dict, root_node: NAryTree, processes_added: list):
    children = process_dict[process]["children"]
    for child in children:
        root_node.insert(child, process) # adding as child of process
    processes_added.extend(children)



def compare_tree(expected_tree: NAryTree, actual_tree: NAryTree):    
    if expected_tree.num_nodes != actual_tree.num_nodes or expected_tree.height() != actual_tree.height():
        print("Missing processes")
        return False
    return _compare_tree_helper(expected_tree, actual_tree)

def _compare_tree_helper(expected_tree: NAryTree, actual_tree: NAryTree):
    same_children = len(expected_tree.children) == len(actual_tree.children)
    # checking subtrees
    for child in expected_tree.children:
        match = False
        i = 0
        while not match and i < len(actual_tree.children):
            child2 = actual_tree.children[i]
            if _compare_tree_helper(child, child2):
                match = True
            i += 1
        if not match:
            return False

    same_exit = expected_process_dict[expected_tree.pid]["exit"] == actual_process_dict[actual_tree.pid]["exit"]
    same_fds =  expected_process_dict[expected_tree.pid]["num_fds"] == actual_process_dict[actual_tree.pid]["num_fds"]

    # same content written
    expected_inode_list = list(expected_log_dict.get(expected_tree.pid, {}).keys())
    actual_inode_list =  list(actual_log_dict.get(actual_tree.pid, {}).keys())
    for inode in expected_inode_list:
        match = False
        i = 0
        while not match and i < len(actual_inode_list):
            inode2 = actual_inode_list[i]
            if expected_log_dict[expected_tree.pid][inode] == actual_log_dict[actual_tree.pid][inode2]:
                match = True
            i += 1
        if not match:
            return False # differing output

    return same_children and same_exit and same_fds
            
if __name__ == '__main__':
    # Driver Code
    # run ./pdt on actual program and expected program
    expected_program_name = input("Expected program's name: ")
    actual_program_name = input("Actual program's name: ")

    dict_tuple = traceProgram(expected_program_name, expected_process_dict, expected_log_dict, {})
    expected_process_dict = dict_tuple[0].copy()
    expected_log_dict = dict_tuple[1].copy()
    dict2_tuple = traceProgram(actual_program_name, actual_process_dict, actual_log_dict, {})
    tree = generate_tree(expected_process_dict)
    actual_process_dict = dict2_tuple[0].copy()
    actual_log_dict = dict2_tuple[1].copy()
    tree2 = generate_tree(actual_process_dict)
    tree.print_tree()
    tree2.print_tree()
    print(compare_tree(tree, tree2))



