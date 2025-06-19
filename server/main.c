#define PAINTERENGINE_STANDALONE
#include "PX_Game.h"

px_int main()
{
	px_dword lastupdate_time = 0;
	PX_Games_Initialize();
	lastupdate_time = PX_TimeGetTime();
	while (1)
	{
		px_dword current_time = PX_TimeGetTime();
		px_dword elapsed = current_time - lastupdate_time;
		lastupdate_time = current_time;
		PX_Games_Update(elapsed);
	}
}