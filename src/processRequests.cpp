#include "voxigen/processRequests.h"

namespace voxigen
{

namespace process
{

 glm::ivec3 Request::regionSize;
 glm::ivec3 Request::chunkSize;

 glm::ivec3 Compare::currentRegion;
 glm::ivec3 Compare::currentChunk;

}//namespace process

}//namespace voxigen
