#include <cassert>
#include <string>

int main() {
    std::string s = "hello";
    assert(s.length() == 5);
    assert(s + " world" == "hello world");
    return 0;
}