#include "dead_ll.h"
#include <stdlib.h>
#include <stdio.h>

DNode* insert_dnode(DNode* node, int exit_status, AVLNode *dead){
    DNode * new_node = (DNode*) malloc(sizeof(DNode));
    if (new_node == NULL) {
        perror("malloc");
        return node;
    }

    new_node->pid = dead->pid;
    new_node->start_time = dead->start_time;
    new_node->end_time = dead->end_time;
    new_node->exit_status = exit_status;
    new_node->seg_fault = dead->seg_fault;
    new_node->open_fds = dead->open_fds;
    new_node->num_children = dead->num_children;
    new_node->num_fds = dead->num_fds;
    new_node->num_open_fds = dead->num_open_fds;
    new_node->child = dead->child;
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

void free_list(DNode* head){
    DNode* curr = head;
    DNode* temp;
    while(curr != NULL){
        temp = curr->next;
        free_the_children(curr->child);
        free_fd_list(curr->open_fds);
        free(curr);
        curr = temp;
    }
}
