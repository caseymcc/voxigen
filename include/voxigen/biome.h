#ifndef _voxigen_biome_h_
#define _voxigen_biome_h_

#include <string>

namespace voxigen
{

struct BiomeDescriptors
{
    std::string m_name;

    float m_percipitation;
    float m_temperature;
    float m_temperatureFlux;

    float m_heightMinimum;
    float m_heightMaximum;
};

class Biome
{
public:
    Biome(unsigned int type) {};

private:
    BiomeDescriptors m_descriptors;
};

}//namesapce voxigen

#endif //_voxigen_biome_h_
