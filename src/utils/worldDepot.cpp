#include "voxigen/worldDepot.h"

std::vector<GridDescriptors> getWorlds()
{
    fs::path worldsDirectory("worlds");
    std::vector<GridDescriptors> descriptors;

    if(!fs::exists(worldsDirectory))
        fs::create_directory(worldsDirectory);

    std::vector<fs::directory_entry> worldDirectories;

    for(auto &entry:fs::directory_iterator(worldsDirectory))
    {
        fs::path directory=worldsDirectory+entry;
        std::string configFile=directory.string()+"/gridConfig.json";
        GridDescriptors descriptor;

        descriptor.load(configFile);
        descriptors.push_back(descriptor);
    }

    return descriptors;
}

    
