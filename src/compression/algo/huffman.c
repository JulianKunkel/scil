// Huffman code implementation
// Used as prefix for SCIL allquant algorithm
// Author: Oliver Pola <5pola@informatik.uni-hamburg.de>

#include <algo/huffman.h>
#include <scil-util.h>

typedef struct huffman_node huffman_node;
struct huffman_node {
  huffman_entity* entity;
  // don't want to create entities for "virtual" ones, so copy the count
  size_t count;
  huffman_node* queue_next;
  huffman_node* tree_left;
  huffman_node* tree_right;
};

void huffman_enqueue(huffman_node** queue, int* queuecount,
                     huffman_node* newnode) {
  huffman_node* pre = NULL;
  huffman_node* post = *queue;
  while(post != NULL && post->count < newnode->count) {
    pre = post;
    post = post->queue_next;
  }
  // post points to entry with count >= newnode->count
  // pre points to entry before
  newnode->queue_next = post;
  if(pre == NULL)
    *queue = newnode;
  else
    pre->queue_next = newnode;
  (*queuecount)++;
}

huffman_node* huffman_dequeue(huffman_node** queue, int* queuecount) {
  huffman_node* result = *queue;
  *queue = result->queue_next;
  result->queue_next = NULL;
  (*queuecount)--;
  return result;
}

void huffman_tree_traverse(huffman_node* node,
                      uint8_t  bitmask, uint8_t bitvalue, uint8_t bitcount) {
  if(node->tree_left == NULL || node->tree_right == NULL) {
    node->entity->bitmask = bitmask;
    node->entity->bitvalue = bitvalue;
    node->entity->bitcount = bitcount;
  } else {
    bitcount++;
    uint8_t shifts = 8 - bitcount;
    uint8_t one = 1 << shifts;
    bitmask = bitmask | one;
    huffman_tree_traverse(node->tree_left, bitmask, bitvalue, bitcount);
    bitvalue = bitvalue | one;
    huffman_tree_traverse(node->tree_right, bitmask, bitvalue, bitcount);
  }
}

void huffman_tree_delete(huffman_node* node) {
  if(node == NULL) return;
  huffman_tree_delete(node->tree_left);
  huffman_tree_delete(node->tree_right);
  free(node);
}

void huffman_encode(huffman_entity* entities, size_t size) {
  if(size < 1) return;

  // the priority queue of nodes to be added to the tree
  // sorted by least count first
  huffman_node* queue = NULL;
  int queuecount = 0;

  // enqueue all given entitiies
  for(size_t i = 0; i < size; i++) {
    if(entities[i].count > 0) {
      huffman_node* newnode = (huffman_node*)scilU_safe_malloc(sizeof(huffman_node));
      newnode->entity = &entities[i];
      newnode->count = entities[i].count;
      newnode->queue_next = NULL;
      newnode->tree_left = NULL;
      newnode->tree_right = NULL;
      huffman_enqueue(&queue, &queuecount, newnode);
    } else {
      entities[i].bitmask = 0;
      entities[i].bitvalue = 1; // will never fit, masked with 0
      entities[i].bitcount = 0;
    }
  }

  // stupid idea: all may have had count = 0
  if(queuecount == 0) return;

  // pick first two and combine to new "virtual" node
  while(queuecount > 1) {
    huffman_node* left = huffman_dequeue(&queue, &queuecount);
    huffman_node* right = huffman_dequeue(&queue, &queuecount);
    huffman_node* merge = (huffman_node*)scilU_safe_malloc(sizeof(huffman_node));
    merge->entity = NULL;
    merge->count = left->count + right->count;
    merge->queue_next = NULL;
    merge->tree_left = left;
    merge->tree_right = right;
    huffman_enqueue(&queue, &queuecount, merge);
  }

  // now there is only one left in queue, that's parent of huffman tree
  // traverse and generate bitpatterns
  huffman_tree_traverse(queue, 0, 0, 0);

  // cleanup
  huffman_tree_delete(queue);
}
