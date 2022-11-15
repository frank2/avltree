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

   template <typename Key, typename Node, typename Allocator>
   class AVLTree;

   template <typename Key, typename Compare=std::less<Key>>
   class AVLNode
   {
      template <typename _Key, typename _Node, typename Allocator>
      friend class AVLTree;

   public:
      using KeyType = typename Key;
      using CompareType = typename Compare;
      using SharedNode = std::shared_ptr<AVLNode<Key, Compare>>;
      using ConstSharedNode = std::shared_ptr<const AVLNode<Key, Compare>>;

   protected:
      Key _key;
      int _height;
      SharedNode _parent, _left, _right;

      AVLNode () : _key(Key()), _parent(nullptr), _left(nullptr), _right(nullptr), _height(0) {}
      AVLNode(const AVLNode &other) : _key(other._key), _parent(other._parent), _left(other._left), _right(other._right), _height(other._height) {}

   public:
      inline const Key &key() const { return this->_key; }
      inline int height() const { return this->_height; }
      inline SharedNode parent() { return this->_parent; }
      inline ConstSharedNode parent() const { return this->_parent; }
      inline SharedNode left() { return this->_left; }
      inline ConstSharedNode left() const { return this->_left; }
      inline SharedNode right() { return this->_right; }
      inline ConstSharedNode right() const { return this->_right; }

      inline bool is_leaf() const { return this->_left == nullptr && this->_right == nullptr; }

      int compare(const Key &key) const {
         auto ab = Compare()(key, this->_key);
         auto ba = Compare()(this->_key, key);

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
   
   template <typename Key, typename Node=AVLNode<Key>, typename Allocator=std::allocator<Node>>
   class AVLTree
   {
      template <class T, class R = void>  
      struct enable_if_type { typedef R type; };

      template <class T, class Enable = void>
      struct has_compare_type : std::false_type {};

      template <class T>
      struct has_compare_type<T, typename enable_if_type<typename T::CompareType>::type> : std::true_type {};
      
      static_assert(has_compare_type<Node>::value && std::is_base_of<AVLNode<Key, typename Node::CompareType>, Node>::value,
                    "Template argument Node must derive AVLNode as a base with the given Key argument.");
      static_assert(std::is_same<Node, Allocator::value_type>::value,
                    "Allocator must allocate Node objects.");
      
   public:
      using KeyType = typename Key;
      using NodeType = typename Node;
      using BaseType = AVLNode<Key, typename Node::CompareType>;
      using SharedNode = std::shared_ptr<Node>;
      using ConstSharedNode = std::shared_ptr<const Node>;
      using BaseNode = std::shared_ptr<BaseType>;
      using ConstBaseNode = std::shared_ptr<const BaseType>;
      
   private:
      struct AllocatorDeleter {
         Allocator allocator;
         AllocatorDeleter(Allocator &allocator) : allocator(allocator) {}
         void operator()(Node *ptr) {
            allocator.deallocate(ptr, 1);
         }
      };

   protected:
      template <typename NodeType>
      class inorder_iterator_base
      {
         static_assert(std::is_same<NodeType, typename SharedNode>::value || std::is_same<NodeType, typename ConstSharedNode>::value,
                       "Iterator template type must be a SharedNode or a ConstSharedNode.");
         
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = typename SharedNode;
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
         static_assert(std::is_same<NodeType, typename SharedNode>::value || std::is_same<NodeType, typename ConstSharedNode>::value,
                       "Iterator template type must be a SharedNode or a ConstSharedNode.");
         
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = typename SharedNode;
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
         static_assert(std::is_same<NodeType, typename SharedNode>::value || std::is_same<NodeType, typename ConstSharedNode>::value,
                       "Iterator template type must be a SharedNode or a ConstSharedNode.");
         
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = typename SharedNode;
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

      Allocator allocator;
      typename SharedNode root;
      std::size_t _size;

      void set_right_child(typename BaseNode &target, typename BaseNode &child) {
         if (target == nullptr) { throw exception::NullPointer(); }
         
         target->_right = child;

         if (child != nullptr)
            child->_parent = target;
      }

      void set_left_child(typename BaseNode &target, typename BaseNode &child) {
         if (target == nullptr) { throw exception::NullPointer(); }

         target->_left = child;

         if (child != nullptr)
            child->_parent = target;
      }

      void set_parent(typename BaseNode &target, typename BaseNode &parent) {
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

      void rotate_left(typename BaseNode &rotation_root) {
         if (rotation_root == nullptr) { throw exception::NullPointer(); }
         
         auto pivot_root = rotation_root->_right;
         auto left_child = pivot_root->_left;
         auto rotation_parent = rotation_root->_parent;

         if (rotation_parent == nullptr)
            this->root = std::static_pointer_cast<Node>(pivot_root);
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

      void rotate_right(typename BaseNode &rotation_root) {
         if (rotation_root == nullptr) { throw exception::NullPointer(); }
         
         auto pivot_root = rotation_root->_left;
         auto right_child = pivot_root->_right;
         auto rotation_parent = rotation_root->_parent;

         if (rotation_parent == nullptr)
            this->root = std::static_pointer_cast<Node>(pivot_root);
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

      void rebalance_node(typename BaseNode &node) {
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

      void update_height(BaseNode &node) {
         if (node == nullptr) { throw exception::NullPointer(); }
         
         BaseNode update = node;

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

            update = update->_parent;
         }
      }

      typename SharedNode allocate_node(const Key &key) {
         auto node = new (this->allocator.allocate(1)) Node();
         node->_key = key;

         return SharedNode(node, AllocatorDeleter(this->allocator));
      }

      typename SharedNode copy_node(ConstSharedNode node) {
         auto node = new (this->allocator.allocate(1)) Node(*node);

         return SharedNode(node, AllocatorDeleter(this->allocator));
      }
         
      typename SharedNode add_node(const Key &key) {
         if (this->root == nullptr)
         {
            this->root = this->allocate_node(key);
            ++this->_size;
            return this->root;
         }

         auto traversal = this->search(key);
         auto last_result = *traversal.rbegin();
         auto parent = std::dynamic_pointer_cast<BaseType>(last_result.first);
         auto branch = last_result.second;
         
         if (branch == 0) { throw exception::KeyExists(); }

         auto new_node = this->allocate_node(key);

         if (branch < 0) { this->set_left_child(parent, std::dynamic_pointer_cast<BaseType>(new_node)); }
         else { this->set_right_child(parent, std::dynamic_pointer_cast<BaseType>(new_node)); }

         this->update_height(std::dynamic_pointer_cast<BaseType>(new_node));

         ++this->_size;

         return new_node;
      }

      void remove_node(const Key &key) {
         if (this->is_empty()) { throw exception::EmptyTree(); }

         auto traversal = this->search(key);
         auto last_node = *traversal.rbegin();

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

            if (node == this->root)
            {
               this->root.reset();
            }
            else
            {
               auto parent = node->_parent;
               node->_parent.reset();

               this->update_height(parent);
            }
         }
         else if (node->_left == nullptr || node->_right == nullptr)
         {
            BaseNode replacement_node;

            if (node->_left == nullptr)
               replacement_node = node->_right;
            else
               replacement_node = node->_left;

            replacement_node->_parent = node->_parent;

            if (node == this->root)
            {
               this->root = std::static_pointer_cast<Node>(replacement_node);
               this->update_height(std::dynamic_pointer_cast<BaseType>(this->root));
            }
            else if (node->_parent->_left == node)
            {
               node->_parent->_left = replacement_node;
               this->update_height(node->_parent);
            }
            else if (node->_parent->_right == node)
            {
               node->_parent->_right = replacement_node;
               this->update_height(node->_parent);
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

            leftmost->_right = node->_right;
            leftmost->_left = node->_left;
            leftmost->_parent = node->_parent;
            leftmost->_height = node->_height;

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

            if (node == this->root)
               this->root = std::static_pointer_cast<Node>(leftmost);

            if (replaced_node != nullptr)
               this->update_height(replaced_node);
            else if (leftmost_parent != node)
               this->update_height(leftmost_parent);
            else
               this->update_height(leftmost);
         }

         node->_left.reset();
         node->_right.reset();
         node->_parent.reset();

         --this->_size;
      }

   public:
      class inorder_iterator : public inorder_iterator_base<typename SharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = typename SharedNode;
         using pointer = value_type *;
         using reference = value_type &;

         inorder_iterator(typename SharedNode node) : inorder_iterator_base(node) {}
         
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

      class const_inorder_iterator : public inorder_iterator_base<typename ConstSharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = typename ConstSharedNode;
         using pointer = value_type *;
         using reference = value_type &;

         const_inorder_iterator(typename ConstSharedNode node) : inorder_iterator_base(node) {}

         reference operator*() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node;
         }
         pointer operator->() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node;
         }
      };

      class preorder_iterator : public preorder_iterator_base<typename SharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = typename SharedNode;
         using pointer = value_type *;
         using reference = value_type &;

         preorder_iterator(typename SharedNode node) : preorder_iterator_base(node) {}
         
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

      class const_preorder_iterator : public preorder_iterator_base<typename ConstSharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = ConstSharedNode;
         using pointer = value_type *;
         using reference = value_type &;

         const_preorder_iterator(typename ConstSharedNode node) : preorder_iterator_base(node) {}

         reference operator*() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node;
         }
         pointer operator->() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node;
         }
      };

      class postorder_iterator : public postorder_iterator_base<typename SharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = typename SharedNode;
         using pointer = value_type *;
         using reference = value_type &;

         postorder_iterator(typename SharedNode node) : postorder_iterator_base(node) {}
         
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

      class const_postorder_iterator : public postorder_iterator_base<typename ConstSharedNode>
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = typename ConstSharedNode;
         using pointer = value_type *;
         using reference = value_type &;

         const_postorder_iterator(typename ConstSharedNode node) : postorder_iterator_base(node) {}

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
      class key_iterator : public iterator_base
      {
         static_assert(std::is_same<iterator_base, const_inorder_iterator>::value ||
                       std::is_same<iterator_base, const_preorder_iterator>::value ||
                       std::is_same<iterator_base, const_postorder_iterator>::value,
                       "Key iterator base must be an const_inorder_iterator, a const_preorder_iterator or a const_postorder_iterator.");

      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = const Key;
         using pointer = value_type *;
         using reference = value_type &;

         key_iterator(typename ConstSharedNode node) : iterator_base(node) {}

         reference operator*() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return this->node->key();
         }

         pointer operator->() const {
            if (this->node == nullptr) { throw exception::NullPointer(); }

            return &this->node->key();
         }
      };

      using iterator = key_iterator<const_inorder_iterator>;

      AVLTree() : _size(0) {}
      AVLTree(std::vector<Key> &nodes) : _size(0) {
         for (auto node : nodes)
            this->add_node(node);
      }
      AVLTree(const AVLTree &other) {
         this->copy(other);
      }
      ~AVLTree() {
         this->destroy();
      }

      inorder_iterator begin_inorder() { return inorder_iterator(this->root); }
      inorder_iterator end_inorder() { return inorder_iterator(nullptr); }
      const_inorder_iterator cbegin_inorder() const { return const_inorder_iterator(this->root); }
      const_inorder_iterator cend_inorder() const { return const_inorder_iterator(nullptr); }

      preorder_iterator begin_preorder() { return preorder_iterator(this->root); }
      preorder_iterator end_preorder() { return preorder_iterator(nullptr); }
      const_preorder_iterator cbegin_preorder() const { return const_preorder_iterator(this->root); }
      const_preorder_iterator cend_preorder() const { return const_preorder_iterator(nullptr); }

      postorder_iterator begin_postorder() { return postorder_iterator(this->root); }
      postorder_iterator end_postorder() { return postorder_iterator(nullptr); }
      const_postorder_iterator cbegin_postorder() const { return const_postorder_iterator(this->root); }
      const_postorder_iterator cend_postorder() const { return const_postorder_iterator(nullptr); }

      iterator begin() const { return iterator(this->root); }
      iterator end() const { return iterator(nullptr); }

      inline bool is_empty() const { return this->root == nullptr; }
      bool contains(const Key &key) const {
         auto traversal = this->search(key);
         auto last_node = *traversal.rbegin();

         return last_node.second == 0;
      }
      std::vector<std::pair<typename ConstSharedNode, int>> search(const Key &key) const {
         auto result = std::vector<std::pair<typename ConstSharedNode, int>>();
         if (this->root == nullptr) { return result; }
         
         typename ConstSharedNode node = std::dynamic_pointer_cast<const Node>(this->root);
         auto branch = node->compare(key);
         
         while (branch != 0 && node != nullptr)
         {
            result.push_back(std::make_pair(node, branch));

            if (branch < 0)
               node = std::static_pointer_cast<const Node>(node->left());
            else
               node = std::static_pointer_cast<const Node>(node->right());

            if (node != nullptr)
               branch = node->compare(key);
         }
            
         if (node != nullptr) { result.push_back(std::make_pair(node, branch)); }

         return result;
      }
      std::vector<std::pair<typename SharedNode, int>> search(const Key &key) {
         auto result = std::vector<std::pair<typename SharedNode, int>>();
         if (this->root == nullptr) { return result; }
         
         auto node = std::dynamic_pointer_cast<Node>(this->root);
         auto branch = node->compare(key);
         
         while (branch != 0 && node != nullptr)
         {
            result.push_back(std::make_pair(node, branch));

            if (branch < 0)
               node = std::static_pointer_cast<Node>(node->_left);
            else
               node = std::static_pointer_cast<Node>(node->_right);

            if (node != nullptr)
               branch = node->compare(key);
         }
            
         if (node != nullptr) { result.push_back(std::make_pair(node, branch)); }

         return result;
      }
      std::optional<typename SharedNode> find(const Key &key) {
         if (this->root == nullptr) { return std::nullopt; }
         
         auto traversal = this->search(key);
         auto last_node = *traversal.rbegin();

         if (last_node.second == 0) { return last_node.first; }
         else { return std::nullopt; }
      }
      std::optional<typename ConstSharedNode> find(const Key &key) const {
         if (this->root == nullptr) { return std::nullopt; }
         
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
      SharedNode insert(const Key &key) {
         return this->add_node(key);
      }
      void remove(const Key &key) {
         this->remove_node(key);
      }
      std::vector<typename SharedNode> to_vec() {
         return std::vector<typename SharedNode>(this->begin_postorder(), this->end_postorder());
      }
      std::vector<typename ConstSharedNode> to_vec() const {
         return std::vector<typename ConstSharedNode>(this->cbegin_postorder(), this->cend_postorder());
      }
      inline std::size_t size() const {
         return this->_size;
      }
      void destroy() {
         if (this->root == nullptr) { return; }
         std::vector<typename BaseNode> visiting = { this->root };

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

         this->root.reset();
      }
      void copy(const AVLTree &other) {
         if (this->root != nullptr)
            this->destroy();
         
         if (other.root == nullptr) { return; }

         std::vector<typename ConstSharedNode> visiting = { other.root };
         std::vector<typename SharedNode> added = { this->copy_node(other.root) }
         std::size_t parent_index = 0;

         this->root = added.front();

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
               visiting.push_back(std::dynamic_pointer_cast<const Node>(node->_left));
            }
            
            if (node->_right != nullptr) {
               auto right = this->copy_node(node->_right);
               this->set_right_child(new_node, right);
               added.push_back(right);
               visiting.push_back(std::dynamic_pointer_cast<const Node>(node->_right));
            }
         }
      }
   };

   template<typename Key, typename Value, typename Compare=std::less<Key>>
   class AVLMapNode : public AVLNode<Key, Compare>
   {
      template <typename _Key, typename _Node, typename Allocator>
      friend class AVLTree;

      template <typename _Key, typename _Value, typename _Node, typename Allocator>
      friend class AVLMap;
      
   protected:
      Value _value;
            
      AVLMapNode() : AVLNode() {}
      AVLMapNode(const AVLMapNode &other) : _value(other._value), AVLNode(other) {}
      
   public:
      using ValueType = typename Value;
      
      Value &value() { return this->_value; }
      const Value &value() const { return this->_value; }
   };

   template <typename Key, typename Value, typename Node=AVLMapNode<Key, Value>, typename Allocator=std::allocator<Node>>
   class AVLMap : public AVLTree<Key, Node, Allocator>
   {
      template <class T, class R = void>  
      struct enable_if_type { typedef R type; };

      template <class T, class Enable = void>
      struct has_value_type : std::false_type {};

      template <class T>
      struct has_value_type<T, typename enable_if_type<typename T::ValueType>::type> : std::true_type {};
      
      static_assert(has_value_type<Node>::value,
                    "Node class must derive AVLMapNode.");
      
      typename AVLTree::SharedNode add_node(const Key &key, const Value &value) {
         auto node = AVLTree::add_node(key);
         node->_value = value;

         return node;
      }
      
   public:
      class iterator : public AVLTree::postorder_iterator
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = std::pair<const Key *, Value *>;
         using pointer = value_type *;
         using reference = value_type &;

         iterator(typename AVLTree::SharedNode node) : AVLTree::postorder_iterator(node) {
            if (this->node == nullptr) { this->entry = std::make_pair<const Key *, Value *>(nullptr, nullptr); }
            else {
               auto node = postorder_iterator::operator*();
               this->entry = std::make_pair(&node->key(), &node->value());
            }
         }

         reference operator*() {
            return this->entry;
         }

         const reference operator*() const {
            return this->entry;
         }

         pointer operator->() {
            return &this->entry;
         }

         const pointer operator->() const {
            return &this->entry;
         }

         iterator& operator++() {
            postorder_iterator::operator++();

            if (this->node == nullptr) { this->entry = std::make_pair<const Key *, Value *>(nullptr, nullptr); }
            else {
               auto node = postorder_iterator::operator*();
               this->entry = std::make_pair(&node->key(), &node->value());
            }

            return *this;
         }
         iterator& operator++(int) { auto tmp = *this; ++(*this); return tmp; }

      protected:
         value_type entry;
      };

      class const_iterator : public AVLTree::const_postorder_iterator
      {
      public:
         using iterator_category = std::forward_iterator_tag;
         using difference_type = std::ptrdiff_t;
         using value_type = std::pair<const Key *, const Value *>;
         using pointer = value_type *;
         using reference = value_type &;

         const_iterator(typename AVLTree::ConstSharedNode node) : AVLTree::const_postorder_iterator(node) {
            if (this->node == nullptr) { this->entry = std::make_pair<const Key *, const Value *>(nullptr, nullptr); }
            else {
               auto node = const_postorder_iterator::operator*();
               this->entry = std::make_pair<const Key *, const Value *>(&node->key(), &node->value());
            }
         }

         const reference operator*() const {
            return this->entry;
         }

         const pointer operator->() const {
            return &this->entry;
         }

         const_iterator& operator++() {
            const_postorder_iterator::operator++();
                        
            if (this->node == nullptr) { this->entry = std::make_pair<const Key *, const Value *>(nullptr, nullptr); }
            else {
               auto node = const_postorder_iterator::operator*();
               this->entry = std::make_pair<const Key *, const Value *>(&node->key(), &node->value());
            }

            return *this;
         }
         const_iterator& operator++(int) { auto tmp = *this; ++(*this); return tmp; }

      protected:
         value_type entry;
      };

      AVLMap() : AVLTree() {}
      AVLMap(std::vector<typename AVLTree::KeyType> &nodes) : AVLTree(nodes) {}
      AVLMap(const AVLMap &other) : AVLTree(other) {}

      Value &operator[](const Key &key) {
         try {
            return this->get(key);
         }
         catch (exception::KeyNotFound &) {
            auto node = this->add_node(key, Value());
            return this->get(key);
         }
      }
      const Value &operator[](const Key &key) const { return this->get(key); }

      iterator begin() { return iterator(this->root); }
      iterator end() { return iterator(nullptr); }
      const_iterator cbegin() { return const_iterator(this->root); }
      const_iterator cend() { return const_iterator(nullptr); }
      
      bool has_key(const Key &key) const {
         return this->contains(key);
      }

      void insert(const Key &key, const Value &value) {
         this->add_node(key, value);
      }

      Value &get(const Key &key) {
         auto node = AVLTree::get(key);
         return node->value();
      }

      const Value &get(const Key &key) const {
         auto node = AVLTree::get(key);
         return node->value();
      }
   };
}

#endif

