#include "../include/tournament.h"

void queue_init(MatchQueue *q) {
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
}

void queue_enqueue(MatchQueue *q, Match *m) {
    QueueNode *node = (QueueNode *)malloc(sizeof(QueueNode));
    if (!node) return;
    node->match = m;
    node->next = NULL;

    if (q->rear) {
        q->rear->next = node;
    } else {
        q->front = node;
    }
    q->rear = node;
    q->size++;
}

Match *queue_dequeue(MatchQueue *q) {
    if (!q->front) return NULL;

    QueueNode *node = q->front;
    Match *m = node->match;
    q->front = node->next;

    if (!q->front) {
        q->rear = NULL;
    }
    free(node);
    q->size--;
    return m;
}

Match *queue_peek(MatchQueue *q) {
    return q->front ? q->front->match : NULL;
}

int queue_is_empty(MatchQueue *q) {
    return q->size == 0;
}

void queue_build_pending(MatchQueue *q, Match *head) {
    queue_free(q);
    Match *curr = head;
    while (curr) {
        if (curr->status == MATCH_PENDING) {
            queue_enqueue(q, curr);
        }
        curr = curr->next;
    }
}

void queue_free(MatchQueue *q) {
    while (q->front) {
        QueueNode *node = q->front;
        q->front = node->next;
        free(node);
    }
    q->rear = NULL;
    q->size = 0;
}
