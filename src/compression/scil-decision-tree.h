#ifndef SCIL_DECISION_TREE_H
#define SCIL_DECISION_TREE_H

typedef struct {
  int* left_children;
  int* right_children;
  double* thresholds;
  int* indices;
  int** classes;
  int amount_classes;
  char** class_names;
} scilU_decision_tree;

scilU_decision_tree* scilU_tree_create(int* left, int* right, double* thresholds, int* indices, int** classes, char** class_names, int amount_classes);
void scilU_tree_remove(scilU_decision_tree* tree);
int scilU_tree_findMax(int* classes, int amount_classes);
char* scilU_tree_predict(scilU_decision_tree* tree, int node, double* features);
#endif //SCIL_DECISION_TREE_H

