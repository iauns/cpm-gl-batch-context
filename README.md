cpm-gl-batch-context
====================

[![Build Status](https://travis-ci.org/iauns/cpm-gl-batch-context.png)](https://travis-ci.org/iauns/cpm-gl-batch-context)

Module to provide a cross platform OpenGL context for testing rendering
algorithms. An OSMesa context is provided so that you can test rendering on
platforms that do not have a video card installed such as VMs.

Usage
-----

### Context

```c++
#include <gl-batch-context/Context.h>
```

Issue a call to Context::createBatchContext to create a new offscreen
OpenGL context for your platform.

See
[documentation](http://iauns.github.io/cpm-gl-batch-context/class_c_p_m___g_l___b_a_t_c_h___c_o_n_t_e_x_t___n_s_1_1_context.html).

Options
-------

* `USE_OS_MESA` - if you set this to `TRUE`, and you are compiling on
Linux, then an OSMesa context will be created for you. This is a software
rendering context which will allow you to test your rendering algorithms on
platforms that have little or no graphics support.

OSMesa Context
--------------

If you are using the OS mesa context, be sure *not* to link against any other
OpenGL libraries. That means that there should be no reference to
`OPENGL_LIBRARIES` in any of your CMakeLists.txt files. This module follows
this convention. But it does add `OPENGL_LIBRARIES` if you do not request an
OSMesa context.

