#include <framework.hpp>
#include <avltree.hpp>

using namespace avltree;

int test_avltree() {
   INIT();

   std::vector<std::uint32_t> nodes = { 5, 7, 2, 4, 3, 8, 10, 1, 0, 6, 9 };
   auto tree = AVLTree<std::uint32_t>(nodes);

   std::vector<std::uint32_t> inorder_expected = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
   std::vector<std::uint32_t> inorder_result(tree.begin_inorder(), tree.end_inorder());

   ASSERT(inorder_result == inorder_expected);

   COMPLETE();
}

int
main
(int argc, char *argv[])
{
   INIT();

   LOG_INFO("Testing AVLTree.");
   PROCESS_RESULT(test_avltree);
      
   COMPLETE();
}
