#include "linterpolation.h"
#include "Globals.h"
#include "dot.h"

#include <windows.h>


linterpolation::linterpolation(){}

void linterpolation::SetPos(dot& d1, dot& d2, double k)
{
	int segments;
	if (k >= 1) 
	{
		segments = 1;
	}
	else if (k < 1) 
	{
		segments = 1 / k;
		k = 1;
	}
	Sleep((d2.time-d1.time)/k);
	for (int num = 1; num < segments ; num+= 1)
	{
		SetCursorPos((num * d2.x + (segments   - num) * d1.x) / (segments ),
			(num * d2.y + (segments  - num) * d1.y) / (segments));
		Sleep((d2.time - d1.time)/k );
	}
	SetCursorPos(d2.x, d2.y);
}