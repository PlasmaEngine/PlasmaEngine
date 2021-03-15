// MIT Licensed (see LICENSE.md).
#pragma once

#include "CommonStandard.hpp"
#include "PlatformStandard.hpp"

#include "SDL.h"

#if !defined(PlasmaPlatformMacOS)
#  include <GL/glew.h>
#  include <GL/gl.h>
#else
#  include <Gl/glew.h>
#  include <OpenGL/gl.h>
#endif

#ifdef PlasmaTargetOsMacOS
#  include <CoreServices/CoreServices.h>
#  include <mach/mach.h>
#  include <mach/mach_time.h>
#endif

#include <new>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
