#include "gtest/gtest.h"

using namespace std;

extern "C" int terminal(const char *file);
string folder;


TEST(File, JPG) {
	string file = folder;
	file.append("Freedom.jpg.bvr");
	terminal(file.c_str());
}


TEST(File, PDF) {
	string file_page1, file_page2;
	string file = folder;
	file.append("TEST.pdf.bvr");
	file_page1 = file + "#page=1";
	file_page2 = file + "#page=2";
	terminal(file_page1.c_str());
	terminal(file_page2.c_str());
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