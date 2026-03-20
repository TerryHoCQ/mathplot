/*
 * A mathplot native version of compound-ray's Ommatidium
 */
export module mplot.compoundray.ommatidium;

export import sm.vec;

export namespace mplot::compoundray
{
    // This is a binary-compatible equivalent to struct Ommatidium from cameras/CompoundEyeDataTypes.h in compound-ray.
    // Use reinterpret_cast<std::vector<mplot::compoundray::Ommatidium>*>(ommatidia) if your ommatidia originate inside compound ray.
    struct Ommatidium
    {
        sm::vec<float, 3> relativePosition = {};
        sm::vec<float, 3> relativeDirection = {};
        float acceptanceAngleRadians = 0.0f;
        float focalPointOffset = 0.0f;
    };
}
