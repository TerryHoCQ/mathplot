/*
 * Test convolution of some data defined on a CartGrid (using CartGrid::convolve)
 */
#include <memory>
#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>

import sm.vec;
import sm.vvec;
import sm.cartgrid;

import mplot.visual;
import mplot.visualdatamodel;
import mplot.cartgridvisual;

std::int32_t main()
{
    std::int32_t rtn = 0;

    // This'll be a 256x64 grid. This constructor creates a 'non-centred' cartgrid.
    sm::cartgrid cg(0.01f, 0.01f, 0.0f, 0.0f, 256*0.01f-0.01f, 64*0.01f-0.01f);
    cg.set_boundary_on_outer_edge();

    // Populate a vector of floats with data
    sm::vvec<float> data (cg.num());
    data.randomize();
    float nonconvolvedSum = data.sum();

    // Create a small CartGrid to contain the convolution kernel
    //mplot::cartgrid kernel (0.01f, 0.01f, 0.05f, 0.05f);
    sm::cartgrid kernel (0.01f, 0.01f, 0.0f, 0.0f, 5*0.01f-0.01f, 5*0.01f-0.01f);
    kernel.set_boundary_on_outer_edge();

    sm::vvec<float> kdata(kernel.num());

    // Put a Gaussian in the kernel
    // Once-only parts of the calculation of the Gaussian.
    float sigma = 0.025f;
    float one_over_sigma_root_2_pi = 1 / sigma * 2.506628275f;
    float two_sigma_sq = 2.0f * sigma * sigma;
    // Gaussian dist. result, and a running sum of the results:
    float gauss = 0;
    float sum = 0;
    for (auto& k : kernel.rects) {
        // Gaussian profile based on the rects's distance from centre
        float d_x = k.x;
        float d_y = k.y;
        gauss = (one_over_sigma_root_2_pi * std::exp ( -(d_x * d_x + d_y * d_y) / two_sigma_sq ));
        kdata[k.vi] = gauss;
        sum += gauss;
    }
    // Renormalise
    for (auto& k : kernel.rects) { kdata[k.vi] /= sum; }

    // A vector for the result
    sm::vvec<float> convolved (cg.num(), 0.0f);

    // Call the convolution method from cartgrid:
    cg.convolve (kernel, kdata, data, convolved);

    float convolvedSum = 0.0f;
    for (float& d : convolved) { convolvedSum += d; }

    std::cout << "Unconvolved sum: " << nonconvolvedSum << ", convolved sum: " << convolvedSum << "\n";

    // Visualize the 3 maps
    mplot::Visual v(800,600,"Convolution window");

    sm::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };
    auto cgv = std::make_unique<mplot::CartGridVisual<float>>(&cg, offset);
    cgv->set_parent (v.get_id());
    cgv->cartVisMode = mplot::CartVisMode::RectInterp;
    cgv->setScalarData (&data);
    cgv->cm.setType (mplot::ColourMapType::GreyscaleInv);
    cgv->zScale.null_scaling();
    cgv->addLabel(std::string("Original"), sm::vec<float, 3>{0.0f,-0.13f,0.0f},
                  mplot::TextFeatures(0.1f, 48));
    cgv->finalize();
    v.addVisualModel (cgv);

    offset = { 0.0f, -0.3f, 0.0f };
    auto cgvk = std::make_unique<mplot::CartGridVisual<float>>(&kernel, offset);
    cgvk->set_parent (v.get_id());
    cgvk->cartVisMode = mplot::CartVisMode::RectInterp;
    cgvk->setScalarData (&kdata);
    cgvk->cm.setType (mplot::ColourMapType::GreyscaleInv);
    cgvk->zScale.null_scaling();
    cgvk->addLabel(std::string("Kernel"), sm::vec<float, 3>{0.0f,-0.13f,0.0f},
                   mplot::TextFeatures(0.1f, 48));
    cgvk->finalize();
    v.addVisualModel (cgvk);

    offset = { 0.0f, -1.3f, 0.0f };
    auto cgvr = std::make_unique<mplot::CartGridVisual<float>>(&cg, offset);
    cgvr->set_parent (v.get_id());
    cgvr->cartVisMode = mplot::CartVisMode::RectInterp;
    cgvr->setScalarData (&convolved);
    cgvr->cm.setType (mplot::ColourMapType::GreyscaleInv);
    cgvr->zScale.null_scaling();
    cgvr->addLabel (std::string("Convolved"), sm::vec<float, 3>{0.0f,-0.13f,0.0f},
                    mplot::TextFeatures(0.1f, 48));
    cgvr->finalize();
    v.addVisualModel (cgvr);

    v.keepOpen();

    return rtn;
}
