
#include <cstdlib>
#include <iostream>

#include <gl-batch-context/Context.hpp>
//#include <gl-platform/GLPlatform.hpp>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glx.h>

#define GL(stmt) do {stmt;} while (0)
#define GL_CHECK()


int main(int /*argc*/, char** /*argv*/)
{
  int width = 600;
  int height = 600;
  CPM_GL_CONTEXT_NS::Context* glContext = 
      CPM_GL_CONTEXT_NS::Context::createBatchContext(
          width, height, 32, 24, 8, true, true);

  // Add a few quick OpenGL calls here to verify the context is working.
  GL(glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height)));

  GLuint mGLFrameBuffer;
  GLuint mGLColorTexture;
  GLuint mGLDepthBuffer;

  // Build an FBO (the only thing we can render to when rendering offscreen)
  /// \todo Migrate this code elsewhere when we add tuvok / a compasited
  ///       renderer as it will no longer work.
  GL(glGenFramebuffersEXT(1, &mGLFrameBuffer));
  GL(glGenRenderbuffersEXT(1, &mGLDepthBuffer));
  GL(glGenTextures(1, &mGLColorTexture));

  GL(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mGLFrameBuffer));

  GL(glBindTexture(GL_TEXTURE_2D, mGLColorTexture));
  GL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(width),
                  static_cast<GLsizei>(height), 0,
                  GL_RGBA, GL_INT, NULL));
  GL(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                               GL_TEXTURE_2D, mGLColorTexture, 0));


  // initialize depth renderbuffer
  GL(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mGLDepthBuffer));
  GL(glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24,
                              static_cast<GLsizei>(width), static_cast<GLsizei>(height)));
  GL(glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                  GL_RENDERBUFFER_EXT, mGLDepthBuffer));

  std::cout << "Frame buffer index: " << mGLFrameBuffer << std::endl;
  std::cout << "Depth buffer index: " << mGLDepthBuffer << std::endl;
  std::cout << "Color texture index: " << mGLColorTexture << std::endl;
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    std::cerr << " *** Unable to generate a complete frame buffer! *** " << std::endl;
  }

  // Now that we are finished, delete the context.
  delete glContext;
  return 0;
}

