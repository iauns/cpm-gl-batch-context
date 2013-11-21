/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2010 Scientific Computing and Imaging Institute,
   University of Utah.


   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

// Shamelessly ripped from ImageVis3D's batch renderer.

#include <iostream>
#include <stdexcept>

#include "spire/src/Common.h"

#include "BatchContext.h"

#if defined(USE_OS_MESA)
  #include "OSMesaContext.h"
#else
  #if defined(SPIRE_USING_WIN)
    #include "WGLContext.h"
  #elif defined(SPIRE_USING_LINUX)
    #include "GLXContext.h"
  #elif defined(SPIRE_USING_OSX)
    #include "CGLContext.h"
    #include "NSContext.h"
  #endif
#endif

BatchContext::BatchContext()
{
}

BatchContext* BatchContext::Create(uint32_t width, uint32_t height,
                                   uint8_t color_bits, uint8_t depth_bits,
                                   uint8_t stencil_bits, bool double_buffer,
                                   bool visible)
{
  BatchContext* bctx;
#if defined(USE_OS_MESA)
  bctx = new OSMesaBatchContext(width, height, color_bits, depth_bits, stencil_bits,
                           double_buffer, visible);
#else
  #ifdef SPIRE_USING_WIN
  bctx = new WGLContext(width, height, color_bits, depth_bits, stencil_bits,
                       double_buffer, visible);
  #elif defined(SPIRE_USING_OSX) && defined(USE_CGL)
  bctx = new CGLContext(width, height, color_bits, depth_bits, stencil_bits,
                       double_buffer, visible);
  #elif defined(SPIRE_USING_OSX)
  bctx = new NSContext(width, height, color_bits, depth_bits, stencil_bits,
                      double_buffer, visible);
  #else
  bctx = new GLXBatchContext(width, height, color_bits, depth_bits, stencil_bits,
                             double_buffer, visible);
  #endif
#endif
  bctx->makeCurrent();

#ifdef SPIRE_USING_WIN
  GLenum err = glewInit();
  if (GLEW_OK != err)
  {
    Log::error() << "GLEW init failed!" << std::endl;
    throw GLError("GLEW failed to initialize.");
  }
#endif

  return bctx;
}

