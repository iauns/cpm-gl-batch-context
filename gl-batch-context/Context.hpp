
/// \author James Hughes
/// \date   November 2013

#ifndef IAUNS_GL_BATCH_CONTEXT_BATCH_HPP
#define IAUNS_GL_BATCH_CONTEXT_BATCH_HPP

namespace CPM_GL_BATCH_CONTEXT_NS
{

/// Base context class.
class Context
{
public:
  Context();
  virtual ~Context();

  virtual bool isValid() const=0;

  /// Creates a batch context not associated with any windows.
  static Context* createBatchContext(
      uint32_t width, uint32_t height, uint8_t color_bits, uint8_t depth_bits,
      uint8_t stencil_bits, bool double_buffer, bool visible);

  //============================================================================
  // Mandatory OpenGL-context related functions
  //============================================================================

  /// Make the context current on the active thread.
  virtual void makeCurrent()    = 0;

  /// Swap the front and back buffers.
  virtual void swapBuffers()    = 0;
};

} // namespace CPM_GL_BATCH_CONTEXT_NS

#endif 
