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

   class NodeKeysMatch : public Exception {
   public:
      NodeKeysMatch() : Exception("Node keys unexpectedly matched.") {}
   };

   class KeyExists : public Exception {
   public:
      KeyExists() : Exception("The key already exists in the tree.") {}
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
      KeyNotFound() : Exception("The key was not found in the tree.") {}
   };
}
   
   template <typename Key, typename Value, typename KeyOfValue, typename KeyCompare>
   class AVLTreeBase
   {
   public:
      class Node;
      
      using KeyType = typename Key;
      using ValueType = typename Value;
      using SharedNode = std::shared_ptr<Node>;
      using ConstSharedNode = std::shared_ptr<const Node>;

      class Node
      {
      protected:
         Value _value;
         int _height;
         SharedNode _parent, _left, _right;
         
      public:
         friend class AVLTreeBase;

         Node () : _value(Value()), _parent(nullptr), _left(nullptr), _right(nullptr), _height(0) {}
         Node(const Value &value) : _value(value), _parent(nullptr), _left(nullptr), _right(nullptr), _height(0) {}
         Node(const Node &other) : _value(other._value), _parent(other._parent), _left(other._left), _right(other._right), _height(other._height) {}

         virtual void copy_node_data(const Node &other) {
            this->_height = other._height;
            this->_parent = other._parent;
            this->_left = other._left;
            this->_right = other._right;
         }
                  
         inline const Key &key() const { return KeyOfValue()(this->value()); }
         inline Value &value() { return this->_value; }
         inline const Value &value() const { return this->_value; }
         inline int height() const { return this->_height; }
         inline SharedNode parent() { return this->_parent; }
         inline ConstSharedNode parent() const { return this->_parent; }
         inline SharedNode left() { return this->_left; }
         inline ConstSharedNode left() const { return this->_left; }
         inline SharedNode right() { return this->_right; }
         inline ConstSharedNode right() const { return this->_right; }

         inline bool is_leaf() const { return this->_left == nullptr && this->_right == nullptr; }

         int compare(const Key &key) const {
            auto ab = KeyCompare()(key, this->key());
            auto ba = KeyCompare()(this->key(), key);

            if (!ab && !ba) { return 0; }
            if (ab && !ba) { return -1; }
            else { return 1; }
         }

         int compare(ConstSharedNode &node) const {
            if (node == nullptr) { throw exception::NullPointer(); }

            return this->compare(node->key());
         }

         int balance() const {
            auto left_height = (this->_left != nullptr) ? this->_left->height() : 0;
            auto right_height = (this->_right != nullptr) ? this->_right->height() : 0;
            
            return right_height - left_height;
         }
                        
         int new_height() const {
            auto left_height = (this->_left != nullptr) ? this->_left->height() : 0;
            auto right_height = (this->_right != nullptr) ? this->_right->height() : 0;

            return std::max(left_height,right_height)+1;
         }
      };

   protected:
      template <typename NodeType>
      class inorder_iterator_base
      {
         static_assert(std::is_same<NodeType, SharedNode>::value || std::is_same<NodeType, ConstSharedNode>::value,
                       "Iterator template type must be a SharedNode or a ConstSharedNode.");
         
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = SharedNode;
         using pointer = value_type *;
         using reference = value_type &;

         inorder_iterator_base(NodeType node) : node(node) {
            if (this->node == nullptr) return;
            
            while (this->node->_left != nullptr)
            {
               this->node = std::dynamic_pointer_cast<Node>(this->node->_left);
            }
         }
         inorder_iterator_base(const inorder_iterator_base &other) : node(other.node) {}
         ~inorder_iterator_base() {}

         inorder_iterator_base& operator=(const inorder_iterator_base& other) { this->node = other.node; }

         inorder_iterator_base& operator++() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            if (this->node->_right != nullptr)
            {
               this->node = std::dynamic_pointer_cast<Node>(this->node->_right);

               while (this->node->_left != nullptr)
                  this->node = std::dynamic_pointer_cast<Node>(this->node->_left);
            }
            else
            {
               auto parent = std::dynamic_pointer_cast<Node>(this->node->_parent);

               while (parent != nullptr && this->node == std::dynamic_pointer_cast<Node>(parent->_right))
               {
                  this->node = parent;
                  parent = std::dynamic_pointer_cast<Node>(parent->_parent);
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
         using value_type = SharedNode;
         using pointer = value_type *;
         using reference = value_type &;

         preorder_iterator_base(NodeType node) : node(node) {}

         preorder_iterator_base& operator++() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            if (this->node->_left != nullptr)
               this->node = std::dynamic_pointer_cast<Node>(this->node->_left);
            else if (this->node->_right != nullptr)
               this->node = std::dynamic_pointer_cast<Node>(this->node->_right);
            else {
               auto parent = std::dynamic_pointer_cast<Node>(this->node->_parent);

               while (parent != nullptr && this->node == std::dynamic_pointer_cast<Node>(parent->_right))
               {
                  this->node = parent;
                  parent = std::dynamic_pointer_cast<Node>(parent->_parent);
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
         using value_type = SharedNode;
         using pointer = value_type *;
         using reference = value_type &;

         postorder_iterator_base(NodeType node) : node(node) {
            if (node == nullptr) return;
            
            while (this->node->_left != nullptr)
               this->node = std::static_pointer_cast<Node>(this->node->_left);

            while (this->node->_right != nullptr)
               this->node = std::static_pointer_cast<Node>(this->node->_right);
         }

         postorder_iterator_base& operator++() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            auto parent = std::static_pointer_cast<Node>(this->node->_parent);

            if (parent != nullptr && this->node == std::static_pointer_cast<Node>(parent->_left))
            {
               if (parent->_right == nullptr)
                  this->node = parent;
               else {
                  this->node = std::static_pointer_cast<Node>(parent->_right);

                  while (this->node->_left != nullptr)
                     this->node = std::static_pointer_cast<Node>(this->node->_left);
                  
                  while (this->node->_right != nullptr)
                     this->node = std::static_pointer_cast<Node>(this->node->_right);
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

      SharedNode _root;
      std::size_t _size;

      void set_right_child(SharedNode &target, SharedNode &child) {
         if (target == nullptr) { throw exception::NullPointer(); }
         
         target->_right = child;

         if (child != nullptr)
            child->_parent = target;
      }

      void set_left_child(SharedNode &target, SharedNode &child) {
         if (target == nullptr) { throw exception::NullPointer(); }

         target->_left = child;

         if (child != nullptr)
            child->_parent = target;
      }

      void set_parent(SharedNode &target, SharedNode &parent) {
         if (target == nullptr) { throw exception::NullPointer(); }

         if (parent == nullptr) { 
            target->_parent = parent;
            return;
         }

         auto branch = parent->compare(target);

         if branch == 0 { throw exception::NodeKeysMatch(); }
         else if branch < 0 { parent->_left = target; }
         else { parent->_right = target; }
      }

      virtual void rotate_left(SharedNode &rotation_root) {
         if (rotation_root == nullptr) { throw exception::NullPointer(); }
         
         auto pivot_root = rotation_root->_right;
         auto left_child = pivot_root->_left;
         auto rotation_parent = rotation_root->_parent;

         if (rotation_parent == nullptr)
            this->_root = pivot_root;
         else if (rotation_parent->_left == rotation_root)
            rotation_parent->_left = pivot_root;
         else if (rotation_parent->_right == rotation_root)
            rotation_parent->_right = pivot_root;

         pivot_root->_parent = rotation_parent;

         if (left_child != nullptr)
            left_child->_parent = rotation_root;

         rotation_root->_right = left_child;
         pivot_root->_left = rotation_root;
         rotation_root->_parent = pivot_root;

         rotation_root->_height = rotation_root->new_height();
         pivot_root->_height = pivot_root->new_height();
      }

      virtual void rotate_right(SharedNode &rotation_root) {
         if (rotation_root == nullptr) { throw exception::NullPointer(); }
         
         auto pivot_root = rotation_root->_left;
         auto right_child = pivot_root->_right;
         auto rotation_parent = rotation_root->_parent;

         if (rotation_parent == nullptr)
            this->_root = pivot_root;
         else if (rotation_parent->_left == rotation_root)
            rotation_parent->_left = pivot_root;
         else if (rotation_parent->_right == rotation_root)
            rotation_parent->_right = pivot_root;

         pivot_root->_parent = rotation_parent;

         if (right_child != nullptr)
            right_child->_parent = rotation_root;

         rotation_root->_left = right_child;
         pivot_root->_right = rotation_root;
         rotation_root->_parent = pivot_root;

         rotation_root->_height = rotation_root->new_height();
         pivot_root->_height = pivot_root->new_height();
      }

      virtual void rebalance_node(SharedNode &node) {
         if (node == nullptr) { throw exception::NullPointer(); }
         
         auto balance = node->balance();
         if (balance == 0) { return; }

         if (balance < 0)
         {
            auto child = node->_left;
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
            auto child = node->_right;
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

      virtual void update_node(SharedNode &node) {
         if (node == nullptr) { throw exception::NullPointer(); }
         
         SharedNode update = node;

         while (update != nullptr)
         {
            auto old_height = update->_height;
            auto new_height = update->new_height();
            auto balance = update->balance();
            
            update->_height = new_height;

            if (balance > 1 || balance < -1)
            {
               this->rebalance_node(update);
               return;
            }

            if (old_height == new_height) { return; }

            if (update->_parent != nullptr)
               update = update->_parent;
         }
      }

      virtual SharedNode allocate_node(const Value &value) {
         auto node = std::make_shared<Node>(value);

         return node;
      }

      virtual SharedNode copy_node(ConstSharedNode node) {
         auto new_node = std::make_shared<Node>(*node);

         return new_node;
      }
         
      virtual SharedNode add_node(const Value &value) {
         if (this->_root == nullptr)
         {
            this->_root = this->allocate_node(value);
            ++this->_size;
            return this->_root;
         }

         auto key = KeyOfValue()(value);
         auto traversal = this->search(key);
         auto last_result = *traversal.rbegin();
         auto parent = last_result.first;
         auto branch = last_result.second;
         
         if (branch == 0) { throw exception::KeyExists(); }

         auto new_node = this->allocate_node(value);

         if (branch < 0) { this->set_left_child(parent, new_node); }
         else { this->set_right_child(parent, new_node); }

         this->update_node(new_node);

         ++this->_size;

         return new_node;
      }

      virtual SharedNode remove_node(const Value &value) {
         if (this->is_empty()) { throw exception::EmptyTree(); }

         auto key = KeyOfValue()(value);
         auto traversal = this->search(key);
         auto last_node = *traversal.rbegin();
         SharedNode update_node = nullptr;

         if (last_node.second != 0) { throw exception::NodeNotFound(); }

         auto node = last_node.first;

         if (node->is_leaf())
         {
            if (node->_parent != nullptr)
            {
               if (node->_parent->_left == node)
                  node->_parent->_left.reset();
               else if (node->_parent->_right == node)
                  node->_parent->_right.reset();
            }

            if (node == this->_root)
            {
               this->_root.reset();
            }
            else
            {
               auto parent = node->_parent;
               node->_parent.reset();

               update_node = parent;
            }
         }
         else if (node->_left == nullptr || node->_right == nullptr)
         {
            SharedNode replacement_node;

            if (node->_left == nullptr)
               replacement_node = node->_right;
            else
               replacement_node = node->_left;

            replacement_node->_parent = node->_parent;

            if (node == this->_root)
            {
               this->_root = replacement_node;
               update_node = this->_root;
            }
            else if (node->_parent->_left == node)
            {
               node->_parent->_left = replacement_node;
               update_node = node->_parent;
            }
            else if (node->_parent->_right == node)
            {
               node->_parent->_right = replacement_node;
               update_node = node->_parent;
            }
         }
         else if (node->_left != nullptr && node->_right != nullptr)
         {
            auto leftmost = node->_right;

            while (leftmost->_left != nullptr)
               leftmost = leftmost->_left;

            auto replaced_node = leftmost->_right;
            auto leftmost_parent = leftmost->_parent;

            if (leftmost_parent == node) {
               node->_right = replaced_node;
            }
            else {
               leftmost->_parent->_left = replaced_node;

               if (replaced_node != nullptr)
                  replaced_node->_parent = leftmost->_parent;
            }

            leftmost->copy_node_data(*node);

            if (leftmost->_left != nullptr)
               leftmost->_left->_parent = leftmost;

            if (leftmost->_right != nullptr)
               leftmost->_right->_parent = leftmost;

            if (leftmost->_parent != nullptr)
            {
               if (node == leftmost->_parent->_left)
                  leftmost->_parent->_left = leftmost;
               else if (node == leftmost->_parent->_right)
                  leftmost->_parent->_right = leftmost;
            }

            if (node == this->_root)
               this->_root = leftmost;

            if (replaced_node != nullptr)
               update_node = replaced_node;
            else if (leftmost_parent != node)
               update_node = leftmost_parent;
            else
               update_node = leftmost;
         }

         node->_left.reset();
         node->_right.reset();
         node->_parent.reset();

         --this->_size;

         if (update_node != nullptr) { this->update_node(update_node); }

         return update_node;
      }

   public:
      class inorder_iterator : public inorder_iterator_base<SharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = SharedNode;
         using pointer = value_type *;
         using reference = value_type &;

         inorder_iterator(SharedNode node) : inorder_iterator_base(node) {}
         
         reference operator*() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node;
         }
         const reference operator*() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node;
         }
         pointer operator->() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node;
         }
         const pointer operator->() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node;
         }
      };

      class const_inorder_iterator : public inorder_iterator_base<ConstSharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = ConstSharedNode;
         using pointer = value_type *;
         using reference = value_type &;

         const_inorder_iterator(ConstSharedNode node) : inorder_iterator_base(node) {}

         reference operator*() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node;
         }
         pointer operator->() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node;
         }
      };

      class preorder_iterator : public preorder_iterator_base<SharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = SharedNode;
         using pointer = value_type *;
         using reference = value_type &;

         preorder_iterator(SharedNode node) : preorder_iterator_base(node) {}
         
         reference operator*() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node;
         }
         const reference operator*() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node;
         }
         pointer operator->() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node;
         }
         const pointer operator->() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node;
         }
      };

      class const_preorder_iterator : public preorder_iterator_base<ConstSharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = ConstSharedNode;
         using pointer = value_type *;
         using reference = value_type &;

         const_preorder_iterator(ConstSharedNode node) : preorder_iterator_base(node) {}

         reference operator*() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node;
         }
         pointer operator->() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node;
         }
      };

      class postorder_iterator : public postorder_iterator_base<SharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = SharedNode;
         using pointer = value_type *;
         using reference = value_type &;

         postorder_iterator(SharedNode node) : postorder_iterator_base(node) {}
         
         reference operator*() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node;
         }
         const reference operator*() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node;
         }
         pointer operator->() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node;
         }
         const pointer operator->() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node;
         }
      };

      class const_postorder_iterator : public postorder_iterator_base<ConstSharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = ConstSharedNode;
         using pointer = value_type *;
         using reference = value_type &;

         const_postorder_iterator(ConstSharedNode node) : postorder_iterator_base(node) {}

         reference operator*() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node;
         }
         pointer operator->() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node;
         }
      };

      template <typename iterator_base>
      class value_iterator : public iterator_base
      {
         static_assert(std::is_same<iterator_base, inorder_iterator>::value ||
                       std::is_same<iterator_base, preorder_iterator>::value ||
                       std::is_same<iterator_base, postorder_iterator>::value,
                       "Value iterator base must be an inorder_iterator, a preorder_iterator or a postorder_iterator.");

      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = Value;
         using pointer = value_type *;
         using reference = value_type &;

         value_iterator(SharedNode node) : iterator_base(node) {}

         reference operator*() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node->value();
         }

         const reference operator*() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node->value();
         }

         pointer operator->() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node->value();
         }

         const pointer operator->() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node->value();
         }
      };
      
      template <typename iterator_base>
      class const_value_iterator : public iterator_base
      {
         static_assert(std::is_same<iterator_base, const_inorder_iterator>::value ||
                       std::is_same<iterator_base, const_preorder_iterator>::value ||
                       std::is_same<iterator_base, const_postorder_iterator>::value,
                       "Value iterator base must be a const_inorder_iterator, a const_preorder_iterator or a const_postorder_iterator.");

      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = const Value;
         using pointer = value_type *;
         using reference = value_type &;

         const_value_iterator(ConstSharedNode node) : iterator_base(node) {}

         reference operator*() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node->value();
         }

         pointer operator->() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node->value();
         }
      };

      using iterator = value_iterator<postorder_iterator>;
      using const_iterator = const_value_iterator<const_postorder_iterator>;

      AVLTreeBase() : _size(0) {}
      AVLTreeBase(std::vector<Value> &nodes) : _size(0) {
         for (auto node : nodes)
            this->add_node(node);
      }
      AVLTreeBase(const AVLTreeBase &other) {
         this->copy(other);
      }
      virtual ~AVLTreeBase() {
         this->destroy();
      }

      inorder_iterator begin_inorder() { return inorder_iterator(this->_root); }
      inorder_iterator end_inorder() { return inorder_iterator(nullptr); }
      const_inorder_iterator cbegin_inorder() const { return const_inorder_iterator(this->_root); }
      const_inorder_iterator cend_inorder() const { return const_inorder_iterator(nullptr); }

      preorder_iterator begin_preorder() { return preorder_iterator(this->_root); }
      preorder_iterator end_preorder() { return preorder_iterator(nullptr); }
      const_preorder_iterator cbegin_preorder() const { return const_preorder_iterator(this->_root); }
      const_preorder_iterator cend_preorder() const { return const_preorder_iterator(nullptr); }

      postorder_iterator begin_postorder() { return postorder_iterator(this->_root); }
      postorder_iterator end_postorder() { return postorder_iterator(nullptr); }
      const_postorder_iterator cbegin_postorder() const { return const_postorder_iterator(this->_root); }
      const_postorder_iterator cend_postorder() const { return const_postorder_iterator(nullptr); }

      value_iterator<inorder_iterator> begin_values_inorder() { return value_iterator<inorder_iterator>(this->_root); }
      value_iterator<inorder_iterator> end_values_inorder() { return value_iterator<inorder_iterator>(nullptr); }
      const_value_iterator<const_inorder_iterator> cbegin_values_inorder() { return const_value_iterator<const_inorder_iterator>(this->_root); }
      const_value_iterator<const_inorder_iterator> cend_values_inorder() { return const_value_iterator<const_inorder_iterator>(nullptr); }

      value_iterator<preorder_iterator> begin_values_preorder() { return value_iterator<preorder_iterator>(this->_root); }
      value_iterator<preorder_iterator> end_values_preorder() { return value_iterator<preorder_iterator>(nullptr); }
      const_value_iterator<const_preorder_iterator> cbegin_values_preorder() { return const_value_iterator<const_preorder_iterator>(this->_root); }
      const_value_iterator<const_preorder_iterator> cend_values_preorder() { return const_value_iterator<const_preorder_iterator>(nullptr); }

      value_iterator<postorder_iterator> begin_values_postorder() { return value_iterator<postorder_iterator>(this->_root); }
      value_iterator<postorder_iterator> end_values_postorder() { return value_iterator<postorder_iterator>(nullptr); }
      const_value_iterator<const_postorder_iterator> cbegin_values_postorder() { return const_value_iterator<const_postorder_iterator>(this->_root); }
      const_value_iterator<const_postorder_iterator> cend_values_postorder() { return const_value_iterator<const_postorder_iterator>(nullptr); }

      iterator begin() { return iterator(this->_root); }
      iterator end() { return iterator(nullptr); }
      const_iterator cbegin() const { return const_iterator(this->_root); }
      const_iterator cend() const { return const_iterator(nullptr); }

      inline bool is_empty() const { return this->_root == nullptr; }
      inline SharedNode root() { return this->_root; }
      inline ConstSharedNode root() const { return this->_root; }
      bool contains(const Key &key) const {
         auto traversal = this->search(key);
         auto last_node = *traversal.rbegin();

         return last_node.second == 0;
      }
      std::vector<std::pair<ConstSharedNode, int>> search(const Key &key) const {
         auto result = std::vector<std::pair<ConstSharedNode, int>>();
         if (this->_root == nullptr) { return result; }
         
         ConstSharedNode node = this->_root;
         auto branch = node->compare(key);
         
         while (branch != 0 && node != nullptr)
         {
            result.push_back(std::make_pair(node, branch));

            if (branch < 0)
               node = node->left();
            else
               node = node->right();

            if (node != nullptr)
               branch = node->compare(key);
         }
            
         if (node != nullptr) { result.push_back(std::make_pair(node, branch)); }

         return result;
      }
      std::vector<std::pair<SharedNode, int>> search(const Key &key) {
         auto result = std::vector<std::pair<SharedNode, int>>();
         if (this->_root == nullptr) { return result; }
         
         auto node = this->_root;
         auto branch = node->compare(key);
         
         while (branch != 0 && node != nullptr)
         {
            result.push_back(std::make_pair(node, branch));

            if (branch < 0)
               node = node->_left;
            else
               node = node->_right;

            if (node != nullptr)
               branch = node->compare(key);
         }
            
         if (node != nullptr) { result.push_back(std::make_pair(node, branch)); }

         return result;
      }
      std::optional<SharedNode> find(const Key &key) {
         if (this->_root == nullptr) { return std::nullopt; }
         
         auto traversal = this->search(key);
         auto last_node = *traversal.rbegin();

         if (last_node.second == 0) { return last_node.first; }
         else { return std::nullopt; }
      }
      std::optional<ConstSharedNode> find(const Key &key) const {
         if (this->_root == nullptr) { return std::nullopt; }
         
         auto traversal = this->search(key);
         auto last_node = *traversal.rbegin();

         if (last_node.second == 0) { return last_node.first; }
         else { return std::nullopt; }
      }
      SharedNode get(const Key &key) {
         auto result = this->find(key);

         if (result == std::nullopt) { throw exception::KeyNotFound(); }
         return *result;
      }
      ConstSharedNode get(const Key &key) const {
         auto result = this->find(key);

         if (result == std::nullopt) { throw exception::KeyNotFound(); }
         return *result;
      }
      SharedNode insert(const Value &value) {
         return this->add_node(value);
      }
      void remove(const Key &key) {
         auto traversal = this->search(key);
         auto last_node = *traversal.rbegin();
         if (last_node.second != 0) { throw exception::KeyNotFound(); }
         
         this->remove_node(last_node.first->value());
      }
      std::vector<Value> to_vec() const {
         return std::vector<Value>(this->cbegin(), this->cend());
      }
      inline std::size_t size() const {
         return this->_size;
      }
      void destroy() {
         if (this->_root == nullptr) { return; }
         std::vector<SharedNode> visiting = { this->_root };

         while (visiting.size() > 0)
         {
            auto node = visiting.front();
            visiting.erase(visiting.begin());
            
            if (node->_left != nullptr) { visiting.push_back(node->_left); }
            if (node->_right != nullptr) { visiting.push_back(node->_right); }

            node->_parent.reset();
            node->_left.reset();
            node->_right.reset();
         }

         this->_root.reset();
      }
      void copy(const AVLTreeBase &other) {
         if (this->_root != nullptr)
            this->destroy();
         
         if (other._root == nullptr) { return; }

         std::vector<ConstSharedNode> visiting = { other.root };
         std::vector<SharedNode> added = { this->copy_node(other.root) }
         std::size_t parent_index = 0;

         this->_root = added.front();

         while (visiting.size() > 0)
         {
            auto node = visiting.front();
            visiting.erase(visiting.begin());
            
            if (node == nullptr) { continue; }

            auto new_node = added.front();
            added.erase(added.begin());

            if (node->is_leaf()) { continue; }

            if (node->_left != nullptr) {
               auto left = this->copy_node(node->_left);
               this->set_left_child(new_node, left);
               added.push_back(left);
               visiting.push_back(node->_left);
            }
            
            if (node->_right != nullptr) {
               auto right = this->copy_node(node->_right);
               this->set_right_child(new_node, right);
               added.push_back(right);
               visiting.push_back(node->_right);
            }
         }
      }
   };

   template <typename Value>
   struct KeyIsValue {
      const Value &operator() (const Value &value) const { return value; }
   };

   template <typename Key, typename KeyCompare=std::less<Key>>
   class AVLTree : public AVLTreeBase<Key, Key, KeyIsValue<Key>, KeyCompare>
   {
   public:
      using iterator = typename AVLTreeBase::const_iterator;

      AVLTree() : AVLTreeBase() {}
      AVLTree(std::vector<Key> &nodes) : AVLTreeBase(nodes) {}
      AVLTree(const AVLTree &other) : AVLTreeBase(other) {}

      iterator begin() const { return iterator(this->root()); }
      iterator end() const { return iterator(nullptr); }
      iterator cbegin() const { return this->begin(); }
      iterator cend() const { return this->end(); }
   };

   template<typename Key, typename Value>
   struct KeyOfPair {
      const Key &operator() (const std::pair<const Key, Value> &pair) const { return pair.first; }
   };

   template <typename Key, typename Value, typename KeyCompare=std::less<Key>>
   class AVLMap : public AVLTreeBase<Key, std::pair<const Key, Value>, KeyOfPair<Key, Value>, KeyCompare>
   {
   public:
      AVLMap() : AVLTreeBase() {}
      AVLMap(std::vector<typename AVLTreeBase::ValueType> &nodes) : AVLTreeBase(nodes) {}
      AVLMap(const AVLMap &other) : AVLTreeBase(other) {}

      Value &operator[](const Key &key) {
         try {
            return this->get(key);
         }
         catch (exception::KeyNotFound &) {
            auto node = this->add_node(std::make_pair(key, Value()));
            return node->value().second;
         }
      }
      const Value &operator[](const Key &key) const { return this->get(key); }
      
      bool has_key(const Key &key) const {
         return this->contains(key);
      }

      void insert(const Key &key, const Value &value) {
         AVLTreeBase::insert(std::make_pair(key, value));
      }

      Value &get(const Key &key) {
         return AVLTreeBase::get(key)->value().second;
      }

      const Value &get(const Key &key) const {
         return AVLTreeBase::get(key)->value().second;
      }
   };
}

#endif

