#include "gtest/gtest.h"

using namespace std;

extern "C" int terminal(char *file);

TEST(File, JPG) {
	terminal("C:\\Users\\gorinje\\Desktop\\test\\Freedom.jpg.bvr");
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}