// Graph the logistic function
#include <sstream>
#include <sm/vvec>
#include <mplot/Visual.h>
#include <mplot/GraphVisual.h>
#include <sm/config>

int main()
{
    // We'll use mathplot's awesome unicode chars
    namespace uc = mplot::unicode;
    // Set up a mplot::Visual 'scene environment'.
    mplot::Visual v(1024, 768, "Logistic functions");
    v.addLabel ("Change logistic function parameters in ../examples/graph_logist2.json (live updates)", sm::vec<float>({0,0,0}));
    v.setSceneTrans (sm::vec<float,3>({-0.732852f, 0.0348977f, -5.0f}));
    // Create a GraphVisual object (obtaining a unique_ptr to the object) with a spatial offset within the scene of 0,0,0
    auto gv = std::make_unique<mplot::GraphVisual<double>> (sm::vec<float>{-0.5f,-0.5f,0.0f});
    v.bindmodel (gv);
    // Params are read from a JSON file
    double x0=0, k=0, g1x0=0, g1x1=0;
    {
        sm::config conf ("../examples/graph_logist2.json");
        k = conf.get<double> ("k", 10.0);
        x0 = conf.get<double> ("x0", 4.0);
        g1x0 = conf.get<double> ("g1x0", -10.0);
        g1x1 = conf.get<double> ("g1x1", 10.0);
    }
    // Data for the x axis. A vvec is like std::vector, but with built-in maths methods
    sm::vvec<double> x;
    // This works like numpy's linspace() (the 3 args are "start", "end" and "num"):
    x.linspace (g1x0, g1x1, 100);
    // Logistic functions. Args are parameters to the function are (xoffset, alpha)
    std::stringstream lftag;
    lftag << "k=" << k << ", x" << uc::toUtf8(uc::subs0) << "=" << x0; // A dataset tag
    // vvec::logistic() returns a new vvec with the logistic function-transformed values:
    gv->setdata (x, x.logistic (k, x0), lftag.str());
    gv->ylabel = "f(x)";
    // finalize() makes the GraphVisual compute the vertices of the OpenGL model
    gv->finalize();
    // Add the GraphVisual OpenGL model to the Visual scene, transferring ownership of the
    // unique_ptr (and returning a regular pointer)
    mplot::GraphVisual<double>* gvptr = v.addVisualModel (gv);

    auto gv2 = std::make_unique<mplot::GraphVisual<double>> (sm::vec<float>{1.0f,-0.5f,0.0f});
    v.bindmodel (gv2);
    sm::vvec<double> x2;
    x2.linspace (0, 1, 100);
    gv2->setlimits (0,1,0,1);
    gv2->setdata (x2, x2.logistic(k, x0), lftag.str());
    gv2->ylabel = "f(x)";

    gv2->finalize();
    mplot::GraphVisual<double>* gv2ptr = v.addVisualModel (gv2);

    bool shown_error = false; // A flag to avoid showing an error 60 times a second

    // Render the scene on the screen until user quits with 'Ctrl-q'
    while (!v.readyToFinish()) {
        v.render();
        v.waitevents (0.018);
        // Update from config file with every render so that changes in the file are immediately reflected in the graph.
        try {
            sm::config conf ("../examples/graph_logist2.json");
            k = conf.get<double> ("k", 10.0);
            x0 = conf.get<double> ("x0", 4.0);

            std::stringstream newtag;
            newtag << "k=" << k << ", x" << uc::toUtf8(uc::subs0) << "=" << x0;

            // Remove label and existing legends with VisualModel::clearTexts()
            gvptr->clearTexts();
            // Update the graphs via their non-owning pointers
            gvptr->update (x, x.logistic(k, x0), newtag.str(), 0);
            // Show the general eqn by adding a label below the first graph
            std::stringstream eqngen;
            eqngen << "f(x) = 1 / [1 + exp (-k(x - x"<< uc::toUtf8(uc::subs0) << ")]";
            gvptr->addLabel (eqngen.str(), sm::vec<float>({0.1f, -0.3f, 0.0f}), mplot::TextFeatures(0.05f));

            gv2ptr->clearTexts();
            // Update legend
            gv2ptr->update (x2, x2.logistic(k, x0), newtag.str(), 0);
            // Add a new eqn label
            std::stringstream ktxt;
            if (k != 1.0) { ktxt << k; }
            std::stringstream brtxt;
            std::stringstream ostxt;
            if (x0 != 0.0) {
                brtxt << "(";
                if (x0 > 0.0) {
                    ostxt << " - " << x0 <<")";
                } else {
                    ostxt << " + " << -x0 <<")";
                }
            }
            // Show the specific equation on the second graph
            std::stringstream eqn;
            eqn << "f(x) = 1 / [1 + exp (-"<< ktxt.str() << brtxt.str() << "x" << ostxt.str() << ")]";
            gv2ptr->addLabel (eqn.str(), sm::vec<float>{0.1f, -0.3f, 0.0f}, mplot::TextFeatures(0.05f));

            if (shown_error) {
                std::cout << "JSON parsed successfully\n";
                shown_error = false; // reset
            }

        } catch (const std::exception& e) {
            if (!shown_error) {
                std::cerr << "JSON file could not be parsed with error: " << e.what() << std::endl;
                shown_error = true;
            }
        }
    }

    return 0;
}
