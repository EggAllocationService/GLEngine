#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct PakHeader {
    char magic[4];
    int version;
    int entryCount;
};

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <output.pak> <input1> <input2> ... <name1> <name2> ...\n";
        return 1;
    }

    const char* outputFile = argv[1];
    int remaining = argc - 2;
    if (remaining % 2 != 0) {
        std::cerr << "Error: number of input files and names must match.\n";
        return 1;
    }
    int entryCount = remaining / 2;

    std::ofstream out(outputFile, std::ios::binary);
    if (!out) {
        std::cerr << "Error: cannot open output file: " << outputFile << "\n";
        return 1;
    }

    PakHeader header;
    header.magic[0] = 'G';
    header.magic[1] = 'P';
    header.magic[2] = 'A';
    header.magic[3] = 'K';
    header.version = 1;
    header.entryCount = entryCount;

    out.write(header.magic, sizeof(header.magic));
    out.write(reinterpret_cast<const char*>(&header.version), sizeof(header.version));
    out.write(reinterpret_cast<const char*>(&header.entryCount), sizeof(header.entryCount));

    for (int i = 0; i < entryCount; ++i) {
        const char* inputPath = argv[2 + i];
        const char* name = argv[2 + entryCount + i];

        std::ifstream in(inputPath, std::ios::binary | std::ios::ate);
        if (!in) {
            std::cerr << "Error: cannot open input file: " << inputPath << "\n";
            return 1;
        }
        auto size = in.tellg();
        in.seekg(0, std::ios::beg);

        std::vector<char> buffer(static_cast<size_t>(size));
        if (!in.read(buffer.data(), size)) {
            std::cerr << "Error: cannot read input file: " << inputPath << "\n";
            return 1;
        }

        std::string nameStr(name);
        if (!nameStr.empty() && nameStr[0] != '/') {
            nameStr = '/' + nameStr;
        }

        unsigned short nameLength = static_cast<unsigned short>(nameStr.size());
        unsigned int dataLength = static_cast<unsigned int>(size);

        out.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
        out.write(nameStr.data(), nameLength);
        out.write(reinterpret_cast<const char*>(&dataLength), sizeof(dataLength));
        out.write(buffer.data(), dataLength);
    }

    out.close();
    std::cout << "Created " << outputFile << " with " << entryCount << " entries.\n";
    return 0;
}
