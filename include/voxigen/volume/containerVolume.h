#ifndef _voxigen_containerVolume_h_
#define _voxigen_containerVolume_h

#include "voxigen/volume/regularGrid.h"
#include "voxigen/freeQueue.h"

#include "voxigen/volume/regionIndex.h"
#include "voxigen/volume/regionChunkIndex.h"

#include <memory>
#include <functional>

namespace voxigen
{

template<typename _Container>
struct LoadContainer
{
    typedef _Container Container;

    LoadContainer():lod(0), container(nullptr) {}
    LoadContainer(size_t lod, Container *container):lod(lod), container(container) {}

    size_t lod;
    Container *container;
};

enum class UpdateStatus
{
    Unknown,
    NeedMesh,
    Release
};

template<typename _Container>
struct UpdateContainer
{
    UpdateContainer():status(UpdateStatus::Unknown), container(nullptr) {}
    UpdateContainer(UpdateStatus status, _Container *container):status(status), container(container) {}

    UpdateStatus status;
    _Container *container;
};

template<typename _Grid, typename _Index, typename _ContainerInfo, typename _Container>
class ContainerVolume
{
public:
    typedef ContainerVolume<_Grid, _Index, _ContainerInfo, _Container> Type;

    typedef typename _Grid::Descriptor Descriptor;
    typedef typename _Grid::Type Grid;
    typedef typename _Grid::Region Region;
    typedef typename _Grid::Chunk Chunk;

    typedef _Index Index;
    typedef _ContainerInfo ContainerInfo;
    typedef _Container Container;
    typedef std::vector<Container> Containers;

    typedef std::function<_Container *()> GetContainer;
    typedef std::function<void(_Container *)> ReleaseContainer;

    typedef std::vector<ContainerInfo> VolumeInfo;
    typedef std::function<void(VolumeInfo &, glm::ivec3 &/*size*/, glm::ivec3 &/*center*/)> InitVolumeInfo;

    typedef LoadContainer<_Container> LoadContainer;
    typedef std::vector<LoadContainer> LoadRequests;

    typedef UpdateContainer<_Container> UpdateContainer;
    typedef std::vector<UpdateContainer> UpdateContainers;

public:
    ContainerVolume(Grid *grid, Descriptor *descriptors, InitVolumeInfo initVolumeInfo, GetContainer getContainer, ReleaseContainer releaseContainer);
    ~ContainerVolume();

    void setViewRadius(const glm::ivec3 &radius);// , LoadRequests &load, UpdateContainers &updates);
    size_t getContainerCount() { return m_containerCount; }

    void setOutlineInstance(unsigned int outlineInstanceId);

    void init(const Index &index, LoadRequests &load, UpdateContainers &updates);
    void update(const Index &index, LoadRequests &load, UpdateContainers &updates);

    ContainerInfo *getContainerInfo(const Index &index);// const Key &key);
    VolumeInfo &getVolume() { return m_volume; }

    void releaseInfo(Container *containerInfo);

    void generateUpdateOrder();

private:
    void rebuild(const Index &index, LoadRequests &load, UpdateContainers &updates);

    glm::ivec3 calcVolumeSize(const glm::ivec3 &radius);

    void getMissingContainers(LoadRequests &load);

    InitVolumeInfo initVolumeInfo;
    GetContainer getContainer;
    ReleaseContainer releaseContainer;

    void getContainers(const glm::ivec3 &start, const Index &startIndex, const glm::ivec3 &size, LoadRequests &load);
    void releaseContainers(const glm::ivec3 &start, const glm::ivec3 &size, UpdateContainers &updates);

    Grid *m_grid;
    const Descriptor *m_descriptors;

    Index m_index;

    glm::ivec3 m_radius;
    VolumeInfo m_volume;
    glm::ivec3 m_volumeSize;
    glm::ivec3 m_volumeCenterIndex;
    size_t m_containerCount;

    bool m_missingContainers;
    bool m_init;
};

}//namespace voxigen

#include "containerVolume.inl"

#endif //_voxigen_containerVolume_h