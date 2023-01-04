#ifndef __AVLTREE_HPP
#define __AVLTREE_HPP

//! @mainpage AVL Tree
//!
//! An **[AVL tree](https://en.wikipedia.org/wiki/AVL_tree)** is a self-balancing binary tree structure.
//! It can be the basis of many types of data structures. For example, a
//! **[red-black tree](https://en.wikipedia.org/wiki/Red%E2%80%93black_tree)**-- which is a similar data
//! structure to an AVL tree-- is the basis for many common C++ data structures, such as mappings and sets.
//!
//! This library attempts to provide a basic, customizable AVL tree for the implementation of other data
//! structures which call for a self-balancing tree, such as an
//! [interval tree](https://en.wikipedia.org/wiki/Interval_tree).
//!

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
   /// @brief The base exception of exceptions thrown by this library.
   ///
   class Exception : public std::exception
   {
   public:
      /// @brief The error string returned by the exception.
      ///
      std::string error;

      Exception() : std::exception() {}
      Exception(std::string error) : error(error), std::exception() {}

      /// @brief Return a C-style string of this exception.
      ///
      /// This is mostly for reverse-compatibility with std::exception.
      ///
      const char *what() const noexcept {
         return this->error.c_str();
      }
   };

   /// @brief Exception thrown when encountering an unexpected null pointer.
   ///
   class NullPointer : public Exception {
   public:
      NullPointer() : Exception("Encountered an unexpected null pointer.") {}
   };

   /// @brief Exception thrown when the keys of two nodes match when they shouldn't.
   ///
   class NodeKeysMatch : public Exception {
   public:
      NodeKeysMatch() : Exception("Node keys unexpectedly matched.") {}
   };

   /// @brief Exception thrown when the key already exists within the tree.
   ///
   class KeyExists : public Exception {
   public:
      KeyExists() : Exception("The key already exists in the tree.") {}
   };

   /// @brief Exception thrown when the tree contains no nodes.
   ///
   class EmptyTree : public Exception {
   public:
      EmptyTree() : Exception("The tree is empty.") {}
   };

   /// @brief Exception thrown when the node being searched wasn't found.
   ///
   class NodeNotFound : public Exception {
   public:
      NodeNotFound() : Exception("The node was not found.") {}
   };

   /// @brief Exception thrown when the key is not found in the tree.
   ///
   class KeyNotFound : public Exception {
   public:
      KeyNotFound() : Exception("The key was not found in the tree.") {}
   };
}

   /// @brief The base implementation of an AVL tree.
   ///
   /// **NOTE**: For a basic AVL tree implementation, this interface is too complex. See the AVLTree class
   /// for a more basic interface to an AVL tree.
   ///
   /// This is the base AVL tree implementation. Its template interface is designed in this way to handle
   /// mapping objects, and takes inspiration from libstdc++'s red-black tree interface for its implementation
   /// of a map. For implementing some tree-based objects that have a map-like design, this is the interface
   /// to use. Otherwise, the simplified versions of this interface, separated into AVLTree and AVLMap, should
   /// suffice.
   ///
   /// @tparam Key The type class of the object that acts as the node's key, also known as its label. This value
   /// is the value responsible for indexing the nodes in the tree. This is expected to be a constant value-- undefined
   /// behavior is likely to happen if you modify a tree node's key.
   ///
   /// @tparam Value The value type class of the object, which is expected to host the key value in some way, whose
   /// extraction of said key is determined by the functor KeyOfValue. This type can be the same as the key type with
   /// use of the KeyIsValue functor.
   ///
   /// @tparam KeyOfValue The functor which extracts the node's Key type value from the node's Value object. See
   /// KeyIsValue and KeyOfPair for examples of how to use this value.
   ///
   /// @tparam KeyCompare The key comparison functor, usually std::less<Key>. This functor must conform to C++'s
   /// [Compare requirements](https://en.cppreference.com/w/cpp/named_req/Compare).
   ///
   template <typename Key, typename Value, typename KeyOfValue, typename KeyCompare>
   class AVLTreeBase
   {
   public:
      class Node;
      
      using KeyType = Key;
      using ValueType = Value;
      using SharedNode = std::shared_ptr<Node>;
      using ConstSharedNode = std::shared_ptr<const Node>;

      /// @brief A node object for an AVL tree.
      ///
      /// This object contains data about a given node's key object, value, tree height and node relationships.
      ///
      class Node
      {
      protected:
         /// @brief The value data of the node.
         ///
         /// This contains the node's key.
         ///
         Value _value;
         
         /// @brief The height of the node in the tree.
         ///
         int _height;

         /// @brief The parent node of this node.
         ///
         SharedNode _parent;

         /// @brief The left child of this node.
         ///
         SharedNode _left;

         /// @brief The right child of this node.
         ///
         SharedNode _right;
         
      public:
         friend class AVLTreeBase;

         Node () : _value(Value()), _parent(nullptr), _left(nullptr), _right(nullptr), _height(0) {}
         Node(const Value &value) : _value(value), _parent(nullptr), _left(nullptr), _right(nullptr), _height(0) {}
         Node(const Node &other) : _value(other._value), _parent(other._parent), _left(other._left), _right(other._right), _height(other._height) {}

         /// @brief Copy everything except the value from the given node.
         ///
         /// This is useful for certain cases when deleting nodes from the tree.
         ///
         virtual void copy_node_data(const Node &other) {
            this->_height = other._height;
            this->_parent = other._parent;
            this->_left = other._left;
            this->_right = other._right;
         }

         /// @brief Get the key value of this node.
         ///
         /// This calls the KeyOfValue functor against the value object within the node and returns it.
         ///
         inline const Key &key() const { return KeyOfValue()(this->value()); }
         /// @brief Get the value object of this node.
         ///
         inline Value &value() { return this->_value; }
         /// @brief Get the const value object of this node.
         ///
         inline const Value &value() const { return this->_value; }
         /// @brief Get the height of this node.
         ///
         inline int height() const { return this->_height; }
         /// @brief Get the parent node of this node.
         ///
         inline SharedNode parent() { return this->_parent; }
         /// @brief Get the const parent node of this node.
         ///
         inline ConstSharedNode parent() const { return this->_parent; }
         /// @brief Get the left child of this node.
         ///
         inline SharedNode left() { return this->_left; }
         /// @brief Get the const left child of this node.
         ///
         inline ConstSharedNode left() const { return this->_left; }
         /// @brief Get the right child of this node.
         ///
         inline SharedNode right() { return this->_right; }
         /// @brief Get the const right child of this node.
         ///
         inline ConstSharedNode right() const { return this->_right; }

         /// @brief Determine if this node is a leaf node.
         ///
         inline bool is_leaf() const { return this->_left == nullptr && this->_right == nullptr; }

         /// @brief Do a binary comparison of the given key against the node's key.
         ///
         /// @returns 0 if the key is equal to the key in the node, -1 if the key is less than the node's key,
         /// and 1 if the key is greater than the node's key.
         ///
         int compare(const Key &key) const {
            auto ab = KeyCompare()(key, this->key());
            auto ba = KeyCompare()(this->key(), key);

            if (!ab && !ba) { return 0; }
            if (ab && !ba) { return -1; }
            else { return 1; }
         }

         /// @brief Compare this node's key against the current node's key.
         ///
         /// See compare(const Key &key).
         ///
         int compare(ConstSharedNode &node) const {
            if (node == nullptr) { throw exception::NullPointer(); }

            return this->compare(node->key());
         }

         /// @brief Determine the balance of this given node.
         ///
         /// In other words, subtract the height of the right child against the height of the left child
         /// and return the value.
         ///
         int balance() const {
            auto left_height = (this->_left != nullptr) ? this->_left->height() : 0;
            auto right_height = (this->_right != nullptr) ? this->_right->height() : 0;
            
            return right_height - left_height;
         }

         /// @brief Determine the new height value of this node.
         ///
         int new_height() const {
            auto left_height = (this->_left != nullptr) ? this->_left->height() : 0;
            auto right_height = (this->_right != nullptr) ? this->_right->height() : 0;

            return std::max(left_height,right_height)+1;
         }
      };

   protected:
      /// @brief The base iterator for performing an in-order traversal.
      ///
      /// This class is the base class for iterating over the nodes in the tree in an
      /// [in-order traversal](https://en.wikipedia.org/wiki/Tree_traversal#In-order,_LNR).
      ///
      /// @tparam NodeType The node class for the base iterator. Can be either SharedNode or ConstSharedNode.
      ///
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
               this->node = this->node->_left;
            }
         }
         inorder_iterator_base(const inorder_iterator_base &other) : node(other.node) {}
         ~inorder_iterator_base() {}

         inorder_iterator_base& operator=(const inorder_iterator_base& other) { this->node = other.node; }

         inorder_iterator_base& operator++() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            if (this->node->_right != nullptr)
            {
               this->node = this->node->_right;

               while (this->node->_left != nullptr)
                  this->node = this->node->_left;
            }
            else
            {
               auto parent = this->node->_parent;

               while (parent != nullptr && this->node == parent->_right)
               {
                  this->node = parent;
                  parent = parent->_parent;
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

      /// @brief The base iterator for performing a pre-order traversal.
      ///
      /// This class is the base class for iterating over the nodes in the tree in a
      /// [pre-order traversal](https://en.wikipedia.org/wiki/Tree_traversal#Pre-order,_NLR).
      ///
      /// @tparam NodeType The node class for the base iterator. Can be either SharedNode or ConstSharedNode.
      ///
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
               this->node = this->node->_left;
            else if (this->node->_right != nullptr)
               this->node = this->node->_right;
            else {
               auto parent = this->node->_parent;

               while (parent != nullptr && (this->node == parent->_right || parent->_right == nullptr))
               {
                  this->node = parent;
                  parent = parent->_parent;
               }

               if (parent != nullptr)
                  this->node = parent->_right;
               else
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

      /// @brief The base iterator for performing a post-order traversal.
      ///
      /// This class is the base class for iterating over the nodes in the tree in a
      /// [post-order traversal](https://en.wikipedia.org/wiki/Tree_traversal#Post-order,_LRN).
      ///
      /// @tparam NodeType The node class for the base iterator. Can be either SharedNode or ConstSharedNode.
      ///
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
               this->node = this->node->_left;

            while (this->node->_right != nullptr)
               this->node = this->node->_right;
         }

         postorder_iterator_base& operator++() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            auto parent = this->node->_parent;

            if (parent != nullptr && this->node == parent->_left)
            {
               if (parent->_right == nullptr)
                  this->node = parent;
               else {
                  this->node = parent->_right;

                  while (this->node->_left != nullptr)
                     this->node = this->node->_left;
                  
                  while (this->node->_right != nullptr)
                     this->node = this->node->_right;
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

      /// @brief The root of the tree.
      ///
      SharedNode _root;
      /// @brief The number of nodes in the tree.
      ///
      std::size_t _size;

      /// @brief Set the right child of the given node.
      ///
      /// This sets the *target* as the parent of *child* and the *child* as the right-child of *target*.
      ///
      /// @param target The node to set the right child of.
      /// @param child The child node to set in the target.
      ///
      /// @throws exception::NullPointer Thrown when the target argument is null.
      ///
      void set_right_child(SharedNode target, SharedNode child) {
         if (target == nullptr) { throw exception::NullPointer(); }
         
         target->_right = child;

         if (child != nullptr)
            child->_parent = target;
      }

      /// @brief Set the left child of the given node.
      ///
      /// This sets the *target* as the parent of *child* and the *child* as the left-child of *target*.
      ///
      /// @param target The node to set the left child of.
      /// @param child The child node to set in the target.
      ///
      /// @throws exception::NullPointer Thrown when the target argument is null.
      ///
      void set_left_child(SharedNode target, SharedNode child) {
         if (target == nullptr) { throw exception::NullPointer(); }

         target->_left = child;

         if (child != nullptr)
            child->_parent = target;
      }

      /// @brief Set the parent of the given node.
      ///
      /// This sets the *parent* of the *target* node and sets the *target* as the left or right child
      /// of the *parent*, depending on how it compares with the node.
      ///
      /// @param target The target node whose parent you wish to alter.
      /// @param parent The parent node to change to.
      ///
      /// @throws exception::NullPointer Thrown when the target argument is null.
      /// @throws exception::NodeKeysMatch Thrown when the target key is equal to the parent key.
      ///
      void set_parent(SharedNode target, SharedNode parent) {
         if (target == nullptr) { throw exception::NullPointer(); }

         if (parent == nullptr) { 
            target->_parent = parent;
            return;
         }

         auto branch = parent->compare(target);

         if (branch == 0) { throw exception::NodeKeysMatch(); }
         else if (branch < 0) { parent->_left = target; }
         else { parent->_right = target; }
      }

      /// @brief Do a left rotation on the given node.
      ///
      /// See [the rebalancing section](https://en.wikipedia.org/wiki/AVL_tree#Rebalancing) for an AVL tree.
      ///
      /// @param rotation_root The node to rotate.
      ///
      /// @throws exception::NullPointer Thrown when the rotation root is null.
      ///
      virtual void rotate_left(SharedNode rotation_root) {
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

      /// @brief Do a right rotation on the given node.
      ///
      /// See [the rebalancing section](https://en.wikipedia.org/wiki/AVL_tree#Rebalancing) for an AVL tree.
      ///
      /// @param rotation_root The node to rotate.
      ///
      /// @throws exception::NullPointer Thrown when the rotation root is null.
      ///
      virtual void rotate_right(SharedNode rotation_root) {
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

      /// @brief Rebalance the given node if its balance is greater than 1 or less than -1.
      ///
      /// When a node needs to be rebalanced, certain rotation operations need to happen.
      /// See [the rebalancing section](https://en.wikipedia.org/wiki/AVL_tree#Rebalancing) on AVL trees.
      ///
      /// @param node The node to rebalance.
      ///
      /// @throws exception::NullPointer Thrown when the node argument is null.
      ///
      virtual void rebalance_node(SharedNode node) {
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

      /// @brief Update the given node after an insertion or deletion operation.
      ///
      /// This function simply updates the height and verifies the tree is balanced, but is virtual
      /// for tree objects which need to update more information or perform other actions.
      ///
      /// @param node The node to update.
      ///
      /// @throws exception::NullPointer Thrown when the node argument is null.
      ///
      virtual void update_node(SharedNode node) {
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

      /// @brief Allocate a new node object with the given value.
      ///
      /// @param value The value the node should have.
      ///
      virtual SharedNode allocate_node(const Value &value) {
         auto node = std::make_shared<Node>(value);

         return node;
      }

      /// @brief Create a copy of the given node.
      ///
      /// @param The node to copy.
      ///
      virtual SharedNode copy_node(ConstSharedNode node) {
         auto new_node = std::make_shared<Node>(*node);

         return new_node;
      }

      /// @brief Add a new node to the tree.
      ///
      /// @param value The value the new node should have.
      ///
      /// @throws exception::KeyExists Thrown when the key of the given value already exists within the tree.
      ///
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

      /// @brief Remove a given node from the tree with the given value.
      ///
      /// Note that only the key of the value is used to verify the node to delete, not the whole value itself.
      ///
      /// @param value The value object to remove from the tree.
      ///
      /// @throw exception::EmptyTree Thrown when the tree is empty.
      /// @throw exception::NodeNotFound Thrown when the key of the value is not found within the tree.
      ///
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
      /// @brief An iterator that performs an in-order traversal on the tree.
      ///
      class inorder_iterator : public inorder_iterator_base<SharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = SharedNode;
         using pointer = value_type *;
         using reference = value_type &;

         inorder_iterator(SharedNode node) : inorder_iterator_base<SharedNode>(node) {}
         
         reference operator*() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node;
         }
         pointer operator->() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node;
         }
      };

      /// @brief An iterator that performs an in-order traversal on the tree, returning const nodes.
      ///
      class const_inorder_iterator : public inorder_iterator_base<ConstSharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = ConstSharedNode;
         using pointer = value_type *;
         using reference = value_type &;

         const_inorder_iterator(ConstSharedNode node) : inorder_iterator_base<ConstSharedNode>(node) {}

         reference operator*() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node;
         }
         pointer operator->() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node;
         }
      };

      /// @brief An iterator that performs a pre-order traversal on the tree.
      ///
      class preorder_iterator : public preorder_iterator_base<SharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = SharedNode;
         using pointer = value_type *;
         using reference = value_type &;

         preorder_iterator(SharedNode node) : preorder_iterator_base<SharedNode>(node) {}
         
         reference operator*() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node;
         }
         pointer operator->() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node;
         }
      };

      /// @brief An iterator that performs a pre-order traversal on the tree, returning const nodes.
      ///
      class const_preorder_iterator : public preorder_iterator_base<ConstSharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = ConstSharedNode;
         using pointer = value_type *;
         using reference = value_type &;

         const_preorder_iterator(ConstSharedNode node) : preorder_iterator_base<ConstSharedNode>(node) {}

         reference operator*() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node;
         }
         pointer operator->() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node;
         }
      };

      /// @brief An iterator that performs a post-order traversal on the tree.
      ///
      class postorder_iterator : public postorder_iterator_base<SharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = SharedNode;
         using pointer = value_type *;
         using reference = value_type &;

         postorder_iterator(SharedNode node) : postorder_iterator_base<SharedNode>(node) {}
         
         reference operator*() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node;
         }
         pointer operator->() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node;
         }
      };

      /// @brief An iterator that performs a post-order traversal on the tree, returning const nodes.
      ///
      class const_postorder_iterator : public postorder_iterator_base<ConstSharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = ConstSharedNode;
         using pointer = value_type *;
         using reference = value_type &;

         const_postorder_iterator(ConstSharedNode node) : postorder_iterator_base<ConstSharedNode>(node) {}

         reference operator*() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node;
         }
         pointer operator->() {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node;
         }
      };

      /// @brief An iterator that yields value objects instead of nodes.
      ///
      /// @tparam iterator_base The base iterator to use as an iterator for this iterator. Options
      /// are inorder_iterator, preorder_iterator and postorder_iterator.
      ///
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

      /// @brief An iterator that yields const value objects instead of nodes.
      ///
      /// @tparam iterator_base The base iterator to use as an iterator for this iterator. Options
      /// are inorder_iterator, preorder_iterator and postorder_iterator.
      ///
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

      /// @brief Return an iterator at the beginning of an in-order traversal.
      ///
      inorder_iterator begin_inorder() { return inorder_iterator(this->_root); }
      /// @brief Return an iterator at the end of an in-order traversal.
      ///
      inorder_iterator end_inorder() { return inorder_iterator(nullptr); }
      /// @brief Return a const iterator at the beginning of an in-order traversal.
      ///
      const_inorder_iterator cbegin_inorder() const { return const_inorder_iterator(this->_root); }
      /// @brief Return a const iterator at the end of an in-order traversal.
      ///
      const_inorder_iterator cend_inorder() const { return const_inorder_iterator(nullptr); }

      /// @brief Return an iterator at the beginning of a pre-order traversal.
      ///
      preorder_iterator begin_preorder() { return preorder_iterator(this->_root); }
      /// @brief Return an iterator at the end of a pre-order traversal.
      ///
      preorder_iterator end_preorder() { return preorder_iterator(nullptr); }
      /// @brief Return a const iterator at the beginning of a pre-order traversal.
      ///
      const_preorder_iterator cbegin_preorder() const { return const_preorder_iterator(this->_root); }
      /// @brief Return a const iterator at the end of a pre-order traversal.
      ///
      const_preorder_iterator cend_preorder() const { return const_preorder_iterator(nullptr); }

      /// @brief Return an iterator at the beginning of a post-order traversal.
      ///
      postorder_iterator begin_postorder() { return postorder_iterator(this->_root); }
      /// @brief Return an iterator at the end of a post-order traversal.
      ///
      postorder_iterator end_postorder() { return postorder_iterator(nullptr); }
      /// @brief Return a const iterator at the beginning of a post-order traversal.
      ///
      const_postorder_iterator cbegin_postorder() const { return const_postorder_iterator(this->_root); }
      /// @brief Return a const iterator at the end of a post-order traversal.
      ///
      const_postorder_iterator cend_postorder() const { return const_postorder_iterator(nullptr); }

      /// @brief Returns a value iterator at the beginning of an in-order traversal.
      ///
      value_iterator<inorder_iterator> begin_values_inorder() { return value_iterator<inorder_iterator>(this->_root); }
      /// @brief Returns a value iterator at the end of an in-order traversal.
      ///
      value_iterator<inorder_iterator> end_values_inorder() { return value_iterator<inorder_iterator>(nullptr); }
      /// @brief Returns a const value iterator at the beginning of an in-order traversal.
      ///
      const_value_iterator<const_inorder_iterator> cbegin_values_inorder() { return const_value_iterator<const_inorder_iterator>(this->_root); }
      /// @brief Returns a const value iterator at the end of an in-order traversal.
      ///
      const_value_iterator<const_inorder_iterator> cend_values_inorder() { return const_value_iterator<const_inorder_iterator>(nullptr); }

      /// @brief Returns a value iterator at the beginning of a pre-order traversal.
      ///
      value_iterator<preorder_iterator> begin_values_preorder() { return value_iterator<preorder_iterator>(this->_root); }
      /// @brief Returns a value iterator at the end of a pre-order traversal.
      ///
      value_iterator<preorder_iterator> end_values_preorder() { return value_iterator<preorder_iterator>(nullptr); }
      /// @brief Returns a const value iterator at the beginning of a pre-order traversal.
      ///
      const_value_iterator<const_preorder_iterator> cbegin_values_preorder() { return const_value_iterator<const_preorder_iterator>(this->_root); }
      /// @brief Returns a const value iterator at the end of a pre-order traversal.
      ///
      const_value_iterator<const_preorder_iterator> cend_values_preorder() { return const_value_iterator<const_preorder_iterator>(nullptr); }

      /// @brief Returns a value iterator at the beginning of a post-order traversal.
      ///
      value_iterator<postorder_iterator> begin_values_postorder() { return value_iterator<postorder_iterator>(this->_root); }
      /// @brief Returns a value iterator at the end of a post-order traversal.
      ///
      value_iterator<postorder_iterator> end_values_postorder() { return value_iterator<postorder_iterator>(nullptr); }
      /// @brief Returns a const value iterator at the beginning of a post-order traversal.
      ///
      const_value_iterator<const_postorder_iterator> cbegin_values_postorder() { return const_value_iterator<const_postorder_iterator>(this->_root); }
      /// @brief Returns a const value iterator at the end of a post-order traversal.
      ///
      const_value_iterator<const_postorder_iterator> cend_values_postorder() { return const_value_iterator<const_postorder_iterator>(nullptr); }

      /// @brief Returns a default iterator to the beginning of the tree.
      ///
      /// The default iterator is a post-order value iterator.
      ///
      iterator begin() { return iterator(this->_root); }
      /// @brief Returns a default iterator to the end of the tree.
      ///
      /// The default iterator is a post-order value iterator.
      ///
      iterator end() { return iterator(nullptr); }

      /// @brief Returns a default const iterator to the beginning of the tree.
      ///
      /// The default iterator is a post-order value iterator.
      ///
      const_iterator cbegin() const { return const_iterator(this->_root); }
      /// @brief Returns a default const iterator to the end of the tree.
      ///
      /// The default iterator is a post-order value iterator.
      ///
      const_iterator cend() const { return const_iterator(nullptr); }

      /// @brief Determine if the tree is empty.
      /// @returns True if the tree is empty, false otherwise.
      ///
      inline bool is_empty() const { return this->_root == nullptr; }
      /// @brief Get the root node of this tree.
      ///
      inline SharedNode root() { return this->_root; }
      /// @brief Get the const root node of this tree.
      ///
      inline ConstSharedNode root() const { return this->_root; }
      /// @brief Determine if the given key exists in the tree.
      /// @param key The key value to search for.
      /// @returns True if the key was found, false otherwise.
      ///
      bool contains(const Key &key) const {
         if (this->_root == nullptr) { return false; }
         
         auto traversal = this->search(key);
         auto last_node = *traversal.rbegin();

         return last_node.second == 0;
      }
      /// @brief Search the tree for the given key, returning const nodes.
      ///
      /// This function does a basic binary traversal on the tree for the given key, with the ability
      /// to return the path taken in order to find that key-- or to not find that key. It will immediately
      /// terminate when it finds the given key.
      ///
      /// @param key The key value to search for.
      /// @returns A vector of pairs: the node that was traversed, and the number representing the path
      /// which was taken. 1 means right, -1 means left, 0 means it matched the key.
      ///
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
      /// @brief Search the tree for the given key.
      ///
      /// This function does a basic binary traversal on the tree for the given key, with the ability
      /// to return the path taken in order to find that key-- or to not find that key. It will immediately
      /// terminate when it finds the given key.
      ///
      /// @param key The key value to search for.
      /// @returns A vector of pairs: the node that was traversed, and the number representing the path
      /// which was taken. 1 means right, -1 means left, 0 means it matched the key.
      ///
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
      /// @brief Attempt to find the node corresponding to the given key in this tree.
      ///
      /// @param key The key to search for.
      /// @returns The node with the given key, or std::nullopt if no node was found.
      ///
      std::optional<SharedNode> find(const Key &key) {
         if (this->_root == nullptr) { return std::nullopt; }
         
         auto traversal = this->search(key);
         auto last_node = *traversal.rbegin();

         if (last_node.second == 0) { return last_node.first; }
         else { return std::nullopt; }
      }
      /// @brief Attempt to find the const node corresponding to the given key in this tree.
      ///
      /// @param key The key to search for.
      /// @returns The node with the given key, or std::nullopt if no node was found.
      ///
      std::optional<ConstSharedNode> find(const Key &key) const {
         if (this->_root == nullptr) { return std::nullopt; }
         
         auto traversal = this->search(key);
         auto last_node = *traversal.rbegin();

         if (last_node.second == 0) { return last_node.first; }
         else { return std::nullopt; }
      }
      /// @brief Attempt to get the node with the given key in the tree, throwing an exception if it fails.
      /// @param key The key to search for.
      /// @returns The node corresponding to the given key.
      /// @throws exception::KeyNotFound Thrown if the key is not found in the tree.
      ///
      SharedNode get(const Key &key) {
         auto result = this->find(key);

         if (result == std::nullopt) { throw exception::KeyNotFound(); }
         return *result;
      }
      /// @brief Attempt to get the const node with the given key in the tree, throwing an exception if it fails.
      /// @param key The key to search for.
      /// @returns The node corresponding to the given key.
      /// @throws exception::KeyNotFound Thrown if the key is not found in the tree.
      ///
      ConstSharedNode get(const Key &key) const {
         auto result = this->find(key);

         if (result == std::nullopt) { throw exception::KeyNotFound(); }
         return *result;
      }
      /// @brief Insert the given value into the tree.
      ///
      /// See AVLTreeBase::add_node.
      ///
      SharedNode insert(const Value &value) {
         return this->add_node(value);
      }
      /// @brief Remove a node with the given key from the tree.
      ///
      /// See AVLTreeBase::remove_node.
      ///
      /// @param key The key to remove from the tree.
      /// @throws exception::KeyNotFound Thrown if the key isn't found in the tree.
      ///
      void remove(const Key &key) {
         if (this->_root == nullptr) { return; }
         
         auto traversal = this->search(key);
         auto last_node = *traversal.rbegin();
         if (last_node.second != 0) { throw exception::KeyNotFound(); }
         
         this->remove_node(last_node.first->value());
      }
      /// @brief Convert this tree into a vector.
      ///
      /// Performs a post-order traversal on the tree and gets const values from the nodes.
      ///
      /// @returns A vector of values in the tree.
      std::vector<Value> to_vec() const {
         return std::vector<Value>(this->cbegin(), this->cend());
      }
      /// @brief Return the number of elements in this tree.
      ///
      inline std::size_t size() const {
         return this->_size;
      }
      /// @brief Destroy this tree.
      ///
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
      /// @brief Copy the given tree into this tree.
      ///
      /// This will destroy the current tree if it exists.
      ///
      /// @param other The other tree to copy.
      ///
      void copy(const AVLTreeBase &other) {
         if (this->_root != nullptr)
            this->destroy();
         
         if (other._root == nullptr) { return; }

         std::vector<ConstSharedNode> visiting = { other._root };
         std::vector<SharedNode> added = { this->copy_node(other._root) };
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

   /// @brief A functor for AVLTreeBase which treats the value argument as the key.
   /// @tparam Value The class of the value type.
   ///
   template <typename Value>
   struct KeyIsValue {
      const Value &operator() (const Value &value) const { return value; }
   };

   /// @brief A simplified AVLTree interface.
   ///
   /// Note that values in this tree cannot be modified, because they are keys, which
   /// are expected to be const within the tree. If you need to modify a value with a
   /// given key, you might be looking for a map, in which case you can use AVLMap.
   ///
   /// @tparam Key The class of the key of this tree.
   /// @tparam KeyCompare The comparison functor for the given key type. Defaults to std::less<Key>.
   /// See AVLTreeBase for Compare functor requirements.
   ///
   template <typename Key, typename KeyCompare=std::less<Key>>
   class AVLTree : public AVLTreeBase<Key, Key, KeyIsValue<Key>, KeyCompare>
   {
   public:
      using TreeBase = AVLTreeBase<Key, Key, KeyIsValue<Key>, KeyCompare>;
      using iterator = typename TreeBase::const_iterator;

      AVLTree() : TreeBase() {}
      AVLTree(std::vector<Key> &nodes) : TreeBase(nodes) {}
      AVLTree(const AVLTree &other) : TreeBase(other) {}

      /// @brief Return an iterator of values at the beginning of this tree.
      ///
      /// This performs a post-order traversal on the tree. See AVLTreeBase::const_iterator.
      ///
      iterator begin() const { return iterator(this->root()); }
      /// @brief Return an iterator of values at the end of this tree.
      ///
      iterator end() const { return iterator(nullptr); }
      /// @brief Return a const iterator of values at the beginning of this tree.
      ///
      /// This performs a post-order traversal on the tree. See AVLTreeBase::const_iterator.
      ///
      iterator cbegin() const { return this->begin(); }
      /// @brief Return an iterator of values at the end of this tree.
      ///
      iterator cend() const { return this->end(); }
   };

   /// @brief A functor which treats the first value of a std::pair as its key.
   ///
   /// @tparam Key The class of the key type.
   /// @tparam Value The class of the value type.
   ///
   template<typename Key, typename Value>
   struct KeyOfPair {
      const Key &operator() (const std::pair<const Key, Value> &pair) const { return pair.first; }
   };

   /// @brief A mapping implementation based on an AVL tree.
   ///
   /// This is not intended to be a replacement for std::map, but rather simply act as a map-like interface
   /// to an AVL tree for further use.
   ///
   /// @tparam Key The type of the key for the mapping.
   /// @tparam Value The type of the value for the mapping.
   /// @tparam KeyCompare The key comparison functor for sorting the nodes. See AVLTreeBase for Comparison
   /// requirements.
   ///
   template <typename Key, typename Value, typename KeyCompare=std::less<Key>>
   class AVLMap : public AVLTreeBase<Key, std::pair<const Key, Value>, KeyOfPair<Key, Value>, KeyCompare>
   {
   public:
      using TreeBase = AVLTreeBase<Key, std::pair<const Key, Value>, KeyOfPair<Key, Value>, KeyCompare>;
      
      AVLMap() : TreeBase() {}
      AVLMap(std::vector<std::pair<const Key, Value>> &nodes) : TreeBase(nodes) {}
      AVLMap(const AVLMap &other) : TreeBase(other) {}

      /// @brief Access the given mapping value with the given key.
      ///
      /// **NOTE**: This will create a new node if the key does not exist. To not do this,
      /// you can use AVLTreeBase::get or use the const equivalent of this operator.
      ///
      /// @param key The key value to search for.
      /// @returns The value associated with the given key.
      ///
      Value &operator[](const Key &key) {
         try {
            return this->get(key);
         }
         catch (exception::KeyNotFound &) {
            auto node = this->add_node(std::make_pair(key, Value()));
            return node->value().second;
         }
      }
      /// @brief Access the given const mapping value with the given key.
      /// @param key The key value to search for.
      /// @returns The value associated with the given key.
      /// @throws exception::KeyNotFound Thrown when the key was not found in the tree.
      ///
      const Value &operator[](const Key &key) const { return this->get(key); }

      /// @brief Check if the given key exists in the tree.
      ///
      /// See AVLTreeBase::contains.
      ///
      /// @param key The key to search for.
      /// @returns True if found, false otherwise.
      ///
      bool has_key(const Key &key) const {
         return this->contains(key);
      }

      /// @brief Insert a given key-value pair into the tree.
      /// @param key The key to associate with the new node.
      /// @param value The value to give the new node.
      ///
      void insert(const Key &key, const Value &value) {
         TreeBase::insert(std::make_pair(key, value));
      }

      /// @brief Get the value associated with the given key.
      /// @param key The key to get.
      /// @returns The value associated with the given key.
      /// @throws exception::KeyNotFound Thrown if the given key isn't found.
      ///
      Value &get(const Key &key) {
         return TreeBase::get(key)->value().second;
      }

      /// @brief Get the const value associated with the given key.
      /// @param key The key to get.
      /// @returns The value associated with the given key.
      /// @throws exception::KeyNotFound Thrown if the given key isn't found.
      ///
      const Value &get(const Key &key) const {
         return TreeBase::get(key)->value().second;
      }
   };
}

#endif

