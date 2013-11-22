
#include <iostream>
#include <stdexcept>

#include "Context.hpp"

#if defined(USE_OS_MESA)
  #include "src/OSMesaContext.hpp"
#else
  #if defined(SPIRE_USING_WIN)
    #include "src/WGLContext.hpp"
  #elif defined(SPIRE_USING_LINUX)
    #include "src/GLXContext.hpp"
  #elif defined(SPIRE_USING_OSX)
    #include "src/CGLContext.hpp"
    #include "src/NSContext.hpp"
  #endif
#endif

namespace CPM_GL_BATCH_CONTEXT_NS
{

//------------------------------------------------------------------------------
Context::Context()
{
}

//------------------------------------------------------------------------------
Context* Context::createBatchContext(uint32_t width, uint32_t height,
                                     uint8_t color_bits, uint8_t depth_bits,
                                     uint8_t stencil_bits, bool double_buffer,
                                     bool visible)
{
  Context* bctx;
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

}
