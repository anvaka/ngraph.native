// TODO: This should not duplicate code from xCode demo.
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <regex>

#include "../../src/layout.h"

using namespace std;

void save(int i, std::vector<Body> *bodies) {
    std::stringstream ss;

    ss << i << ".bin";
    std::ofstream outfile (ss.str(), std::ofstream::binary);

    char block[3 * 4];
    int *triplet = (int *)&block;
    for (vector<Body>::iterator body = bodies->begin() ; body != bodies->end(); ++body) {
        triplet[0] = floor(body->pos.x + 0.5);
        triplet[1] =floor(body->pos.y + 0.5);
        triplet[2] = floor(body->pos.z + 0.5);
        //    cout << triplet[0] << "," << triplet[1] << "," << triplet[2] << " <- node" << endl;
        outfile.write(block, 3 * 4);
    }
    outfile.close();
}

struct FileContent {
    int *content;
    size_t size;
};

FileContent readFile(const char *fileName) {
    streampos size;
    char * memblock;
    ifstream file (fileName, ios::in|ios::binary|ios::ate);
    if (file.is_open())
    {
        size = file.tellg();
        memblock = new char [size];
        file.seekg(0, ios::beg);
        file.read(memblock, size);
        file.close();
        FileContent result;
        result.size = size/4;
        result.content = (int *) memblock;
        return result;
    } else {
        throw "Could not open file";
    }
}

int getIterationNumberFromPositionFileName(const char *positionFileName) {
    cmatch match;
    regex pattern(".*?(\\d+)\\.bin$");
    regex_match(positionFileName, match, pattern);
    if (match.size() == 2) {
        try {
            return stoi(match[1]) + 1;
        }
        catch (...) {
            return 0;
        }
    }
    return 0;
}

int main(int argc, const char * argv[]) {
    if (argc < 2) {
        cout << "Usage: " << endl
        << "  layout++ links.bin [positions.bin]" << endl
        << "Where" << endl
        << " `links.bin` is a path to the serialized graph. See " << endl
        << "    https://github.com/anvaka/ngraph.tobinary for format description" << endl
        << "  `positoins.bin` is optional file with previously saved positions. " << endl
        << "    This file should match `links.bin` graph, otherwis bad things " << endl
        << "    will happen" << endl;
        return -1;
    }
    const char * graphFileName = argv[1];
    cout << argv[0] << endl;
    srand(42);

    cout << "Loading links from " << graphFileName << "... " << endl;
    FileContent graphFile = readFile(graphFileName);

    Layout graphLayout;
    int startFrom = 0;
    if (argc < 3) {
        graphLayout.init(graphFile.content, graphFile.size);
        cout << "Done. " << endl;
        cout << "Loaded " << graphLayout.getBodiesCount() << " bodies;" << endl;
    } else {
        const char * posFileName = argv[2];
        startFrom = getIterationNumberFromPositionFileName(posFileName);
        cout << "Loading positions from " << posFileName << "... ";
        FileContent positions = readFile(posFileName);
        cout << "Done." << endl;
        graphLayout.init(graphFile.content, graphFile.size, positions.content, positions.size);
        cout << "Loaded " << graphLayout.getBodiesCount() << " bodies;" << endl;
    }
    cout << "Starting layout from " << startFrom << " iteration;" << endl;

    for (int i = startFrom; i <= 10000; ++i) {
        cout << "Step " << i << endl;
        bool done = graphLayout.step();
        if (done) {
            cout << "Done!" << endl;
            break;
        }
        if (i % 5 == 0) {
            save(i, graphLayout.getBodies());
        }
    }

    delete[] graphFile.content;
}
