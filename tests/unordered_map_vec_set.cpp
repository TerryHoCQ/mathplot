#include <iostream>
#include <cstdint>
#include <set>
#include <unordered_map>
#include <sm/vec>

int main()
{
    std::unordered_map<sm::vec<float, 3>, std::set<uint32_t>, sm::vec<float, 3>::hash> equiv_v;

    std::set<uint32_t> st = { 1, 2, 3 };
    std::set<uint32_t> st2 = { 4, 5, 6 };

    sm::vec<float, 3> vc = { 0.1f, 0.2f, 0.3f };

    equiv_v[vc] = st;
    sm::vec<float, 3> vc2 = vc;
    vc2[0] += 0.2f;
    equiv_v[vc2] = st2;

    std::cout << "(orig) unordered_map[" << vc << "]:\n";
    for (auto el : equiv_v[vc]) {
        std::cout << "set element: " << el << std::endl;
    }

    sm::vec<float, 3> vc1 = vc;
    vc1[0] += 0.2f;
    std::cout << "+= 0.2 unordered_map[" << vc1 << "]:\n";
    for (auto el : equiv_v[vc1]) {
        std::cout << "set element: " << el << std::endl;
    }

    vc1[0] -= 0.2f;
    std::cout << "+= 0.2 -= unordered_map[" << vc1 << "]:\n";
    for (auto el : equiv_v[vc1]) {
        std::cout << "set element: " << el << std::endl;
    }
    if (vc1 != vc) { std::cout << "They're different anyway!\n"; }

    std::cout << "vc2 unordered_map[" << vc2 << "]:\n";
    for (auto el : equiv_v[vc2]) {
        std::cout << "set element: " << el << std::endl;
    }

    return 0;
}
