#ifndef _voxigen_worldDepot_h_
#define _voxigen_worldDepot_h_

#include "voxigen/defines.h"
#include "voxigen/gridDescriptors.h"
#include "voxigen/regularGrid.h"

#include <vector>
#include <boost/filesystem.hpp>

std::vector<GridDescriptors> getWorlds();

template<typename _Grid>
bool createWorld(std::string worldName, _Grid &world, glm::ivec3 size)
{
    fs::path worldsDirectory("worlds");
    std::string worldDirectory=worldsDirectory.string()+worldName;

    world.create(worldDirectory, "TestApWorld", size, "EquiRectWorldGenerator");
    return true;
}

template<typename _Grid>
bool openWorld(std::string worldName, _Grid &world)
{
    fs::path worldsDirectory("worlds");

    if(!fs::exists(worldsDirectory))
        fs::create_directory(worldsDirectory);

    fs::path worldDirectory(worldsDirectory.string()+"/worldName");

    if(!fs::exists(worldDirectory))
        return false;

    world.load(worldDirectory.string());
    return true;
}

#endif //_voxigen_worldDepot_h_