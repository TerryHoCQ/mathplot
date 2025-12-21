---
title: Visualization API
parent: Reference
permalink: /ref/visual/
nav_order: 2
layout: page
has_children: true
---
# Visualization API reference

This section contains reference material for the classes that you'll
make use of to build a mathplot visualization.

mathplot uses OpenGL to draw graphics and text.
In a way it's like a game engine for data visualization because it provides a 3D world into which models are drawn and you can move the scene around, viewing the models from different angles.
Unlike a game engine, mathplot draws simple models with coloured triangles, which are generated at runtime. The scene is provided by [`mplot::Visual`](https://github.com/sebsjames/mathplot/blob/main/mplot/Visual.h).

Within your scene, you create one or more instances of a [`mplot::VisualModel`](https://github.com/sebsjames/mathplot/blob/main/mplot/VisualModel.h). Each `VisualModel` is an object within the scene. You can use [pre-defined VisualModels](/mathplot/ref/visualmodels/) such as graphs and surface plots, or you can [derive your own using graphics primitives](https://github.com/sebsjames/mathplot/blob/main/examples/ring.cpp#L13).
