#ifndef __AVLTREE_T
#define __AVLTREE_T

#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <utility>

namespace avltree
{
   namespace exception {
      class Exception : public std::exception
      {
      public:
         std::string error;

         Exception() : std::exception() {}
         Exception(std::string error) : error(error), std::exception() {}

         const char *what() const noexcept {
            return this->error.c_str();
         }
      };

      class NullPointer : public Exception {
      public:
         NullPointer() : Exception("Encountered an unexpected null pointer.") {}
      };

      class NodeLabelsMatch : public Exception {
      public:
         NodeLabelsMatch() : Exception("Node labels unexpectedly matched.") {}
      };

      class LabelExists : public Exception {
      public:
         LabelExists() : Exception("The label already exists in the tree.") {}
      };

      class EmptyTree : public Exception {
      public:
         EmptyTree() : Exception("The tree is empty.") {}
      };

      class NodeNotFound : public Exception {
      public:
         NodeNotFound() : Exception("The node was not found.") {}
      };
   }
   
   template <typename Label, typename Compare = std::less<Label>>
   class AVLTree
   {
   protected:
      struct Node
      {
         using Shared = std::shared_ptr<Node>;
      
         Label label;
         int height;
         Shared parent, left, right;

         Node(Label label, Shared parent=nullptr, Shared left=nullptr, Shared right=nullptr)
            : label(label), parent(parent), left(left), right(right), height(0) {}

         bool is_leaf() const { return this->left == nullptr && this->right == nullptr; }

         int compare(const Label &label) const {
            auto ab = Compare()(label, this->label);
            auto ba = Compare()(this->label, label);

            if (!ab && !ba) { return 0; }
            if (ab && !ba) { return -1; }
            else { return 1; }
         }

         int compare(const Node::Shared &node) const {
            if (node == nullptr) { throw exception::NullPointer(); }

            return this->compare(node->label)
         }

         int balance() const {
            auto left_height = (this->left != nullptr) ? this->left->height : 0;
            auto right_height = (this->right != nullptr) ? this->right->height : 0;

            return right_height - left_height;
         }
                        
         int new_height() const {
            auto left_height = (this->left != nullptr) ? this->left->height : 0;
            auto right_height = (this->right != nullptr) ? this->right->height : 0;

            return std::max(left_height,right_height)+1;
         }
      };

      typename Node::Shared root;

      void set_right_child(typename Node::Shared &target, typename Node::Shared &child) {
         if (target == nullptr) { throw exception::NullPointer(); }
         
         target->right = child;

         if (child != nullptr)
            child->parent = target;
      }

      void set_left_child(typename Node::Shared &target, typename Node::Shared &child) {
         if (target == nullptr) { throw exception::NullPointer(); }

         target->left = child;

         if (child != nullptr)
            child->parent = target;
      }

      void set_parent(typename Node::Shared &target, typename Node::Shared &parent) {
         if (target == nullptr) { throw exception::NullPointer(); }

         if (parent == nullptr) { 
            target->parent = parent;
            return;
         }

         auto branch = parent.compare(target);

         if branch == 0 { throw exception::NodeLabelsMatch(); }
         else if branch < 0 { parent->left = target; }
         else { parent->right = target; }
      }

      void rotate_left(typename Node::Shared &rotation_root) {
         if (rotation_root == nullptr) { throw exception::NullPointer(); }

         std::cout << "Rotating node " << rotation_root->label << " left" << std::endl;
         
         auto pivot_root = rotation_root->right;
         if (pivot_root != nullptr) std::cout << "\tPivot root: " << pivot_root->label << std::endl;
         
         auto left_child = pivot_root->left;
         if (left_child != nullptr) std::cout << "\tLeft child: " << left_child->label << std::endl;
         
         auto rotation_parent = rotation_root->parent;
         if (rotation_parent != nullptr) std::cout << "\tRotation parent: " << rotation_parent->label << std::endl;

         if (rotation_parent == nullptr)
            this->root = pivot_root;
         else if (rotation_parent->left == rotation_root)
            rotation_parent->left = pivot_root;
         else if (rotation_parent->right == rotation_root)
            rotation_parent->right = pivot_root;

         pivot_root->parent = rotation_parent;

         if (left_child != nullptr)
         {
            left_child->parent = rotation_root;
         }

         rotation_root->right = left_child;
         pivot_root->left = rotation_root;
         rotation_root->parent = pivot_root;

         rotation_root->height = rotation_root->new_height();
         pivot_root->height = pivot_root->new_height();
      }

      void rotate_right(typename Node::Shared &rotation_root) {
         if (rotation_root == nullptr) { throw exception::NullPointer(); }
         
         std::cout << "Rotating node " << rotation_root->label << " right" << std::endl;
         
         auto pivot_root = rotation_root->left;
         if (pivot_root != nullptr) std::cout << "\tPivot root: " << pivot_root->label << std::endl;
         
         auto right_child = pivot_root->right;
         if (right_child != nullptr) std::cout << "\tRight child: " << right_child->label << std::endl;
         
         auto rotation_parent = rotation_root->parent;
         if (rotation_parent != nullptr) std::cout << "\tRotation parent: " << rotation_parent->label << std::endl;

         if (rotation_parent == nullptr)
            this->root = pivot_root;
         else if (rotation_parent->left == rotation_root)
            rotation_parent->left = pivot_root;
         else if (rotation_parent->right == rotation_root)
            rotation_parent->right = pivot_root;

         pivot_root->parent = rotation_parent;

         if (right_child != nullptr)
            right_child->parent = rotation_root;

         rotation_root->left = right_child;
         pivot_root->right = rotation_root;
         rotation_root->parent = pivot_root;

         rotation_root->height = rotation_root->new_height();
         pivot_root->height = pivot_root->new_height();
      }

      void rebalance_node(typename Node::Shared &node) {
         if (node == nullptr) { throw exception::NullPointer(); }
         
         auto balance = node->balance();
         if (balance == 0) { return; }

         if (balance < 0)
         {
            auto child = node->left;
            auto balance = (child != nullptr) ? child->balance() : 0;

            if (balance > 0)
            {
               this->rotate_left(child);
               this->rotate_right(node);
            }
            else {
               this->rotate_right(node);
            }
         }
         else
         {
            auto child = node->right;
            auto balance = (child != nullptr) ? child->balance() : 0;

            if (balance < 0) {
               this->rotate_right(child);
               this->rotate_left(node);
            }
            else {
               this->rotate_left(node);
            }
         }
      }

      void update_height(typename Node::Shared &node) {
         if (node == nullptr) { throw exception::NullPointer(); }
         
         Node::Shared update = node;

         while (update != nullptr)
         {
            auto old_height = update->height;
            auto new_height = update->new_height();
            auto balance = update->balance();
            
            update->height = new_height;

            if (balance > 1 || balance < -1)
            {
               this->rebalance_node(update);
               return;
            }

            if (old_height == new_height) { return; }

            update = update->parent;
         }
      }

      std::vector<std::pair<typename Node::Shared, int>> search(const Label &label) const {
         auto result = std::vector<std::pair<typename Node::Shared, int>>();
         if (this->root == nullptr) { return result; }
         
         auto node = this->root;
         auto branch = node->compare(label);
         
         while (branch != 0 && node != nullptr)
         {
            result.push_back(std::make_pair(node, branch));

            if (branch < 0)
               node = node->left;
            else
               node = node->right;

            if (node != nullptr)
               branch = node->compare(label);
         }
            
         if (node != nullptr) { result.push_back(std::make_pair(node, branch)); }

         return result;
      }

      typename Node::Shared add_node(Label &label) {
         if (this->root == nullptr)
         {
            this->root = std::make_shared<Node>(label);
            return this->root;
         }

         auto traversal = this->search(label);
         auto last_result = *traversal.rbegin();
         auto parent = last_result.first;
         auto branch = last_result.second;
         
         if (branch == 0) { throw exception::LabelExists(); }

         auto new_node = std::make_shared<Node>(label, parent);

         if (branch < 0) { parent->left = new_node; }
         else { parent->right = new_node; }

         this->update_height(new_node);

         return new_node;
      }

      void remove_node(const Label &label) {
         if (this->is_empty()) { throw exception::EmptyTree(); }

         auto traversal = this->search(label);
         auto last_node = *traversal.rbegin();

         if (last_node.second != 0) { throw exception::NodeNotFound(); }

         auto node = last_node.first;

         if (node->is_leaf())
         {
            if (node->parent != nullptr)
            {
               if (node->parent->left == node)
                  node->parent->left.reset();
               else if (node->parent->right == node)
                  node->parent->right.reset();
            }

            if (node == this->root)
            {
               this->root.reset();
            }
            else
            {
               auto parent = node->parent;
               node->parent.reset();

               this->update_height(parent);
            }
         }
         else if (node->left == nullptr || node->right == nullptr)
         {
            Node::Shared replacement_node;

            if (node->left == nullptr)
               replacement_node = node->right;
            else
               replacement_node = node->left;

            replacement_node->parent = node->parent;

            if (node == this->root)
            {
               this->root = replacement_node;
               this->update_height(this->root);
            }
            else if (node->parent->left == node)
            {
               node->parent->left = replacement_node;
               this->update_height(node->parent);
            }
            else if (node->parent->right == node)
            {
               node->parent->right = replacement_node;
               this->update_height(node->parent);
            }
         }
         else if (node->left != nullptr && node->right != nullptr)
         {
            auto leftmost = node->right;

            while (leftmost->left != nullptr)
               leftmost = leftmost->left;

            auto replaced_node = leftmost->right;
            auto leftmost_parent = leftmost->parent;

            if (leftmost_parent == node) {
               node->right = replaced_node;
            }
            else {
               leftmost->parent->left = replaced_node;

               if (replaced_node != nullptr)
                  replaced_node->parent = leftmost->parent;
            }

            leftmost->right = node->right;
            leftmost->left = node->left;
            leftmost->parent = node->parent;
            leftmost->height = node->height;

            if (leftmost->left != nullptr)
               leftmost->left->parent = leftmost;

            if (leftmost->right != nullptr)
               leftmost->right->parent = leftmost;

            if (leftmost->parent != nullptr)
            {
               if (node == leftmost->parent->left)
                  leftmost->parent->left = leftmost;
               else if (node == leftmost->parent->right)
                  leftmost->parent->right = leftmost;
            }

            if (node == this->root)
               this->root = leftmost;

            if (replaced_node != nullptr)
               this->update_height(replaced_node);
            else if (leftmost_parent != node)
               this->update_height(leftmost_parent);
            else
               this->update_height(leftmost);
         }

         node->left.reset();
         node->right.reset();
         node->parent.reset();
      }

   public:
      class inorder_iterator
      {
         friend AVLTree;

      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = const Label;
         using pointer = const Label *;
         using reference = const Label &;

         inorder_iterator(typename Node::Shared node) : node(node) {
            if (this->node == nullptr) return;
            
            while (this->node->left != nullptr)
            {
               this->node = this->node->left;
            }
         }
         inorder_iterator(const inorder_iterator &other) : node(other.node) {}
         ~inorder_iterator() {}

         inorder_iterator& operator=(const inorder_iterator& other) { this->node = other.node; }

         reference operator*() { return this->node->label; }
         pointer operator->() { return &this->node->label; }

         inorder_iterator& operator++() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            if (this->node->right != nullptr)
            {
               this->node = this->node->right;

               while (this->node->left != nullptr)
                  this->node = this->node->left;
            }
            else
            {
               auto parent = this->node->parent;

               while (parent != nullptr && this->node == parent->right)
               {
                  this->node = parent;
                  parent = parent->parent;
               }

               this->node = parent;
            }
            
            return *this;
         }
         inorder_iterator& operator++(int) { auto tmp = *this; ++(*this); return tmp; }

         friend bool operator== (const inorder_iterator &a, const inorder_iterator &b) { return a.node == b.node; }
         friend bool operator!= (const inorder_iterator &a, const inorder_iterator &b) { return a.node != b.node; }

      private:
         typename Node::Shared node;
      };

      class preorder_iterator
      {
         friend AVLTree;
         
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = const Label;
         using pointer = const Label *;
         using reference = const Label &;

         preorder_iterator(typename Node::Shared node) : node(node) {}
         reference operator*() { return this->node->label; }
         pointer operator->() { return &this->node->label; }

         preorder_iterator& operator++() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            if (this->node->left != nullptr)
               this->node = this->node->left;
            else if (this->node->right != nullptr)
               this->node = this->node->right;
            else {
               auto parent = this->node->parent;

               while (parent != nullptr && this->node == parent->right)
               {
                  this->node = parent;
                  parent = parent->parent;
               }

               this->node = parent;
            }
                   
            return *this;
         }
         preorder_iterator& operator++(int) { auto tmp = *this; ++(*this); return tmp; }

         friend bool operator== (const preorder_iterator &a, const preorder_iterator &b) { return a.node == b.node; }
         friend bool operator!= (const preorder_iterator &a, const preorder_iterator &b) { return a.node != b.node; }

      private:
         typename Node::Shared node;
      };

      class postorder_iterator
      {
         friend AVLTree;
         
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = const Label;
         using pointer = const Label *;
         using reference = const Label &;

         postorder_iterator(typename Node::Shared node) : node(node) {
            if (node == nullptr) return;
            
            while (this->node->left != nullptr)
               this->node = this->node->left;

            while (this->node->right != nullptr)
               this->node = this->node->right;
         }

         reference operator*() { return this->node->label; }
         pointer operator->() { return &this->node->label; }

         postorder_iterator& operator++() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            auto parent = this->node->parent;

            if (parent != nullptr && this->node == parent->left)
            {
               if (parent->right == nullptr)
                  this->node = parent;
               else {
                  this->node = parent->right;

                  while (this->node->left != nullptr)
                     this->node = this->node->left;
                  
                  while (this->node->right != nullptr)
                     this->node = this->node->right;
               }
            }
            else
               this->node = parent;
                   
            return *this;
         }
         postorder_iterator& operator++(int) { auto tmp = *this; ++(*this); return tmp; }

         friend bool operator== (const postorder_iterator &a, const postorder_iterator &b) { return a.node == b.node; }
         friend bool operator!= (const postorder_iterator &a, const postorder_iterator &b) { return a.node != b.node; }

      private:
         typename Node::Shared node;
      };

      AVLTree() {}
      AVLTree(std::vector<Label> &nodes) {
         for (auto node : nodes)
            this->add_node(node);
      }
      ~AVLTree() {
         auto nodes = std::vector<Label>(this->begin_postorder(), this->end_postorder());

         for (auto node : nodes)
            this->remove_node(node);
      }

      bool is_empty() const { return this->root == nullptr; }
      bool contains(const Label &label) {
         auto traversal = this->search(label);
         auto last_node = *traversal.rbegin();

         return last_node.second == 0;
      }
      void insert(const Label &label) {
         this->add_node(label);
      }
      void remove(const Label &label) {
         this->remove_node(label);
      }

      inorder_iterator begin_inorder() const { return inorder_iterator(this->root); }
      inorder_iterator end_inorder() const { return inorder_iterator(nullptr); }
      preorder_iterator begin_preorder() const { return preorder_iterator(this->root); }
      preorder_iterator end_preorder() const { return preorder_iterator(nullptr); }
      postorder_iterator begin_postorder() const { return postorder_iterator(this->root); }
      postorder_iterator end_postorder() const { return postorder_iterator(nullptr); }
   };
}

#endif
