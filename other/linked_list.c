/**
 * Linked List basics
 * gcc linked_list.c -o llist
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _node
{
	int data;
	struct _node* next;
} node;

void print_list(node* head)
{
	int cnt = 0;
	if (head != NULL) {
		node* n = head;
		while (n != NULL) {
			printf("NODE %d: %p->%p (%d)\n", cnt++, n, n->next, n->data);
			n = n->next;
		}
	} else {
		printf("LIST IS EMPTY\n");
	}
}

int insert_node(node** head, int data)
{
	node* new_node = (node*)malloc(sizeof(node));
	if (new_node == NULL)
		return 0;

	new_node->data = data;
	new_node->next = *head;
	*head = new_node;
}

int delete_node(node** head, int del)
{
	node* tmp;
	printf("\tDELETING NODE %d\n", del);
	if (*head == NULL)
		return 0;
	tmp = *head;
	if (tmp->data == del) {
		// delete head case
		*head = tmp->next;
		free(tmp);
		return (1);
	} else {
		while (tmp->next != NULL) {
			if (tmp->next->data == del) {
				node* n = tmp->next;
				tmp->next = n->next;
				free(n);
				return (1);
			}
			tmp = tmp->next;
		}
		printf("\tNOT FOUND\n");
		return (0);
	}
}

int main()
{
	int cnt = 0;
	node* head = NULL;
	print_list(head);
	for (cnt = 10; cnt < 20; cnt++) {
		insert_node(&head, cnt);
	}
	print_list(head);
	delete_node(&head, 15);
	print_list(head);
	delete_node(&head, 19);
	print_list(head);
	delete_node(&head, 919);
	print_list(head);
	return 0;
}
