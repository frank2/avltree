#ifndef __AVLTREE_HPP
#define __AVLTREE_HPP

#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
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

      class KeyNotFound : public Exception {
      public:
         KeyNotFound() : Exception("The key was not found in the mapping.") {}
      };
   }
   
   template <typename Label, typename Compare = std::less<Label>>
   class AVLTree
   {
   public:
      using LabelType = typename Label;

   protected:
      struct Node;
      
      using SharedNode = std::shared_ptr<Node>;
      using ConstSharedNode = std::shared_ptr<const Node>;
      
      struct Node
      {
         Label label;
         int height;
         SharedNode parent, left, right;

         Node(Label label, SharedNode parent=nullptr, SharedNode left=nullptr, SharedNode right=nullptr)
            : label(label), parent(parent), left(left), right(right), height(0) {}

         bool is_leaf() const { return this->left == nullptr && this->right == nullptr; }

         int compare(const Label &label) const {
            auto ab = Compare()(label, this->label);
            auto ba = Compare()(this->label, label);

            if (!ab && !ba) { return 0; }
            if (ab && !ba) { return -1; }
            else { return 1; }
         }

         int compare(const SharedNode &node) const {
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

      template <typename NodeType>
      class inorder_iterator_base
      {
         static_assert(std::is_same<NodeType, SharedNode>::value || std::is_same<NodeType, ConstSharedNode>::value,
                       "Iterator template type must be a SharedNode or a ConstSharedNode.");
         
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = Label;
         using pointer = value_type *;
         using reference = value_type &;

         inorder_iterator_base(NodeType node) : node(node) {
            if (this->node == nullptr) return;
            
            while (this->node->left != nullptr)
            {
               this->node = this->node->left;
            }
         }
         inorder_iterator_base(const inorder_iterator_base &other) : node(other.node) {}
         ~inorder_iterator_base() {}

         inorder_iterator_base& operator=(const inorder_iterator_base& other) { this->node = other.node; }

         inorder_iterator_base& operator++() {
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
         inorder_iterator_base& operator++(int) { auto tmp = *this; ++(*this); return tmp; }

         friend bool operator== (const inorder_iterator_base &a, const inorder_iterator_base &b) { return a.node == b.node; }
         friend bool operator!= (const inorder_iterator_base &a, const inorder_iterator_base &b) { return a.node != b.node; }

      protected:
         NodeType node;
      };

      template <typename NodeType>
      class preorder_iterator_base
      {
         static_assert(std::is_same<NodeType, SharedNode>::value || std::is_same<NodeType, ConstSharedNode>::value,
                       "Iterator template type must be a SharedNode or a ConstSharedNode.");
         
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = Label;
         using pointer = value_type *;
         using reference = value_type &;

         preorder_iterator_base(NodeType node) : node(node) {}

         preorder_iterator_base& operator++() {
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
         preorder_iterator_base& operator++(int) { auto tmp = *this; ++(*this); return tmp; }

         friend bool operator== (const preorder_iterator_base &a, const preorder_iterator_base &b) { return a.node == b.node; }
         friend bool operator!= (const preorder_iterator_base &a, const preorder_iterator_base &b) { return a.node != b.node; }

      protected:
         NodeType node;
      };

      template <typename NodeType>
      class postorder_iterator_base
      {
         static_assert(std::is_same<NodeType, SharedNode>::value || std::is_same<NodeType, ConstSharedNode>::value,
                       "Iterator template type must be a SharedNode or a ConstSharedNode.");
         
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = Label;
         using pointer = value_type *;
         using reference = value_type &;

         postorder_iterator_base(NodeType node) : node(node) {
            if (node == nullptr) return;
            
            while (this->node->left != nullptr)
               this->node = this->node->left;

            while (this->node->right != nullptr)
               this->node = this->node->right;
         }

         postorder_iterator_base& operator++() {
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
         postorder_iterator_base& operator++(int) { auto tmp = *this; ++(*this); return tmp; }

         friend bool operator== (const postorder_iterator_base &a, const postorder_iterator_base &b) { return a.node == b.node; }
         friend bool operator!= (const postorder_iterator_base &a, const postorder_iterator_base &b) { return a.node != b.node; }

      protected:
         NodeType node;
      };

      SharedNode root;
      std::size_t _size;

      void set_right_child(SharedNode &target, SharedNode &child) {
         if (target == nullptr) { throw exception::NullPointer(); }
         
         target->right = child;

         if (child != nullptr)
            child->parent = target;
      }

      void set_left_child(SharedNode &target, SharedNode &child) {
         if (target == nullptr) { throw exception::NullPointer(); }

         target->left = child;

         if (child != nullptr)
            child->parent = target;
      }

      void set_parent(SharedNode &target, SharedNode &parent) {
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

      void rotate_left(SharedNode &rotation_root) {
         if (rotation_root == nullptr) { throw exception::NullPointer(); }
         
         auto pivot_root = rotation_root->right;
         auto left_child = pivot_root->left;
         auto rotation_parent = rotation_root->parent;

         if (rotation_parent == nullptr)
            this->root = pivot_root;
         else if (rotation_parent->left == rotation_root)
            rotation_parent->left = pivot_root;
         else if (rotation_parent->right == rotation_root)
            rotation_parent->right = pivot_root;

         pivot_root->parent = rotation_parent;

         if (left_child != nullptr)
            left_child->parent = rotation_root;

         rotation_root->right = left_child;
         pivot_root->left = rotation_root;
         rotation_root->parent = pivot_root;

         rotation_root->height = rotation_root->new_height();
         pivot_root->height = pivot_root->new_height();
      }

      void rotate_right(SharedNode &rotation_root) {
         if (rotation_root == nullptr) { throw exception::NullPointer(); }
         
         auto pivot_root = rotation_root->left;
         auto right_child = pivot_root->right;
         auto rotation_parent = rotation_root->parent;

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

      void rebalance_node(SharedNode &node) {
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

      void update_height(SharedNode &node) {
         if (node == nullptr) { throw exception::NullPointer(); }
         
         SharedNode update = node;

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

      std::vector<std::pair<ConstSharedNode, int>> search(const Label &label) const {
         auto result = std::vector<std::pair<ConstSharedNode, int>>();
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

      std::vector<std::pair<SharedNode, int>> search(const Label &label) {
         auto result = std::vector<std::pair<SharedNode, int>>();
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

      SharedNode add_node(Label &label) {
         if (this->root == nullptr)
         {
            this->root = std::make_shared<Node>(label);
            ++this->_size;
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

         ++this->_size;

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
            SharedNode replacement_node;

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

         --this->_size;
      }

   public:
      class inorder_iterator : public inorder_iterator_base<SharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = Label;
         using pointer = value_type *;
         using reference = value_type &;

         inorder_iterator(SharedNode node) : inorder_iterator_base(node) {}
         
         reference operator*() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node->label;
         }
         const reference operator*() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node->label;
         }
         pointer operator->() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node->label;
         }
         const pointer operator->() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node->label;
         }
      };

      class const_inorder_iterator : public inorder_iterator_base<ConstSharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = const Label;
         using pointer = value_type *;
         using reference = value_type &;

         const_inorder_iterator(ConstSharedNode node) : inorder_iterator_base(node) {}

         reference operator*() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node->label;
         }
         pointer operator->() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node->label;
         }
      };

      class preorder_iterator : public preorder_iterator_base<SharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = Label;
         using pointer = value_type *;
         using reference = value_type &;

         preorder_iterator(SharedNode node) : preorder_iterator_base(node) {}
         
         reference operator*() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node->label;
         }
         const reference operator*() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node->label;
         }
         pointer operator->() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node->label;
         }
         const pointer operator->() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node->label;
         }
      };

      class const_preorder_iterator : public preorder_iterator_base<ConstSharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = const Label;
         using pointer = value_type *;
         using reference = value_type &;

         const_preorder_iterator(ConstSharedNode node) : preorder_iterator_base(node) {}

         reference operator*() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node->label;
         }
         pointer operator->() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node->label;
         }
      };

      class postorder_iterator : public postorder_iterator_base<SharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = Label;
         using pointer = value_type *;
         using reference = value_type &;

         postorder_iterator(SharedNode node) : postorder_iterator_base(node) {}
         
         reference operator*() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node->label;
         }
         const reference operator*() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node->label;
         }
         pointer operator->() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node->label;
         }
         const pointer operator->() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node->label;
         }
      };

      class const_postorder_iterator : public postorder_iterator_base<ConstSharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = const Label;
         using pointer = value_type *;
         using reference = value_type &;

         const_postorder_iterator(ConstSharedNode node) : postorder_iterator_base(node) {}

         reference operator*() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node->label;
         }
         pointer operator->() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node->label;
         }
      };

      AVLTree() : _size(0) {}
      AVLTree(std::vector<Label> &nodes) : _size(0) {
         for (auto node : nodes)
            this->add_node(node);
      }
      AVLTree(const AVLTree &other) : _size(0) {
         if (this->root != nullptr)
            this->destroy();

         for (auto iter=other.begin_postorder(); iter!=other.end_postorder(); ++iter)
            this->add_node(*iter);
      }
      ~AVLTree() {
         this->destroy();
      }

      const_inorder_iterator begin_inorder() const { return const_inorder_iterator(this->root); }
      const_inorder_iterator end_inorder() const { return const_inorder_iterator(nullptr); }
      const_preorder_iterator begin_preorder() const { return const_preorder_iterator(this->root); }
      const_preorder_iterator end_preorder() const { return const_preorder_iterator(nullptr); }
      const_postorder_iterator begin_postorder() const { return const_postorder_iterator(this->root); }
      const_postorder_iterator end_postorder() const { return const_postorder_iterator(nullptr); }

      bool is_empty() const { return this->root == nullptr; }
      bool contains(const Label &label) const {
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
      std::vector<Label> to_vec() const {
         return std::vector<Label>(this->begin_postorder(), this->end_postorder());
      }
      std::size_t size() const {
         return this->_size;
      }
      void destroy() {
         auto nodes = this->to_vec();

         for (auto node : nodes)
            this->remove_node(node);
      }
   };

   template<typename Key, typename Value, typename Compare=std::less<Key>>
   struct AVLMapKeyCompare
   {
      bool operator()(const std::pair<const Key, Value> &left, const std::pair<const Key, Value> &right) {
         return Compare()(left.first, right.first);
      }
   };

   template <typename Key, typename Value, typename Compare=std::less<Key>>
   class AVLMap : public AVLTree<std::pair<const Key, Value>, AVLMapKeyCompare<Key, Value, Compare>>
   {
   public:
      AVLMap() : AVLTree() {}
      AVLMap(std::vector<typename AVLTree::LabelType> &nodes) : AVLTree(nodes) {}
      AVLMap(const AVLMap &other) : AVLTree(other) {}

      Value &operator[](const Key &key) {
         try {
            return this->get(key);
         }
         catch (exception::KeyNotFound &) {
            auto node = this->add_node(std::pair<const Key, Value>(key, Value()));
            return this->get(key);
         }
      }
      const Value &operator[](const Key &key) const { return this->get(key); }

      typename AVLTree::inorder_iterator begin_inorder() { return AVLTree::inorder_iterator(this->root); }
      typename AVLTree::inorder_iterator end_inorder() { return AVLTree::inorder_iterator(nullptr); }
      typename AVLTree::const_inorder_iterator cbegin_inorder() const { return AVLTree::const_inorder_iterator(this->root); }
      typename AVLTree::const_inorder_iterator cend_inorder() const { return AVLTree::const_inorder_iterator(nullptr); }

      typename AVLTree::preorder_iterator begin_preorder() { return AVLTree::preorder_iterator(this->root); }
      typename AVLTree::preorder_iterator end_preorder() { return AVLTree::preorder_iterator(nullptr); }
      typename AVLTree::const_preorder_iterator cbegin_preorder() const { return AVLTree::const_preorder_iterator(this->root); }
      typename AVLTree::const_preorder_iterator cend_preorder() const { return AVLTree::const_preorder_iterator(nullptr); }
      
      typename AVLTree::postorder_iterator begin_postorder() { return AVLTree::postorder_iterator(this->root); }
      typename AVLTree::postorder_iterator end_postorder() { return AVLTree::postorder_iterator(nullptr); }
      typename AVLTree::const_postorder_iterator cbegin_postorder() const { return AVLTree::const_postorder_iterator(this->root); }
      typename AVLTree::const_postorder_iterator cend_postorder() const { return AVLTree::const_postorder_iterator(nullptr); }

      typename AVLTree::postorder_iterator begin() { return this->begin_postorder(); }
      typename AVLTree::postorder_iterator end() { return this->end_postorder(); }
      typename AVLTree::const_postorder_iterator cbegin() const { return this->cbegin_postorder(); }
      typename AVLTree::const_postorder_iterator cend() const { return this->cend_postorder(); }

      std::optional<typename AVLTree::LabelType *> find(const Key &key)
      {
         if (this->root == nullptr) { return std::nullopt; }
         
         auto traversal = this->search(std::make_pair(key, Value()));
         auto last_node = *traversal.rbegin();

         if (last_node.second == 0) { return &last_node.first->label; }
         else { return std::nullopt; }
      }

      std::optional<const typename AVLTree::LabelType *> find(const Key &key) const
      {
         if (this->root == nullptr) { return std::nullopt; }
         
         auto traversal = this->search(std::make_pair(key, Value()));
         auto last_node = *traversal.rbegin();

         if (last_node.second == 0) { return &last_node.first->label; }
         else { return std::nullopt; }
      }

      bool has_key(const Key &key) const {
         return this->find(key).has_value();
      }

      void remove(const Key &key) {
         auto search = this->find(key);
         if (!search.has_value()) { throw exception::KeyNotFound(); }

         AVLTree::remove(**search);
      }

      Value &get(const Key &key) {
         auto result = this->find(key);
         if (result == std::nullopt) { throw exception::KeyNotFound(); }

         return (*result)->second;
      }

      const Value &get(const Key &key) const {
         auto result = this->find(key);
         if (result == std::nullopt) { throw exception::KeyNotFound(); }

         return (*result)->second;
      }
   };
}

#endif

