#pragma once

#include "CommonStandard.hpp"
#include "SupportStandard.hpp"
#include "RendererGLStandard.hpp"

// Include glew before OpenGl
#include <GL/glew.h>

#if defined(PlasmaPlatformWindows)
	#include <GL/wglew.h>
#else
	#include "SDL.h"
#endif


// Include OpenGl
#if defined(PlasmaPlatformMacOS)
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif


#include "OpenglRenderer.hpp"

