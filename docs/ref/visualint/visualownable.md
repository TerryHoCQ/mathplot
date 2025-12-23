---
title: mplot::VisualOwnable
parent: Internal classes
grand_parent: Reference
permalink: /ref/visualint/visualownable
layout: page
nav_order: 9
---

`VisualOwnable` and `VisualOwnableMX` derive from `VisualBase` and add
functionality that requires OpenGL function calls, but without adding
any windowing system specific functionality.

This class is a mathplot-internal class and there is typically no
access of its methods in mathplot client code.

However, if you want to incorporate mathplot graphics into another
windowing system, this class provides the drawing functionality. For examples, see [mplot/qt/viswidget.h](https://github.com/sebsjames/mathplot/blob/main/mplot/qt/viswidget.h) and [mplot/qt/viswidget_mx.h](https://github.com/sebsjames/mathplot/blob/main/mplot/qt/viswidget_mx.h) for Qt and [mplot/wx/viswx.h](https://github.com/sebsjames/mathplot/blob/main/mplot/wx/viswx.h) for wxWidgets.
