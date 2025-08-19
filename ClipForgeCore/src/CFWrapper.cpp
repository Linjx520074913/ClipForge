#define CF_CORE_DLL

#include "CFWrapper.h"
#include "CFPlayer.h"

CFPlayer player;

void cf_player_open(const char* file_path)
{
    player.open(file_path);
}

void cv_player_close()
{
    player.close();
}

CFFrame* cf_player_get_frame_at(int64_t timestamp)
{
    return player.get_frame(timestamp).get();
}