/*
 * An example mplot::Visual scene, containing a HexGrid, onto which is sampled an image.
 */

#include <iostream>
#include <vector>
#include <cmath>

#include <sm/vec>
#include <sm/vvec>
#include <sm/hexgrid>

#include <mplot/loadpng.h>
#include <mplot/Visual.h>
#include <mplot/VisualDataModel.h>
#include <mplot/HexGridVisual.h>

int main()
{
    mplot::Visual v(1600, 1000, "Demo of sm::hexgrid::resampleImage");

    sm::hexgrid hg(0.01f, 3.0f, 0.0f);
    hg.setCircularBoundary (1.2f);

    // Load an image with the help of mplot::loadpng().
    std::string fn = "../examples/bike256.png";
    sm::vvec<float> image_data;
    sm::vec<unsigned int, 2> dims = mplot::loadpng (fn, image_data);

    // This controls how large the photo will be on the HexGrid
    sm::vec<float,2> image_scale = {1.8f, 1.8f};
    // You can shift the photo with an offset if necessary
    sm::vec<float,2> image_offset = {0.0f, 0.0f};

    // Here's the HexGrid method that will resample the square pixel grid onto the hex grid
    sm::vvec<float> hex_image_data = hg.resampleImage (image_data, dims[1], image_scale, image_offset);

    // Now visualise with a HexGridVisual
    auto hgv = std::make_unique<mplot::HexGridVisual<float>>(&hg, sm::vec<float>({0,0,0}));
    v.bindmodel (hgv);

    // Set the image data as the scalar data for the HexGridVisual
    hgv->setScalarData (&hex_image_data);
    // The inverse greyscale map is appropriate for a monochrome image
    hgv->cm.setType (mplot::ColourMapType::GreyscaleInv);
    // As it's an image, we don't want relief, so set the zScale to have a zero gradient
    hgv->zScale.set_params (0, 1);

    hgv->finalize();
    v.addVisualModel (hgv);

    v.keepOpen();

    return 0;
}
