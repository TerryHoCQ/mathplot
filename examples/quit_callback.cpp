// Shows how to use the external quit callback.
#include <memory>
#include <iostream>
import mplot.visual;
import mplot.graphvisual;

void extra_quit_stuff()
{
    std::cout << "User signalled quit, so do any additional shutdown I need to do now!\n";
}

static constexpr bool lambda_quit_callback = true;

int main()
{
    // Set up a mplot::Visual 'scene environment'.
    mplot::Visual v(1024, 768, "Made with mplot::GraphVisual");
    // Create a GraphVisual object (obtaining a unique_ptr to the object) with a spatial offset within the scene of 0,0,0

    // Assign our extra_quit_stuff function to the external quit callback
    if constexpr (lambda_quit_callback == true) {
        // You could do this with a lambda function...
        v.external_quit_callback = []{ std::cout << "Additional shutdown...\n";};
    } else {
        // ...or assign the address of a function
        v.external_quit_callback = &extra_quit_stuff;
    }

    auto gv = std::make_unique<mplot::GraphVisual<double>> (sm::vec<float>({0,0,0}));
    // This mandatory line of boilerplate code sets the parent pointer in GraphVisual and binds some functions
    gv->set_parent (v.get_id());
    // Data for the x axis. A vvec is like std::vector, but with built-in maths methods
    sm::vvec<double> x;
    // This works like numpy's linspace() (the 3 args are "start", "end" and "num"):
    x.linspace (-0.5, 0.8, 14);
    // Set a graph up of y = x^3
    gv->setdata (x, x.pow(3));
    // finalize() makes the GraphVisual compute the vertices of the OpenGL model
    gv->finalize();
    // Add the GraphVisual OpenGL model to the Visual scene, transferring ownership of the unique_ptr
    v.addVisualModel (gv);
    // Render the scene on the screen until user quits with 'Ctrl-q'
    v.keepOpen();
    // Because v owns the unique_ptr to the GraphVisual, its memory will be deallocated when v goes out of scope.
    return 0;
}
