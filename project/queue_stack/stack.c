#include <stdio.h>
#include <stdlib.h>

typedef struct Node
{
    int data;
    struct Node *next;
} Node;

typedef struct Stack
{
    Node *top;
} Stack;

Stack *create_stack()
{
    Stack *s = malloc(sizeof(Stack));
    s->top = NULL;
    return s;
}

void push(Stack *s, int value)
{
    Node *new_node = malloc(sizeof(Node));
    new_node->data = value;
    new_node->next = s->top;
    s->top = new_node;
}

int pop(Stack *s)
{
    if (s->top == NULL)
    {
        printf("Empty stack!\n");
        return -1;
    }

    Node *temp = s->top;
    int value = temp->data;
    s->top = s->top->next;
    free(temp);
    return value;
}

int peek(Stack *s)
{
    if (s->top == NULL)
    {
        printf("Empty Stack!\n");
        return -1;
    }
    return s->top->data;
}

int is_empty(Stack *s)
{
    return s->top == NULL;
}

void print_stack(Stack *s)
{
    Node *curr = s->top;
    printf("Stack (top -> bottom): ");
    while (curr != NULL)
    {
        printf("%d ", curr->data);
        curr = curr->next;
    }
    printf("\n");
}

void free_stack(Stack *s)
{
    while (!is_empty(s))
    {
        pop(s);
    }
    free(s);
}

int main()
{
    Stack *s = create_stack();

    while (1)
    {
        int op;
        printf("\n1) Push\n2) Pop\n3) Peek\n4) Print stack\n5) Exit\n> ");
        scanf("%d", &op);

        switch (op)
        {
        case 1:
        {
            int value;
            printf("Enter a value: ");
            scanf("%d", &value);
            push(s, value);
            break;
        }
        case 2:
            printf("Pop: %d\n", pop(s));
            break;
        case 3:
            printf("Top: %d\n", peek(s));
            break;
        case 4:
            print_stack(s);
            break;
        default:
            free_stack(s);
            return 0;
        }
    }
}