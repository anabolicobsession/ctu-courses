typedef struct node {
    int x;
    char *text;
    struct node *next;
} node_t;

typedef struct {
    node_t *tail;
    node_t *head;
    int size;
} linked_list_t;

void linked_list_init(linked_list_t *ll);

void linked_list_push(linked_list_t *ll, int x, char *text);

node_t *linked_list_pop(linked_list_t *ll);

void linked_list_clear_node(node_t *node);

void linked_list_clear(linked_list_t *ll);

void linked_list_print(linked_list_t *ll);
