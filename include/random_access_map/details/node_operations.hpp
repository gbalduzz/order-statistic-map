// Copyright (C) 2020 Giovanni Balduzzi
// All rights reserved.
//
// See LICENSE for terms of usage.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//
// Implement the fix to RB tree violoation.
// Code inspired from https://www.geeksforgeeks.org/red-black-tree-set-1-introduction-2/

#pragma once

#include "color.hpp"

namespace ramlib {
namespace details {

template <class Node>
const auto& get_key(const Node* const node) {
  return node->data.first;
}

template <class Node>
bool isLeftChild(const Node* node) {
  return node->parent && node->parent->left == node;
}

template <class Node>
bool isRightChild(const Node* node) {
  return node->parent && node->parent->right == node;
}

template <class Node>
auto getUncle(const Node* node) -> Node* {
  const Node* parent = node->parent;
  if (isLeftChild(parent))
    return parent->parent->right;
  else if (isRightChild(parent))
    return parent->parent->left;
  else
    return nullptr;
}

template <class Node>
auto getSibling(const Node* node) -> Node* {
  if (isLeftChild(node))
    return node->parent->right;
  else if (isRightChild(node))
    return node->parent->left;
  else
    return nullptr;
}

template <class Node>
void moveDown(Node* node, Node* new_parent) {
  auto& parent = node->parent;
  if (isLeftChild(node)) {
    parent->left = new_parent;
  }
  else if (isRightChild(node)) {
    parent->right = new_parent;
  }
  new_parent->parent = parent;
  parent = new_parent;
}

template <class Node>
void fixDoubleBlack(Node* x, Node*& root) {
  if (x == root) {  // Reached root
    return;
  }

  Node* sibling = getSibling(x);
  Node* parent = x->parent;

  auto has_red_child = [](Node* n) {
    return (n->left != nullptr && n->left->color == RED) ||
           (n->right != nullptr && n->right->color == RED);
  };

  if (sibling == nullptr) {
    // No sibiling, double black pushed up
    fixDoubleBlack(parent, root);
  }
  else {
    if (sibling->color == RED) {
      // Sibling red
      parent->color = RED;
      sibling->color = BLACK;
      if (isLeftChild(sibling)) {
        // left case
        rightRotate(parent, root);
      }
      else {
        // right case
        leftRotate(parent, root);
      }
      fixDoubleBlack(x, root);
    }
    else {
      // Sibling black
      if (has_red_child(sibling)) {
        // at least 1 red children
        if (sibling->left != nullptr and sibling->left->color == RED) {
          if (isLeftChild(sibling)) {
            // left left
            sibling->left->color = sibling->color;
            sibling->color = parent->color;
            rightRotate(parent, root);
          }
          else {
            // right left
            sibling->left->color = parent->color;
            rightRotate(sibling, root);
            leftRotate(parent, root);
          }
        }
        else {
          if (isLeftChild(sibling)) {
            // left right
            sibling->right->color = parent->color;
            leftRotate(sibling, root);
            rightRotate(parent, root);
          }
          else {
            // right right
            sibling->right->color = sibling->color;
            sibling->color = parent->color;
            leftRotate(parent, root);
          }
        }
        parent->color = BLACK;
      }
      else {
        // 2 black children
        sibling->color = RED;
        if (parent->color == BLACK)
          fixDoubleBlack(parent, root);
        else
          parent->color = BLACK;
      }
    }
  }
}

template <class Node>
void rightRotate(Node* const node, Node*& root) {
  // new parent will be node's left child
  Node* new_parent = node->left;

  // update root if current node is root
  if (node == root)
    root = new_parent;

  moveDown(node, new_parent);

  // connect node with new parent's right element
  node->left = new_parent->right;
  // connect new parent's right element with node
  // if it is not nullptr
  if (new_parent->right != nullptr)
    new_parent->right->parent = node;

  // connect new parent with node
  new_parent->right = node;

  node->updateSubtreeWeight();
  new_parent->updateSubtreeWeight();
}

template <class Node>
void leftRotate(Node* node, Node*& root) {
  // new parent will be node's right child
  Node* new_parent = node->right;

  // update root_ if current node is root_
  if (node == root)
    root = new_parent;

  moveDown(node, new_parent);

  // connect node with new parent's left element
  node->right = new_parent->left;
  // connect new parent's left element with node
  // if it is not nullptr
  if (new_parent->left != nullptr)
    new_parent->left->parent = node;

  // connect new parent with node
  new_parent->left = node;

  node->updateSubtreeWeight();
  new_parent->updateSubtreeWeight();
}

template <class Node>
void fixRedRed(Node* x, Node*& root) {
  // if x is root color it black and return
  if (x == root) {
    x->color = BLACK;
    return;
  }

  // initialize relatives.
  Node* parent = x->parent;
  Node* grandparent = parent->parent;
  Node* uncle = getUncle(x);

  if (parent->color != BLACK) {
    if (uncle && uncle->color == RED) {
      // uncle is red, perform recoloring and recurse
      parent->color = BLACK;
      uncle->color = BLACK;
      grandparent->color = RED;
      fixRedRed(grandparent, root);
    }
    else {
      if (isLeftChild(parent)) {
        if (isLeftChild(x)) {
          // for left right
          std::swap(parent->color, grandparent->color);
        }
        else {
          leftRotate(parent, root);
          std::swap(x->color, grandparent->color);
        }
        // for left left and left right
        rightRotate(grandparent, root);
      }
      else {
        if (isLeftChild(x)) {
          // for right left
          rightRotate(parent, root);
          std::swap(x->color, grandparent->color);
        }
        else {
          std::swap(parent->color, grandparent->color);
        }

        // for right right and right left
        leftRotate(grandparent, root);
      }
    }
  }
}
template <class Node>
void removeNoDoubleChild(Node* to_delete, Node*& root) noexcept {
  Node* replacement = to_delete->left ? to_delete->left : to_delete->right;

  auto color = [](const Node* n) { return n ? n->color : BLACK; };
  const bool both_black = color(replacement) == BLACK && to_delete->color == BLACK;

  if (both_black) {
    fixDoubleBlack(to_delete, root);
  }
  else {
    auto sibling = getSibling(to_delete);
    if (sibling && !replacement)
      sibling->color = RED;
    else if (replacement)
      replacement->color = BLACK;
  }

  // delete to_delete from the tree
  Node* parent = to_delete->parent;
  if (isLeftChild(to_delete)) {
    parent->left = replacement;
  }
  else if (isRightChild(to_delete)) {
    parent->right = replacement;
  }
  if (replacement)
    replacement->parent = parent;

  // Update root if necessary.
  if (to_delete == root) {
    root = replacement;
  }
}

template <class Node>
void reconnect(Node* node, Node* old_pos) {
  if (node->parent && node->parent->left == old_pos)
    node->parent->left = node;
  else if (node->parent && node->parent->right == old_pos)
    node->parent->right = node;

  if (node->right)
    node->right->parent = node;
  if (node->left)
    node->left->parent = node;
}

template <class Node>
void swapParentChild(Node* p, Node* c) {
  if (isLeftChild(c)) {
    std::swap(c->right, p->right);
    p->left = c->left;
    c->left = p;
  }
  else {
    std::swap(c->left, p->left);
    p->right = c->right;
    c->right = p;
  }

  c->parent = p->parent;
  p->parent = c;
}
template <class Node>
void swap(Node* a, Node* b, Node*& root) {
  if (root == a)
    root = b;

  if (b->parent == a) {
    swapParentChild(a, b);
  }
  else {
    // Swap pointers
    std::swap(a->left, b->left);
    std::swap(a->right, b->right);
    std::swap(a->parent, b->parent);
  }

  reconnect(a, b);
  reconnect(b, a);

  assert(a->parent != a);
  assert(b->parent != b);

  a->swapMetadata(*b);
  std::swap(a->color, b->color);
}

}  // namespace details
}  // namespace ramlib
