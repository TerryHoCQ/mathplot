/*
 * The scalar products of a set of randomly directed, normalised vectors should follow
 * the beta distribution. Here, I multiply their length by a normally distributed amount
 * near 1.
 */

#include <memory>

import sm.random;
import sm.histo;
import mplot.visual;
import mplot.graphvisual;

int main()
{
    static constexpr size_t N = 1000000;
    static constexpr size_t n = 2;

    // Create N normalized vectors at random.
    sm::vvec<sm::vec<float, n>> vVecs(N);
    sm::rand_uniform<float> rn_u(-1.0f, 1.0f);
    sm::rand_normal<float> rn_n(1.0f, 0.06f);
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < n; ++j) {
            vVecs[i][j] = rn_u.get();
        }
        // Renormalise
        vVecs[i].renormalize();
        // Multiply length by a normally distributed amount near 1
        vVecs[i] *= rn_n.get();
    }

    // Get scalar products between pairs
    sm::vvec<float> sp (N/n);
    for (size_t i = 0; i < N/n; ++i) { sp[i] = vVecs[i].dot (vVecs[i+N/n]); } // No good for n!=2

    // Make a histogram of the scalar product pairs
    sm::histo h(sp, 100);

    // Set up a mplot::Visual for a graph
    mplot::Visual v(1024, 768, "Histogram");

    // Create a new GraphVisual with offset within the scene of 0,0,0
    auto gv = std::make_unique<mplot::GraphVisual<float>> (sm::vec<float>({0,0,0}));
    gv->set_parent (v.get_id());
    gv->setdata (h); // to become gv->add_bargraph (h [,mplot::colour::darkorchid1] [,mplot::colour::orchid2])
    gv->xlabel = "Scalar product";
    gv->ylabel = "Proportion";
    gv->finalize();
    v.addVisualModel (gv);

    // Render the graph until user exits
    v.keepOpen();

    return 0;
}
