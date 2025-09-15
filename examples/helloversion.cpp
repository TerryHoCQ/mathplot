#include <mplot/version.h>
#include <mplot/Visual.h>
#include <string>

int main()
{
    mplot::Visual<mplot::gl::version_3_1_es> v(600, 400, "Hello Version!");
    v.addLabel ("Hello World!", {0,-0.1,0}, mplot::TextFeatures(0.1f, 48));
    v.addLabel (std::string("mathplot version ") + mplot::version_string(), {0,-0.2,0}, mplot::TextFeatures(0.04f, 32));
    v.keepOpen();
    return 0;
}
