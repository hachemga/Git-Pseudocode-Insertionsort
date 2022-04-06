#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "introprog_telefonbuch.h"

/*
 * Fügt einen Knoten mit der Telefonnummer phone und dem Namen name
 * in den Binären Suchbaum bst ein.  Für den Suchbaum soll die
 * Eigenschaft gelten, dass alle linken Kinder einen Wert kleiner
 * gleich (<=) und alle rechten Kinder einen Wert größer (>) haben.
 */
void bst_insert_node(bstree* bst, unsigned long phone, char *name) {
    bst_node* bsst=malloc(sizeof(bst_node));
    bsst->name=malloc(sizeof(char)*(strlen(name)+1));
    strcpy(bsst->name,name);
    bsst->phone=phone;
    bsst->left=NULL;
    bsst->right=NULL;
    bsst->parent=NULL;
    bst_node* y;
    y=NULL;
    bst_node* x;
    x=bst->root;
    if (bst == NULL){
        return;
    }
    if (bst->root == NULL)
    {
        bst->root=bsst;
    }
    while ((x != NULL) && (x->phone !=phone)){
            y=x;
            if (bsst->phone <= x->phone){
                x=x->left;
            }
            else{
                x=x->right;
            }
    }
        if (x != NULL) {
            return;
        }

        bsst->parent=y;
        if (y==NULL) {
            bst->root=bsst;
        }
        else if (bsst->phone < y->phone){
            y->left=bsst;
        }
        else {
            y->right=bsst;
        }
}
    


/*
 * Diese Funktion liefert einen Zeiger auf einen Knoten im Baum mit
 * dem Wert phone zurück, falls dieser existiert.  Ansonsten wird
 * NULL zurückgegeben.
 */
bst_node* find_node(bstree* bst, unsigned long phone) {
    bst_node* x=bst->root;
    while ((x != NULL) && (phone != x->phone) ){
        if(phone < x->phone){
            x=x->left;
        }
        else {
            x=x->right;
        }
    }
    return x;
}

/* Gibt den Unterbaum von node in "in-order" Reihenfolge aus */
void bst_in_order_walk_node(bst_node* node) {
        if (node->left != NULL){
            bst_in_order_walk_node(node->left);
        }
        print_node(node);
        if (node->right != NULL){
            bst_in_order_walk_node(node->right);
        }
    
    return;
}

/* 
 * Gibt den gesamten Baum bst in "in-order" Reihenfolge aus.  Die
 * Ausgabe dieser Funktion muss aufsteigend soriert sein.
 */
void bst_in_order_walk(bstree* bst) {
    if (bst != NULL) {
        bst_in_order_walk_node(bst->root);
    }
}

/*
 * Löscht den Teilbaum unterhalb des Knotens node rekursiv durch
 * "post-order" Traversierung, d.h. zurerst wird der linke und dann
 * der rechte Teilbaum gelöscht.  Anschließend wird der übergebene
 * Knoten gelöscht.
 */
void bst_free_subtree(bst_node* node) {
    if (node !=NULL){
        bst_free_subtree(node->left);
        bst_free_subtree(node->right);
        free(node->name);
        free(node);
    }
    return;
}

/* 
 * Löscht den gesamten Baum bst und gibt den entsprechenden
 * Speicher frei.
 */
void bst_free_tree(bstree* bst) {
    if(bst != NULL && bst->root != NULL) {
        bst_free_subtree(bst->root);
        bst->root = NULL;
    }
}
