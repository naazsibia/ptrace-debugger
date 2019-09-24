#include "dead_ll.h"
#include <stdlib.h>
#include <stdio.h>

DNode* insert_dnode(DNode* node, pid_t pid, int exit_status) {
    DNode * new_node = (DNode*) malloc(sizeof(DNode));
    if (new_node == NULL) {
        perror("malloc");
        return node;
    }

    new_node->pid = pid;
    new_node->exit_status = exit_status;
    new_node->next = NULL;

    if (node == NULL) {
        return new_node;
    }

    DNode* temp = node;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = new_node;
    return node;
}


