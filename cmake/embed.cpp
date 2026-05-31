#include "stdio.h"
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace std;


int main(int argc, char** argv) {
	string outfile(argv[1]);

	ofstream header;
	header.open(outfile + ".h");

	ofstream source;
	source.open(outfile + ".c");

	header << "#pragma once" << std::endl << std::endl;

	header << R"abc(
#ifdef __cplusplus
extern "C" {
#endif
		)abc";
	source << std::format("#include \"{}.h\"", outfile) << std::endl << std::endl;

	for (int i = 2; i < argc; i++) {

		filesystem::path p(argv[i]);
		ifstream file;

		file.open(p, std::ios::in | std::ios::binary);

		file.seekg(0, std::ios::end);
		int length = file.tellg();
		file.seekg(0, std::ios::beg);

		printf("Transforming file %s (%d bytes)\n", argv[i], length);

		auto filename = p.filename().stem().string();
		auto extension = p.filename().extension().string().substr(1);

		header << std::format("extern const char embed_{}_{}[];", filename, extension) << std::endl;

		source << std::format("const char embed_{}_{}[] = {{ ", filename, extension) << std::endl;

		for (int b = 0; b < length; b++) {
			char val;
			file.read(&val, 1);

			source << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(val) << ",";
		}

		source << "0x00\n};\n";
	}

	header << R"abc(
#ifdef __cplusplus
}
#endif
)abc";

	header.flush();
	header.close();

	source.flush();
	source.close();


	return 0;
}

