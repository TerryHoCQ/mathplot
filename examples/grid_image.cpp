#include <memory>
#include <iostream>
#include <string>

import sm.vec;
import sm.vvec;
import sm.grid;

import mplot.visual;
import mplot.gridvisual;
import mplot.loadpng;

int main()
{
    mplot::Visual v(1600, 1000, "Demo of Grid showing an image");

    sm::vec<float, 2> dx = { 0.02f, 0.02f };
    sm::vec<float, 2> nul = { 0.0f, 0.0f };
    // Top left to bottom right order matches image loaded by loadpng and avoids the need for a
    // vec<bool, 2> flip arg to mplot::loadpng.
    sm::grid g1(256U, 65U, dx, nul, sm::griddomainwrap::horizontal, // Triangles, TLBR
                sm::gridorder::topleft_to_bottomright);

    sm::grid g2(256U, 65U, dx, nul, sm::griddomainwrap::horizontal, // Triangles, BLTR
                sm::gridorder::bottomleft_to_topright);

    sm::grid g3(256U, 65U, dx, nul, sm::griddomainwrap::horizontal, // RectInterp TLBR
                sm::gridorder::topleft_to_bottomright);

    sm::grid g4(256U, 65U, dx, nul, sm::griddomainwrap::horizontal, // RectInterp, BLTR
                sm::gridorder::bottomleft_to_topright);

    // Load an image
    std::string fn = "../examples/bike256_65.png";
    sm::vvec<float> image_data_tlbr;
    sm::vec<unsigned int, 2> dims = mplot::loadpng (fn, image_data_tlbr, sm::vec<bool, 2>{false,false});
    sm::vvec<float> image_data_bltr;
    dims = mplot::loadpng (fn, image_data_bltr);
    std::cout << "Image dims: " << dims << std::endl;

    // Now visualise with a GridVisual
    auto gv1 = std::make_unique<mplot::GridVisual<float>>(&g1, sm::vec<float>({0,0,0}));
    gv1->set_parent (v.get_id());
    gv1->gridVisMode = mplot::GridVisMode::Triangles;
    gv1->setScalarData (&image_data_tlbr);
    gv1->cm.setType (mplot::ColourMapType::GreyscaleInv); // inverse greyscale is good for a monochrome image
    gv1->zScale.set_params (0, 1); // As it's an image, we don't want relief, so set the zScale to have a zero gradient
    gv1->finalize();
    v.addVisualModel (gv1);

    auto gv2 = std::make_unique<mplot::GridVisual<float>>(&g2, sm::vec<float>({6,0,0}));
    gv2->set_parent (v.get_id());
    gv2->gridVisMode = mplot::GridVisMode::Triangles;
    gv2->setScalarData (&image_data_bltr);
    gv2->cm.setType (mplot::ColourMapType::GreyscaleInv);
    gv2->zScale.set_params (0, 1);
    gv2->finalize();
    v.addVisualModel (gv2);

    auto gv3 = std::make_unique<mplot::GridVisual<float>>(&g3, sm::vec<float>({0,1.6,0}));
    gv3->set_parent (v.get_id());
    gv3->gridVisMode = mplot::GridVisMode::RectInterp;
    gv3->setScalarData (&image_data_tlbr);
    gv3->cm.setType (mplot::ColourMapType::GreyscaleInv);
    gv3->zScale.set_params (0, 1);
    gv3->finalize();
    v.addVisualModel (gv3);

    auto gv4 = std::make_unique<mplot::GridVisual<float>>(&g4, sm::vec<float>({6,1.6,0}));
    gv4->set_parent (v.get_id());
    gv4->gridVisMode = mplot::GridVisMode::RectInterp;
    gv4->setScalarData (&image_data_bltr);
    gv4->cm.setType (mplot::ColourMapType::GreyscaleInv);
    gv4->zScale.set_params (0, 1);
    gv4->finalize();
    v.addVisualModel (gv4);

    v.keepOpen();

    return 0;
}
