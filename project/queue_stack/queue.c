#include <stdio.h>
#include <stdlib.h>

typedef struct Node
{
    int data;
    struct Node *next;
} Node;

typedef struct Queue
{
    Node *start;
    Node *end;
} Queue;

Queue *createQueue()
{
    Queue *q = malloc(sizeof(Queue));
    q->start = q->end = NULL;
    return q;
}

void enqueue(Queue *q, int value)
{
    Node *val = malloc(sizeof(Node));
    val->data = value;
    val->next = NULL;

    if (q->end == NULL)
    {
        q->start = q->end = val;
        return;
    }
    q->end->next = val;
    q->end = val;
}

int dequeue(Queue *q)
{
    if (q->start == NULL)
    {
        printf("Empty queue\n");
        return -1;
    }

    Node *val = q->start;
    int value = val->data;
    q->start = q->start->next;

    if (q->start == NULL)
        q->end = NULL;

    free(val);
    return value;
}

void print_queue(Queue *q)
{
    Node *val = q->start;
    printf("Queue: ");
    while (val)
    {
        printf("%d ", val->data);
        val = val->next;
    }
    printf("\n");
}

void free_queue(Queue *q)
{
    while (q->start != NULL)
        dequeue(q);
    free(q);
}

int main()
{
    Queue *q = createQueue();

    do
    {
        int op;
        printf("1) Enqueue\n2) Dequeue\n3) Print queue\n4) Exit\n");
        scanf("%d", &op);

        switch (op)
        {
        case 1:
        {
            int value;
            printf("Enter a value: ");
            scanf("%d", &value);
            enqueue(q, value);
            break;
        }
        case 2:
            printf("Value: %d\n", dequeue(q));
            break;
        case 3:
            print_queue(q);
            break;
        default:
            free_queue(q);
            return 0;
        }
    } while (1);
}