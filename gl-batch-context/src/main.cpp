/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
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

#include <cstdlib>
#include <iostream>

#include "gtest/gtest.h"

#include "spire/src/Common.h"
#include "spire/src/Log.h"

#include "BatchContext.h"
#include "../tests/namespaces.h"
#include "../tests/GlobalTestEnvironment.h"
#include "../tests/namespaces.h"

// Defining cimg_display to 0 ensures CImg doesn't try to include OS specific
// windowing header files.
#define cimg_display 0
#include "CImg.h"
using namespace cimg_library;

// Lots of yucky code in the test-environment.
class TestEnvironment : public GlobalTestEnvironment
{
public:
  TestEnvironment(uint32_t width, uint32_t height, 
                  int32_t colorBits, int32_t depthBits, int32_t stencilBits,
                  bool doubleBuffer, bool visible) :
      mWidth(width),
      mHeight(height)
  {
    mContext = createContext(width, height, colorBits, depthBits, stencilBits,
                             doubleBuffer, visible);

    // Pre-allocate an image of appropriate size.
    try
    {
      mRawImage.resize(width*height*4);  // 4 is the bit depth - RGBA.
    }
    catch (...)
    {
      mRawImage.clear();
    }
    if ( mRawImage.empty() )
      throw std::runtime_error("Unable to allocate space for image.");

    mContext->makeCurrent();

    // Build an FBO (the only thing we can render to when rendering offscreen)
    /// \todo Migrate this code elsewhere when we add tuvok / a compasited
    ///       renderer as it will no longer work.
    GL(glGenFramebuffersEXT(1, &mGLFrameBuffer));
    GL(glGenRenderbuffersEXT(1, &mGLDepthBuffer));
    GL(glGenTextures(1, &mGLColorTexture));

    GL(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mGLFrameBuffer));

    GL(glBindTexture(GL_TEXTURE_2D, mGLColorTexture));
    GL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(mWidth),
                    static_cast<GLsizei>(mHeight), 0,
                    GL_RGBA, GL_INT, NULL));
    GL(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                              GL_TEXTURE_2D, mGLColorTexture, 0));


    // initialize depth renderbuffer
    GL(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mGLDepthBuffer));
    GL(glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24,
                                static_cast<GLsizei>(mWidth), static_cast<GLsizei>(mHeight)));
    GL(glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                    GL_RENDERBUFFER_EXT, mGLDepthBuffer));
  }


  /// Creates a context if one hasn't already been created and returns it.
  /// Otherwise, it returns the currently active context.
  std::shared_ptr<spire::Context> getContext() const override
  {
    GL(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mGLFrameBuffer));
    return std::dynamic_pointer_cast<spire::Context>(mContext);
  }

  void writeFBO(const std::string& file) override
  {
    // This function should be called from test code. So we are safe writing
    // test assertions in this code.
    GLint viewport[4];
    GL(glGetIntegerv(GL_VIEWPORT, viewport));
    ASSERT_EQ(0, viewport[0]);
    ASSERT_EQ(0, viewport[1]);
    ASSERT_EQ(mWidth, viewport[2]);
    ASSERT_EQ(mHeight, viewport[3]);

    GL(glPixelStorei(GL_PACK_ALIGNMENT, 1));
    GL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    GL(glReadBuffer(GL_COLOR_ATTACHMENT0));     // Reading straight from the FBO
    GL(glReadPixels(0,0,static_cast<GLsizei>(mWidth),static_cast<GLsizei>(mHeight),
                    GL_RGBA,GL_UNSIGNED_BYTE,&mRawImage[0]));

    // We are using shared memory for the CImg class so it doesn't have to allocate its own.
    // It would be better to std::move vector into CImg though.
    // \todo image flipped?
    CImg<uint8_t> img(&mRawImage[0], mWidth, mHeight, 1, 4, false);

    // 'De-interleave' the image data - extremelly inneficient seeing as we are
    // going to writing the output data in interleaved format anyways.
    uint8_t* glData     = &mRawImage[0];
    uint8_t* cimgData   = img.data();

    int channelSize  = static_cast<int>(mWidth * mHeight);
    size_t imgSize   = static_cast<size_t>(channelSize) * 4;

    size_t  i        = 0;
    int     pixIndex = 0;
    for (i = 0, pixIndex = 0; i < imgSize; i += 4, ++pixIndex)
    {
      // Origin is at bottom left for OpenGL. So while we are de-interleaving,
      // we may as well flip the image. This is what the subtraction is all
      // about, and the multiplication start at 1 instead of 0.
      int row = static_cast<int>(
          (static_cast<uint32_t>(pixIndex) / mWidth) * mWidth); // Expect this to be floored.
      int offset = pixIndex % static_cast<int>(mWidth);         // Offset from the beginning of the row.
      int rowoff = row + offset;              // Combined row / offset.
      cimgData[(channelSize*1-1) - rowoff] = glData[i + 0];
      cimgData[(channelSize*2-1) - rowoff] = glData[i + 1]; 
      cimgData[(channelSize*3-1) - rowoff] = glData[i + 2];
      cimgData[(channelSize*4-1) - rowoff] = glData[i + 3];
    }
    img.save(file.c_str());
  }

  uint32_t getScreenWidth() const override  { return mWidth; }
  uint32_t getScreenHeight() const override { return mHeight; }

  /// Overrides from gtest
  /// @{
  void SetUp() override     {}
  void TearDown() override  {}
  /// @}

private:
  static std::shared_ptr<BatchContext> createContext(
      uint32_t width, uint32_t height,
      int32_t colorBits, int32_t depthBits, int32_t stencilBits,
      bool doubleBuffer, bool visible)
  {
    std::shared_ptr<BatchContext> ctx(
        BatchContext::Create(width,height,
                                    static_cast<uint8_t>(colorBits),
                                    static_cast<uint8_t>(depthBits),
                                    static_cast<uint8_t>(stencilBits),
                                    doubleBuffer,visible));
    if (ctx->isValid() == false)
      throw std::runtime_error("Invalid context generated.");
    ctx->makeCurrent();

    // Setup the viewport correctly.
    GL(glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height)));

    return ctx;
  }

  std::shared_ptr<BatchContext> mContext;

  /// Pre-allocated rawImage of what we will pull from OpenGL.
  std::vector<uint8_t> mRawImage;
  uint32_t mWidth;
  uint32_t mHeight;

  GLuint mGLFrameBuffer;
  GLuint mGLColorTexture;

  GLuint mGLDepthBuffer;
};


int main(int argc, char** argv)
{
  // Create the global testing environment.
  //::testing::Environment* const testEnv = 
      ::testing::AddGlobalTestEnvironment(
          new TestEnvironment(600, 600, 32, 24, 8, true, false));

  // Run the tests.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

