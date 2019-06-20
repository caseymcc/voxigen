#ifndef _voxigen_world_h_
#define _voxigen_world_h_

#include "voxigen/defines.h"
#include "voxigen/cell.h"
#include "voxigen/regularGrid.h"
#include "voxigen/simpleRenderer.h"
#include "voxigen/equiRectWorldGenerator.h"

typedef voxigen::RegularGrid<voxigen::Cell, 64, 64, 16> World;
namespace voxigen
{
//force generator instantiation
template class GeneratorTemplate<EquiRectWorldGenerator<World>>;
}

typedef voxigen::GeneratorTemplate<voxigen::EquiRectWorldGenerator<World>> WorldGeneratorTemplate;
typedef voxigen::EquiRectWorldGenerator<World> WorldGenerator;
typedef voxigen::SimpleRenderer<World> WorldRenderer;
typedef voxigen::SimpleRenderer<World>::ChunkRendererType ChunkRenderer;

#endif//_voxigen_world_h_
