#include <stdlib.h>

#include "scil-decision-tree.h"

scilU_decision_tree* scilU_tree_create(int* left, int* right, double* thresholds, int* indices, int** classes, char** class_names, int amount_classes){
  scilU_decision_tree* tree = malloc(sizeof(scilU_decision_tree));
  tree->left_children = left;
  tree->right_children = right;
  tree->thresholds = thresholds;
  tree->indices = indices;
  tree->classes = classes;
  tree->class_names = class_names;
  tree->amount_classes = amount_classes;
  return tree;
}

int scilU_tree_findMax(int* classes, int amount_classes){
  int index = 0;
  for (int i = 0; i < amount_classes; i++) {
    index = classes[i] > classes[index] ? i : index;
  }
  return index;
}

char* scilU_tree_predict(scilU_decision_tree* tree, int node, double*features){
  if (tree->thresholds[node] != -2.0) {
    if (features[tree->indices[node]] <= tree->thresholds[node]) {
      return scilU_tree_predict(tree, tree->left_children[node], features);
    } else {
      return scilU_tree_predict(tree, tree->right_children[node], features);
    }
  }
  return tree->class_names[scilU_tree_findMax(tree->classes[node], tree->amount_classes)];
}

void scilU_tree_remove(scilU_decision_tree* tree){
  free(tree->left_children);
  free(tree->right_children);
  free(tree->thresholds);
  free(tree->classes);
  free(tree->class_names);

  free(tree);
}


