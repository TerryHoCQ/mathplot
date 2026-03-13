// B.cppm
export module B;

export template <int tp = 1>
struct B
{
    B() {}
    int f() { return tp; }
    virtual void vf ([[maybe_unused]] int b) {}
};
