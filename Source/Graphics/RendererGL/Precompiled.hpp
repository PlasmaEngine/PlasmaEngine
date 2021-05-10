#pragma once

#include "CommonStandard.hpp"
#include "SupportStandard.hpp"
#include "RendererGLStandard.hpp"

// Include glew before OpenGl
#include <GL/glew.h>

#if defined(PLASMA_PLATFORM_WINDOWS)
	#include <GL/wglew.h>
#else
	#include "SDL.h"
#endif


// Include OpenGl
#if defined(PLASMA_PLATFORM_OSX)
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif


#include "OpenglRenderer.hpp"

