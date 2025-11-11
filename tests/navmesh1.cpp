#include <iostream>
#include <sm/hdfdata>
#include <mplot/NavMesh.h>

int main()
{
    int rtn = 0;

    mplot::NavMeshMovementData md;
    md.mv_camframe = {2,3,4};
    md.cam_to_scene = { 2,3,4,5, 7,6,5,4, 9,8,7,6, 4,5,6,7 };
    md.model_to_scene = {}; // identity
    md.ti0 = { 40,500,60,70 };
    md.hoverheight = 23.2f;

    {
        sm::hdfdata hd ("./navmesh1.hd5", std::ios::out | std::ios::trunc);
        md.save (hd, 2);
        md.save (hd, 4);
    }

    mplot::NavMeshMovementData mdr;
    {
        sm::hdfdata hd ("./navmesh1.hd5", std::ios::in);
        mdr.load (hd, 2);
    }

    std::cout << "mdr.cam_to_scene: " << mdr.cam_to_scene << std::endl;
    if (mdr != md) { --rtn; }

    if (rtn) { std::cout << "Test failed\n"; }

    return rtn;
}
