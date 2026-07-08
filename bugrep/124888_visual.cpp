module;

export module visual;

import resources;
import specific;

export struct Visual
{
    Visual() { Resources::i().f_impl = Specific::f; }
    void call_f() { Resources::i().f(); }
};
