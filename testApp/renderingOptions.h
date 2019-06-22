#ifndef _voxigen_renderingoptions_h_
#define _voxigen_renderingoptions_h_

#include "voxigen/simpleCamera.h"

struct RenderingOptions
{
    RenderingOptions():
        resetCamera(false),
        showPlayer(false),
        move_player(true),
        show_chunks(true),
        show_regions(true),
        cursor_capture(false)
    {}

    bool resetCamera;
    bool showPlayer;
    bool move_player;
    bool show_chunks;
    bool show_regions;
    bool cursor_capture;

    voxigen::SimpleFpsCamera camera;
    voxigen::Position player;
    unsigned int playerRegion;
    glm::ivec3 playerRegionIndex;
    glm::ivec3 playerChunkIndex;
    unsigned int playerChunk;
};

#endif//_voxigen_renderingoptions_h_