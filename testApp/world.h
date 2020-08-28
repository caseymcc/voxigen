#ifndef _voxigen_world_h_
#define _voxigen_world_h_

#include "voxigen/defines.h"
#include "voxigen/volume/cell.h"
#include "voxigen/volume/regularGrid.h"
#include "voxigen/rendering/simpleRenderer.h"
#include "voxigen/generators/equiRectWorldGenerator.h"

static std::string defaultWorldName="TestAppWorld";
typedef voxigen::RegularGrid<voxigen::Cell, 64, 64, 16> World;
//static std::string defaultWorldName="TestAppWorld_128";
//typedef voxigen::RegularGrid<voxigen::Cell, 128, 128, 128> World;

namespace voxigen
{

//force generator instantiation
template class GeneratorTemplate<EquiRectWorldGenerator<World>, generic::io::StdFileIO>;

}//namespace voxigen

typedef voxigen::GeneratorTemplate<voxigen::EquiRectWorldGenerator<World>, generic::io::StdFileIO> WorldGeneratorTemplate;
typedef voxigen::EquiRectWorldGenerator<World> WorldGenerator;
typedef voxigen::SimpleRenderer<World> WorldRenderer;
typedef voxigen::SimpleRenderer<World>::ChunkRendererType ChunkRenderer;

#endif//_voxigen_world_h_
