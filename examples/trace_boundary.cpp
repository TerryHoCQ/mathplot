/*
 * Trace around a boundary of points in 2D
 */
#include <iostream>

#include <sm/mathconst>
#include <sm/random>
#include <sm/vvec>
#include <sm/vec>
#include <sm/centroid>

#include <mplot/Visual.h>
#include <mplot/GraphVisual.h>


int main (int argc, char** argv)
{
    int nbp = 5;
    if (argc > 1) {
        nbp = std::stoi (argv[1]);
    }
    // Graph the data
    mplot::Visual v(1024, 768, "Boundary tracing");
    mplot::DatasetStyle ds (mplot::stylepolicy::markers);
    // Create a GraphVisual object (obtaining a unique_ptr to the object) with a spatial offset within the scene of 0,0,0
    auto gv = std::make_unique<mplot::GraphVisual<float>> (sm::vec<float>{-0.5f,-0.5f,0.0f});

    constexpr unsigned int n_points = 80;

    // Create data
    sm::rand_uniform<float> rngxy(-0.8f, 0.8f, n_points);
    sm::vvec<sm::vec<float, 2>> points(n_points);
    for (unsigned int i = 0; i < n_points; ++i) {
        points[i] = { rngxy.get(), rngxy.get() };
    }

    // Trace algorithm
    sm::vec<float, 2> cent2 = sm::algo::centroid (points);
    sm::vvec<sm::vec<float, 2>> boundary (nbp, sm::vec<float, 2>{});
    sm::vvec<sm::vec<float, 2>> centres (nbp, sm::vec<float, 2>{});

    // For tracing, make an angle-sorted set of the coordinates,, based on the angle
    // from the first two dimensions, sorted between angle 0 and 2pi
    sm::vvec<sm::vec<float, 2>> dcoords_srt (points.size());
    // Offset by centroid
    for (unsigned int i = 0; i < points.size(); ++i) {
        dcoords_srt[i] = points[i] - cent2;
    }
    // Sort by angle
    std::sort (dcoords_srt.begin(), dcoords_srt.end(), [](const sm::vec<float, 2>& a, const sm::vec<float, 2>& b){
        float aa = a.angle();
        sm::algo::zero_to_twopi (aa);
        float ab = b.angle();
        sm::algo::zero_to_twopi (ab);
        return aa < ab;
    });

    int dssz = dcoords_srt.size();
    // Can now iterate through dcoords_srt, slice by slice
    int step = dcoords_srt.size() / (nbp - 1);
    const float theta = sm::mathconst<float>::pi / nbp;
    for (int i = 0; i < dssz; i += step) {
        // This is a pie slice.
        int idx0 = (i / step)     % nbp;                 // current index
        float phi0 = idx0 * sm::mathconst<float>::two_pi / nbp;

        std::cout << "Pie slice starting at i = " << i << std::endl;
        float l = 0.0f;
        for (int j = i - step / 2; j < (i + step / 2); ++j) {
            int idx = (dssz + (j % dssz)) % dssz; // (w + (i % w)) % w;
            float ll = dcoords_srt[idx].length();
            float aa = dcoords_srt[idx].angle();
            sm::algo::zero_to_twopi (aa);
            float th = std::abs(phi0 - aa);
            ll /= std::cos (th);
            l = ll > l ? ll : l;
            // FIXME: project onto phi0 (trying above)
        }
        std::cout << "max length is " << l << std::endl;

        int idx1 = (nbp + ((i / step - 1) % nbp)) % nbp; // 'previous' (or more clockwise)

        centres[idx0] = cent2 + sm::vec<float, 2>{ l * std::cos (phi0), l * std::sin (phi0) };
        std::cout << "centres[idx0] = " << centres[idx0] << std::endl;

        // Start and end of slice
        float idx0f = static_cast<float>(idx0);
        float phi1 = (idx0f - 0.5f) * sm::mathconst<float>::two_pi / nbp;
        float phi2 = (idx0f + 0.5f) * sm::mathconst<float>::two_pi / nbp;

        std::cout << "Slice centre idx " << idx0 << " angle " << phi0 << " with bnd index "
                  << idx1 << " to " << idx0 << " from angle " << phi1 << " to " << phi2 << std::endl;

        float b1len = boundary[idx1].length();
        if (b1len) {
            b1len = (boundary[idx1] - cent2).length();
        }
        // Length at phi1/phi2 is longer than at phi0
        float hyp = l / std::cos (theta);
        std::cout << " for l = " << l << " and theta = " << theta * sm::mathconst<float>::rad2deg << " hyp = " << hyp << std::endl;
        sm::vec<float, 2> cand_vec = { hyp * std::cos (phi1), hyp * std::sin (phi1) };
        float cvlen = cand_vec.length();

        boundary[idx1] = cvlen > b1len ? (cent2 + cand_vec) : boundary[idx1];                          // prev
        boundary[idx0] = cent2 + sm::vec<float, 2>{ hyp * std::cos (phi2), hyp * std::sin (phi2) }; // curr
    }

    v.bindmodel (gv);
    gv->setlimits (-1.5, 1.5, -1.5, 1.5);

    gv->setdata (points, ds);

    sm::vvec<sm::vec<float, 2>> vv_cent2 (1);
    ds.markercolour = mplot::colour::springgreen2;
    ds.markersize *= 2;
    ds.markerstyle = mplot::markerstyle::hexagon;
    vv_cent2[0] = cent2;
    gv->setdata (vv_cent2, ds);
    ds.markersize /= 2;

    ds.linecolour = mplot::colour::crimson;
    ds.markercolour = mplot::colour::black;
    ds.markerstyle = mplot::markerstyle::circle;
    ds.showlines = true;
    gv->setdata (boundary, ds);

    ds.markercolour = mplot::colour::maroon;
    ds.markersize /= 3;
    ds.showlines = false;
    gv->setdata (centres, ds);

    gv->finalize();
    v.addVisualModel (gv);
    v.keepOpen();
}
