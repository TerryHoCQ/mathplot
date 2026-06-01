// Utility to debug some triangle intersection problems

#include <memory>
#include <iostream>
#include <array>
#include <string>

import sm.config;
import mplot.visual;
import mplot.visualmodel;
import mplot.spherevisual;
import mplot.vectorvisual;
import mplot.trianglevisual;

int main (int argc, char** argv)
{
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " '{ \"json\" : \"string\" }'\n\n"
                  << "An example:\n  " << argv[0] << " '{ \"t0\" : [-0.76,0.99,-0.87], \"t1\" : [-0.04,0.64,-1.4], \"t2\" : [-0.94,0.55,-1], \"l0\" : [-0.43,0.62,-1.24], \"l\" : [0.43,-0.43,0.8] }'\n\n"
                  << "where t1, t2 and t3 define triangle vertices and l0, l define ray start and direction\n\n"
                  << "This program can be used to visually debug sm::algo::ray_tri_intersection\n";

        return -1;
    }

    std::string jstr = std::string (argv[1]);
    std::cout << "json arg: " << jstr << std::endl;

    // {"t0" : [-0.760357976,0.990107834,-0.865470469],"t1" : [-0.0382558256,0.637388527,-1.44368529],"t2" : [-0.941664934,0.551642597,-1.00695968],"l0" : [-0.428883553,0.622524023,-1.24276352],"l" : [0.425372064,-0.432439089,0.795018792]}

    sm::config conf;
    conf.parse (jstr);
    std::cout << "Got t0: " << conf.get_vec<float, 3> ("t0") << std::endl;

    mplot::Visual v(1024, 768, "Ray-triangle intersection");
    v.lightingEffects();

    // Triangle vertices
    sm::vec<> v1 = conf.get_vec<float, 3> ("t0");
    sm::vec<> v2 = conf.get_vec<float, 3> ("t1");
    sm::vec<> v3 = conf.get_vec<float, 3> ("t2");
    // Ray start and direction
    sm::vec<> start = conf.get_vec<float, 3> ("l0");
    sm::vec<> dirn = conf.get_vec<float, 3> ("l");

    auto tv = std::make_unique<mplot::TriangleVisual<>> (sm::vec<>{}, v1, v2, v3, mplot::colour::blue);
    tv->set_parent (v.get_id());
    tv->finalize();
    [[maybe_unused]] auto tvp = v.addVisualModel (tv);

    float start_sphr = dirn.length() / 20.0f;
    auto sv = std::make_unique<mplot::SphereVisual<>>(start, start_sphr, mplot::colour::goldenrod3);
    sv->set_parent (v.get_id());
    sv->finalize();
    v.addVisualModel (sv);

    auto vvm = std::make_unique<mplot::VectorVisual<float, 3>>(start);
    vvm->set_parent (v.get_id());
    vvm->thevec = dirn;
    vvm->vgoes = mplot::VectorGoes::FromOrigin;
    vvm->thickness = 0.02f;
    vvm->arrowhead_prop = 0.1f;
    vvm->fixed_colour = true;
    vvm->single_colour = mplot::colour::crimson;
    vvm->finalize();
    [[maybe_unused]] auto vvmp = v.addVisualModel (vvm);

    tvp->make_navmesh();

    auto vm = tvp->getViewMatrix();
    auto vmi = vm.inverse();

    auto start_wr = (vmi * start).less_one_dim(); // wr to tvp
    std::cout << "start_wr = " << start_wr << std::endl;
    auto [hit, ti] = tvp->navmesh->find_triangle_crossing (start_wr, dirn, vm);
    if (ti == std::numeric_limits<uint32_t>::max()) {
        std::cout << "NO HIT\n";
    } else {
        std::cout << "Indices: " << ti << std::endl;
        std::cout << "Contains hit " << hit << std::endl;

        sv = std::make_unique<mplot::SphereVisual<>>(hit, start_sphr * 1.1f, mplot::colour::springgreen2);
        sv->set_parent (v.get_id());
        sv->finalize();
        v.addVisualModel (sv);
    }

    v.keepOpen();
    return 0;
}
