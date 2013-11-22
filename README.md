cpm-gl-batch-context
====================

[![Build Status](https://travis-ci.org/iauns/cpm-gl-batch-context.png)](https://travis-ci.org/iauns/cpm-gl-batch-context)

Module to provide a cross platform OpenGL context for testing rendering
algorithms. An OSMesa context is provided so that you can test rendering on
platforms that do not have a video card installed such as VMs.

Use
---

```c++
#include "gl-batch-context/Context.h"
```

Issue a call to Context::createBatchContext to create a new offscreen
OpenGL context for your platform.

Options
-------

* `USE_OS_MESA` - if you set this to `TRUE`, and you are compiling on
Linux, then an OSMesa context will be created for you. This is a software
rendering context which will allow you to test your rendering algorithms on
platforms that have little or no graphics support.

