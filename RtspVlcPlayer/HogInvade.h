#pragma once
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "opencv2/opencv.hpp"
class HogInvade
{
public:
	HogInvade();
	~HogInvade();
public:
	void HogImage(IplImage* img);
};

