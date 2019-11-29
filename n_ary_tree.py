class NAryTree:


    def __init__(self, pid):
        self.pid = pid
        self.children = []
        self.num_nodes = 1
    
    def insert(self, pid, parent):
        if(parent == self.pid):
            pid = pid
            self.children.append(NAryTree(pid))
            self.num_nodes += 1
            return 0

        for child in self.children:
            if child.insert(pid, parent) == 0:
                self.num_nodes += 1
                return 0
        return -1
    def print_tree(self):
        _print_tree_helper(self, 0)

    def get_nodes_on_level_k(self, k: int):
        if k == 0:
            return [self]
        lst = []
        for child in self.children:
            lst.extend(child.get_nodes_on_level_k(k - 1))
        return lst
    def height(self):
        if self.children == []:
            return 1
        return 1 + max([child.height() for child in self.children])

def _print_tree_helper(tree, level):
    height = tree.height()
    while level != height:
        print("Level {}: ".format(level), end = " ")
        print(" ".join([child.pid for child in tree.get_nodes_on_level_k(level)]))
        level += 1
            
if __name__ == "__main__":
    # driver code
    node1 = NAryTree("12")
    node1.insert("13", "12")
    node1.insert("14", "13")
    node1.insert("15", "12")
    node1.print_tree()

