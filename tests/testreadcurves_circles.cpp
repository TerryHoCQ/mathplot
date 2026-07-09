#include <iostream>
#include <stdexcept>
#include <vector>
#include <cmath>

import sm.bezcoord;
import sm.bezcurvepath;

import mplot.readcurves;

int main()
{
    int rtn = -1;

    try {
        mplot::ReadCurves r("../../tests/whiskerbarrels_withcentres.svg");
        //r.save (0.001f);
        sm::bezcurvepath<float> bcp = r.getCorticalPath();
        bcp.compute_points (0.01f);
        std::vector<sm::bezcoord<float>> pts = bcp.get_points();
        auto i = pts.begin();
        while (i != pts.end()) {
            std::cout << *i << std::endl;
            ++i;
        }

        std::cout.precision(12);
        std::cout << "pts[23] =  " << pts[23].t()
             << " " << pts[23].x()
             << " " << pts[23].y()
             << std::endl;
        if ((std::abs(pts[23].t() - 0.110523112118) < 0.000001f)
            && (std::abs(pts[23].x() - 0.74002712965) < 0.000001f)
            && (std::abs(pts[23].y() - 0.393309623003) < 0.000001f)) {
            std::cout << "rtn IS 0" << std::endl;
            rtn = 0;
        } else {
            std::cout << "rtn not 0" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception reading trial.svg: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
