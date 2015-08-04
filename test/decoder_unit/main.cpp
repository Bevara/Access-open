#include "gtest/gtest.h"

using namespace std;

extern "C" int terminal(const char *file);
string folder;


TEST(File, JPG) {
	string file = folder;
	file.append("Freedom.jpg.bvr");
	terminal(file.c_str());
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);

	for (int i = 1; i < argc; i++) {
		if (i + 1 != argc)
			if (strcmp("-folder", argv[i]) == 0) {
				folder = argv[++i];
				std::cout << "Test signal folder set to " << folder << endl;
			}
	}

	return RUN_ALL_TESTS();
}