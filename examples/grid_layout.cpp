/*
 * An example program showing you how the indices are arranged in Grids with different orders. This
 * prints out the grid to stdout for inspection.
 */
#include <iostream>
import sm.vec;
import sm.grid;

int main()
{
    sm::vec<float, 2> dx = { 1, 1 };
    sm::vec<float, 2> offset = { 0, 0 };

    sm::griddomainwrap wrap = sm::griddomainwrap::none;

    sm::grid<int, float> g_bltr(4, 2, dx, offset, wrap, sm::gridorder::bottomleft_to_topright);
    sm::grid<int, float> g_tlbr(4, 2, dx, offset, wrap, sm::gridorder::topleft_to_bottomright);
    sm::grid<int, float> g_bltrc(4, 2, dx, offset, wrap, sm::gridorder::bottomleft_to_topright_colmaj);
    sm::grid<int, float> g_tlbrc(4, 2, dx, offset, wrap, sm::gridorder::topleft_to_bottomright_colmaj);

    std::cout << "Calling grid::str() for a width 4, height 2 grid (different orders; offset (0,0)):\n\n";
    std::cout << g_bltr.str() << std::endl;
    std::cout << g_tlbr.str() << std::endl;
    std::cout << g_bltrc.str() << std::endl;
    std::cout << g_tlbrc.str() << std::endl;
    return 0;
}
