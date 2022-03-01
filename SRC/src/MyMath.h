#pragma once

#ifndef MATHINCLUDES_H
#define MATHINCLUDES_H

#include <cmath>	/*sin and cos*/
#include <ctime>
#include <stdlib.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/integer.hpp>
#include <glm/gtx/orthonormalize.hpp>

// Do not change
static const float EPSILON = 0.0001f;

// Macro definitions
#define isZero(x) ((x < EPSILON) && (x > -EPSILON))
#define isEqual(x,y) (((x >= y) ? (x-y) : (y-x)) < EPSILON)

#ifndef PI
#define	PI		3.1415926535897932384626433832795f
#endif


#define	HALF_PI	(PI * 0.5f)
#define	TWO_PI	(PI * 2.0f)

#endif