#include <framework.hpp>
#include <avltree.hpp>

#include <string>

using namespace avltree;

int test_avltree() {
   INIT();

   std::vector<std::uint32_t> nodes = { 5, 7, 2, 4, 3, 8, 10, 1, 0, 6, 9 };
   auto tree = AVLTree<std::uint32_t>(nodes);

   std::vector<std::uint32_t> inorder_expected = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
   std::vector<std::uint32_t> inorder_result(tree.begin_values_inorder(), tree.end_values_inorder());
   std::vector<std::uint32_t> preorder_expected = { 5, 3, 1, 0, 2, 4, 8, 7, 6, 10, 9 };
   std::vector<std::uint32_t> preorder_result(tree.begin_values_preorder(), tree.end_values_preorder());
   std::vector<std::uint32_t> postorder_expected = { 0, 2, 1, 4, 3, 6, 7, 9, 10, 8, 5 };
   std::vector<std::uint32_t> postorder_result(tree.begin_values_postorder(), tree.end_values_postorder());

   ASSERT(inorder_result == inorder_expected);
   ASSERT(preorder_result == preorder_expected);
   ASSERT(postorder_result == postorder_expected);

   COMPLETE();
}

int test_avlmap() {
   INIT();

   AVLMap<std::string, std::uint32_t> map;
   ASSERT(map.size() == 0);
   
   map["abad1dea"] = 0xabad1dea;
   map["deadbeef"] = 0xdeadbeef;
   map["facebabe"] = 0xfacebabe;
   map["defaced1"] = 0xdefaced1;
   
   ASSERT(map.size() == 4);
   ASSERT(map["abad1dea"] == 0xabad1dea);
   ASSERT_THROWS(map.get("badkey") == 0, exception::KeyNotFound);

   map["abad1dea"] = 0;
   ASSERT(map["abad1dea"] == 0);
   ASSERT(map.size() == 4);

   ASSERT_SUCCESS(map.remove("abad1dea"));
   ASSERT(map.size() == 3);
   ASSERT_THROWS(map.get("abad1dea") == 0, exception::KeyNotFound);

   for (auto iter=map.begin(); iter!=map.end(); ++iter)
      iter->second = 0;

   ASSERT(map.has_key("deadbeef") && map["deadbeef"] == 0);
   
   COMPLETE();
}

int
main
(int argc, char *argv[])
{
   INIT();

   LOG_INFO("Testing AVLTree.");
   PROCESS_RESULT(test_avltree);

   LOG_INFO("Testing AVLMap.");
   PROCESS_RESULT(test_avlmap);
      
   COMPLETE();
}
