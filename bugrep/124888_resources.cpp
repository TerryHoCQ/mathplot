module;
#include <functional>
export module resources;

export struct Resources
{
private:
    Resources(){}
    ~Resources();
public:
    void f() { if (f_impl) { f_impl(); } }
    std::function<void()> f_impl;

    static auto& i()
    {
        static Resources instance;
        return instance;
    }
};
