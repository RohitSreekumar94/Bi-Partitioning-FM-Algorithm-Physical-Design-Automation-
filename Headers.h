#ifndef HEADER_H
#define HEADER_H


/***************************************************************
* System Header Files
****************************************************************/

#include<iostream>
#include <sys/timeb.h>
#include<stdarg.h>
#include<time.h>
#include "math.h"
#include "string.h"
#include<stdio.h>
#include<stdlib.h>
#include<map>
#include<list>
#include<iterator>
#include<vector>
#include<algorithm>
#include<cmath>

using namespace std;

#ifdef WIN32
#include<chrono>
using namespace std::chrono;
#endif

/***************************************************************
* User Control Definitions
****************************************************************/
#define	NUM_PASSES	5
#define AREA_RATIO  0.45
#define DEBUG_LOG 1

/***************************************************************
* User Definitions
****************************************************************/

#define DESIGN_AUX_FILE "design.aux"

/***************************************************************
* Enumerations
****************************************************************/
typedef enum
{
	FM_ERROR	= -1,
	FM_SUCCESS	= 1,

}FM_RESULT;



#endif
