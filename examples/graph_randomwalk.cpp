// Visualize a random walk generated with sm::random_walk
#include <memory>
#include <cmath>
#include <cstdint>
import mplot.visual;
import mplot.graphvisual;
import sm.random_walk;

int main()
{
    const std::uint32_t n_steps = 1000;
    const std::uint32_t a_tau = 50;
    const float kappa = 10.0f;
    const float acc_max = 0.1f;
    sm::random_walk<float> walk (n_steps, a_tau, kappa, acc_max);

    sm::vvec<sm::vec<float, 2>> coords = walk.generate();

    mplot::Visual v(1024, 768, "A random walk");
    auto gv = std::make_unique<mplot::GraphVisual<float>> (sm::vec<float>{0,0,0});
    gv->set_parent (v.get_id());
    mplot::DatasetStyle ds (mplot::stylepolicy::lines);
    gv->setdata (coords, ds);
    gv->finalize();
    v.addVisualModel (gv);
    v.keepOpen();
}
