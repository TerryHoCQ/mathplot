 /*
 * Test convolution of some data defined on a HexGrid (using HexGrid::convolve)
 *
 * This is modified to run with OpenGL 3.1 ES instead of the default OpenGL 4.1. Consequently it
 * runs on a Raspberry Pi.
 */
#include <iostream>
#include <vector>
#include <string>
#include <cmath>

#include <sm/random>
#include <sm/scale>
#include <sm/vec>
#include <sm/hexgrid>

#include <mplot/Visual.h>
#include <mplot/VisualDataModel.h>
#include <mplot/HexGridVisual.h>

int main()
{
    int rtn = 0;

    mplot::Visual<mplot::gl::version_3_1_es> v(800,600,"Convolution window");
    v.zNear = 0.001;
    v.backgroundBlack();
    v.setSceneTransZ (-3.0f);

    // Create an elliptical hexgrid for the input/output domains
    sm::hexgrid hg(0.01, 3, 0);
    hg.setEllipticalBoundary (0.45, 0.3);

    // Populate a vector of floats with data
    std::vector<float> data (hg.num(), 0.0f);
    sm::rand_uniform<float> rng;
    float nonconvolvedSum = 0.0f;
    for (float& d : data) {
        d = rng.get();
        nonconvolvedSum += d;
    }

    // Create a circular hexgrid to contain the Gaussian convolution kernel
    float sigma = 0.025f;
    sm::hexgrid kernel(0.01, 20.0f*sigma, 0);
    kernel.setCircularBoundary (6.0f*sigma);
    std::vector<float> kerneldata (kernel.num(), 0.0f);
    // Once-only parts of the calculation of the Gaussian.
    float one_over_sigma_root_2_pi = 1 / sigma * 2.506628275;
    float two_sigma_sq = 2.0f * sigma * sigma;
    // Gaussian dist. result, and a running sum of the results:
    float gauss = 0;
    float sum = 0;
    for (auto& k : kernel.hexen) {
        // Gaussian profile based on the hex's distance from centre, which is
        // already computed in each hex as hex::r
        gauss = (one_over_sigma_root_2_pi * std::exp ( -(k.r*k.r) / two_sigma_sq ));
        kerneldata[k.vi] = gauss;
        sum += gauss;
    }
    // Renormalise
    for (auto& k : kernel.hexen) { kerneldata[k.vi] /= sum; }

    // A vector for the result
    std::vector<float> convolved (hg.num(), 0.0f);

    // Call the convolution method from hexgrid:
    hg.convolve (kernel, kerneldata, data, convolved);

    float convolvedSum = 0.0f;
    for (float& d : convolved) { convolvedSum += d; }

    std::cout << "Unconvolved sum: " << nonconvolvedSum << ", convolved sum: " << convolvedSum << "\n";

    // Visualize the 3 maps
    sm::vec<float, 3> offset = { -0.5, 0.0, 0.0 };
    auto hgv = std::make_unique<mplot::HexGridVisual<float, mplot::gl::version_3_1_es>>(&hg, offset);
    v.bindmodel (hgv);
    hgv->setScalarData (&data);
    hgv->cm.setType(mplot::ColourMapType::Viridis);
    hgv->addLabel ("Input", { -0.3f, -0.45f, 0.01f }, mplot::TextFeatures(0.1f, mplot::colour::white));
    hgv->finalize();
    // Get the non-owning pointer to hgv from the addVisualModel call
    auto hgvp = v.addVisualModel (hgv);

    offset[1] += 0.6f;
    auto kgv = std::make_unique<mplot::HexGridVisual<float, mplot::gl::version_3_1_es>>(&kernel, offset);
    v.bindmodel (kgv);
    kgv->setScalarData (&kerneldata);
    kgv->cm.setType(mplot::ColourMapType::Viridis);
    kgv->finalize();
    auto kgvp = v.addVisualModel (kgv);

    // Labels can be added after finalize() and after addVisualModel
    kgvp->addLabel ("Kernel", { 0.1f, 0.14f, 0.01f }, mplot::TextFeatures(0.1f, mplot::colour::white));

    offset[1] -= 0.6f;
    offset[0] += 1.0f;
    auto rgv = std::make_unique<mplot::HexGridVisual<float, mplot::gl::version_3_1_es>>(&hg, offset);
    v.bindmodel (rgv);
    rgv->setScalarData (&convolved);
    rgv->cm.setType(mplot::ColourMapType::Viridis);
    rgv->finalize();
    rgv->addLabel ("Output", { -0.3f, -0.45f, 0.01f }, mplot::TextFeatures(0.1f, mplot::colour::white));
    rgv->finalize();
    auto rgvp = v.addVisualModel (rgv);

    // Demonstrate how to divide existing scale by 10:
    float newGrad = hgvp->zScale.get_params(0) / 10.0f;
    // Set this in a new zscale object:
    sm::scale<float> zscale;
    zscale.set_params (newGrad, 0);
    // Use the un-owned pointer rgvp:
    rgvp->updateZScale (zscale);

    v.keepOpen();

    return rtn;
}
