#include "IoUtils.h"
#include <fstream>
#include <assert.h>
std::vector<char> readFile(const char* fileName)
{
    std::ifstream input(fileName, std::ios::binary | std::ios::ate);
    if (!input) {
        return {};
    }
    size_t len = static_cast<size_t>(input.tellg());
    input.seekg(std::ios::beg);
    std::vector<char> res(len);
    input.read(res.data(), len);
    return res;
}