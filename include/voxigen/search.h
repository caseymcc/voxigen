#ifndef _voxigen_chunkSearch_h_
#define _voxigen_chunkSearch_h_

#include "voxigen/voxigen_export.h"

namespace voxigen
{

struct SearchSettings
{
    float radius;
    float lodDistance;
    glm::ivec3 regionIndex;
    glm::ivec3 chunkIndex;
};

template<typename _Grid, typename _AllocateRenderer, typename _ChunkRendererContainer, typename _ChunkRenderer>
bool searchRenderers(_Grid *grid, const SearchSettings& settings, _AllocateRenderer &&allocRenderer,
    _ChunkRendererContainer &currentRenderers, std::vector<_ChunkRenderer *> &addRenderers, std::vector<_ChunkRenderer *> &updateRenderers, std::vector<_ChunkRenderer *> &removeRenderers)
{
    typename _Grid::DescriptorType &descriptors=grid->getDescriptors();

//    glm::ivec3 playerRegionIndex=grid->getRegionIndex(m_camera->getRegionHash());
//    glm::ivec3 playerChunkIndex=grid->getChunkIndex(m_camera->getPosition());

    typedef std::unordered_map<Key::Type, bool> AddMap;
    AddMap addChunk;

    RegionHash playerRegionHash=descriptors.regionHash(settings.regionIndex);
    ChunkHash playerChunkHash=descriptors.chunkHash(settings.chunkIndex);
    Key playerChunkKey(playerRegionHash, playerChunkHash);

    //make sure player chunk added
    addChunk.insert(AddMap::value_type(playerChunkKey.hash, true));

    for(auto chunkIter=currentRenderers.begin(); chunkIter!=currentRenderers.end(); )
    {
        auto *chunkRenderer=chunkIter->second;
        Key key(chunkRenderer->getRegionHash(), chunkRenderer->getChunkHash());

        float chunkDistance=descriptors.distance(settings.regionIndex, settings.chunkIndex, chunkRenderer->getRegionIndex(), chunkRenderer->getChunkIndex());

        //chunk outside of range so invalidate
        if(chunkDistance>settings.radius)
        {
            addChunk[key.hash]=false;
            removeRenderers.push_back(chunkRenderer);
//            chunkIter=currentRenderers.erase(chunkIter);
            ++chunkIter;
        }
        else
        {
            size_t lod=floor(chunkDistance/settings.lodDistance);

            if(chunkRenderer->getLod()!=lod)
            {
                chunkRenderer->setLod(lod);
                updateRenderers.push_back(chunkRenderer);
            }
            addChunk[key.hash]=false;

            //add neighbors chunks
            for(int z=-1; z<=1; ++z)
            {
                for(int y=-1; y<=1; ++y)
                {
                    for(int x=-1; x<=1; ++x)
                    {
                        glm::ivec3 neighborRegion=chunkRenderer->getRegionIndex();
                        glm::ivec3 neighborChunk=chunkRenderer->getChunkIndex()+glm::ivec3(x, y, z);

                        descriptors.adjustRegion(neighborRegion, neighborChunk);
                        float chunkDistance=descriptors.distance(settings.regionIndex, settings.chunkIndex, neighborRegion, neighborChunk);

                        if(chunkDistance<settings.radius)
                        {
                            RegionHash regionHash=descriptors.regionHash(neighborRegion);
                            ChunkHash chunkHash=descriptors.chunkHash(neighborChunk);
                            Key key(regionHash, chunkHash);

                            if(addChunk.find(key.hash)==addChunk.end())
                                addChunk.insert(AddMap::value_type(key.hash, true));
                        }
                    }
                }
            }
            ++chunkIter;
        }
    }

    size_t addedRenderers=0;
    for(auto &iter:addChunk)
    {
        if(iter.second)
        {
            _ChunkRenderer *chunkRenderer=allocRenderer();

            if(chunkRenderer==nullptr)
                continue;

            Key key(iter.first);
            _Grid::SharedChunkHandle chunkHandle=grid->getChunk(key.regionHash, key.chunkHash);

            chunkRenderer->setChunk(chunkHandle);

            float chunkDistance=descriptors.distance(settings.regionIndex, settings.chunkIndex, chunkRenderer->getRegionIndex(), chunkRenderer->getChunkIndex());
            float lod=floor(chunkDistance/settings.lodDistance);

            chunkRenderer->setLod(lod);

            currentRenderers.insert(_ChunkRendererContainer::value_type(key.hash, chunkRenderer));
            addRenderers.push_back(chunkRenderer);
            addedRenderers++;
        }
    }

    if(addedRenderers > 0)
        return true;

    return false;
}

//template<typename _Grid>
//bool SimpleRenderer<_Grid>::updateChunks()
//{
//    _Grid::DescriptorType &descriptors=m_grid->getDescriptors();
//
//    glm::ivec3 playerRegionIndex=m_grid->getRegionIndex(m_camera->getRegionHash());
//    glm::ivec3 playerChunkIndex=m_grid->getChunkIndex(m_camera->getPosition());
//
//    typedef std::unordered_map<Key::Type, bool> AddMap;
//    AddMap addChunk;
//
//    RegionHash playerRegionHash=descriptors.regionHash(playerRegionIndex);
//    ChunkHash playerChunkHash=descriptors.chunkHash(playerRegionIndex);
//    Key playerChunkKey(playerRegionHash, playerChunkHash);
//
//    //make sure player chunk added
//    addChunk.insert(AddMap::value_type(playerChunkKey.hash, true));
//
//    for(auto regionIter=m_regionRenderers.begin(); regionIter!=m_regionRenderers.end(); )
//    {
//        RegionRendererType &regionRenderer=regionIter->second;
//        glm::ivec3 regionOffset=regionRenderer.index-playerRegionIndex;
//
//        regionRenderer.offset=regionOffset*m_grid->getDescriptors().m_regionCellSize;
//
//        auto &chunkRendererMap=regionRenderer.chunkRenderers;
//
//        for(auto chunkIter=chunkRendererMap.begin(); chunkIter!=chunkRendererMap.end(); )
//        {
//            auto *chunkRenderer=chunkIter->second;
//            Key key(chunkRenderer->getRegionHash(), chunkRenderer->getChunkHash());
//
//            glm::ivec3 regionIndex=m_grid->getDescriptors().regionIndex(key.regionHash);
//            glm::ivec3 chunkIndex=m_grid->getDescriptors().chunkIndex(key.chunkHash);
//            float chunkDistance=m_grid->getDescriptors().distance(playerRegionIndex, playerChunkIndex, regionIndex, chunkIndex);
//
//            //chunk outside of range so invalidate
//            if(chunkDistance>m_viewRadiusMax)
//            {
//                addChunk[key.hash]=false;
//                if(chunkRenderer->refCount==0) //need to keep if in prepThread
//                {
//                    chunkRenderer->invalidate();
//                    chunkIter=chunkRendererMap.erase(chunkIter);
//                    m_freeChunkRenderers.push_back(chunkRenderer);
//                    continue;
//                }
//            }
//            else
//            {
//                addChunk[key.hash]=false;
//
//                //add neighbors chunks
//                for(int z=-1; z<=1; ++z)
//                {
//                    for(int y=-1; y<=1; ++y)
//                    {
//                        for(int x=-1; x<=1; ++x)
//                        {
//                            glm::ivec3 neighborRegion=regionIndex;
//                            glm::ivec3 neighborChunk=chunkIndex+glm::ivec3(x, y, z);
//
//                            descriptors.adjustRegion(neighborRegion, neighborChunk);
//                            float chunkDistance=m_grid->getDescriptors().distance(playerRegionIndex, playerChunkIndex, neighborRegion, neighborChunk);
//
//                            if(chunkDistance<m_viewRadiusMax)
//                            {
//                                RegionHash regionHash=descriptors.regionHash(neighborRegion);
//                                ChunkHash chunkHash=descriptors.chunkHash(neighborChunk);
//                                Key key(regionHash, chunkHash);
//
//                                if(addChunk.find(key.hash)==addChunk.end())
//                                    addChunk.insert(AddMap::value_type(key.hash, true));
//                            }
//                        }
//                    }
//                }
//                ++chunkIter;
//            }
//        }
//
//        if(chunkRendererMap.empty())
//            regionIter=m_regionRenderers.erase(regionIter);
//        else
//            ++regionIter;
//    }
//
//    size_t addedRenderers=0;
//    for(auto &iter:addChunk)
//    {
//        if(iter.second)
//        {
//            ChunkRenderType *chunkRenderer=getFreeRenderer();
//
//            if(chunkRenderer==nullptr)
//                continue;
//
//            Key key(iter.first);
//            SharedChunkHandle chunkHandle=m_grid->getChunk(key.regionHash, key.chunkHash);
//
//            chunkRenderer->setChunk(chunkHandle);
//
//            auto regionIter=m_regionRenderers.find(key.regionHash);
//
//            if(regionIter==m_regionRenderers.end())
//            {
//                glm::ivec3 index=m_grid->getRegionIndex(key.regionHash);
//                glm::ivec3 offset=index*descriptors.getRegionCellSize();
//
//                auto interResult=m_regionRenderers.insert(RegionRendererMap::value_type(key.regionHash, RegionRendererType(key.regionHash, index, offset)));
//
//                assert(interResult.second);
//                regionIter=interResult.first;
//            }
//            
//            m_addedChunks.push_back(chunkRenderer);
//            regionIter->second.chunkRenderers.insert(ChunkRendererMap::value_type(key.chunkHash, chunkRenderer));
//#ifndef OCCLUSSION_QUERY
//            m_grid->loadChunk(chunkRenderer->getChunkHandle(), 0);
//#endif//!OCCLUSSION_QUERY
//
//            addedRenderers++;
//        }
//    }
//
//    if(addedRenderers > 0)
//        return true;
//    return false;
//}


//old search
//template<typename _Grid>
//bool SimpleRenderer<_Grid>::updateChunks()
//{
//    glm::ivec3 playerRegionIndex=m_grid->getRegionIndex(m_camera->getRegionHash());
//    glm::ivec3 playerChunkIndex=m_grid->getChunkIndex(m_camera->getPosition());
//
//    int chunkIndicesSize=m_chunkIndices.size();
//
//    //    std::unordered_map<RegionHash, glm::vec3> regions;
//    std::unordered_map<Key::Type, ChunkQueryOffset> chunks;
//    glm::ivec3 index;
//    glm::ivec3 currentRegionIndex;
//
//    for(size_t i=0; i<m_maxChunkRing; ++i)
//    {
//        std::vector<glm::ivec3> &chunkIndices=m_chunkIndices[i];
//
//        for(size_t j=0; j<chunkIndices.size(); ++j)
//        {
//            index=playerChunkIndex+chunkIndices[j];
//            currentRegionIndex=playerRegionIndex;
//
//            glm::vec3 regionOffset=m_grid->getDescriptors().adjustRegion(currentRegionIndex, index);
//            Key key=m_grid->getHashes(currentRegionIndex, index);
//
//            chunks.insert(std::pair<Key::Type, ChunkQueryOffset>(key.hash, ChunkQueryOffset(i, regionOffset)));
//        }
//    }
//
//    m_chunkQueryOrder.resize(m_maxChunkRing);
//
//    //invalidate renderers that pass outside max range
//    for(auto regionIter=m_regionRenderers.begin(); regionIter!=m_regionRenderers.end(); )
//    {
//        RegionRendererType &regionRenderer=regionIter->second;
//        glm::ivec3 regionOffset=regionRenderer.index-playerRegionIndex;
//
//        regionRenderer.offset=regionOffset*m_grid->getDescriptors().m_regionCellSize;
//
//        auto &chunkRendererMap=regionRenderer.chunkRenderers;
//
//        for(auto chunkIter=chunkRendererMap.begin(); chunkIter!=chunkRendererMap.end(); )
//        {
//            auto *chunkRenderer=chunkIter->second;
//            Key key(chunkRenderer->getRegionHash(), chunkRenderer->getChunkHash());
//
//            glm::ivec3 regionIndex=m_grid->getDescriptors().regionIndex(key.regionHash);
//            glm::ivec3 chunkIndex=m_grid->getDescriptors().chunkIndex(key.chunkHash);
//            float chunkDistance=m_grid->getDescriptors().distance(playerRegionIndex, playerChunkIndex, regionIndex, chunkIndex);
//
//            //chunk outside of range so invalidate
//            if(chunkDistance > m_viewRadiusMax)
//            {
//                if(chunkRenderer->refCount==0) //need to keep if in prepThread
//                {
//                    chunkRenderer->invalidate();
//                    chunkIter=chunkRendererMap.erase(chunkIter);
//                    m_freeChunkRenderers.push_back(chunkRenderer);
//                    continue;
//                }
//            }
//            else
//            {
//                auto iter=chunks.find(key.hash);
//
//                if(iter!=chunks.end())
//                {
//                    m_chunkQueryOrder[iter->second.queryRing].push_back(chunkRenderer);
//                    chunks.erase(iter);
//                }
//            }
//            ++chunkIter;
//        }
//
//        if(chunkRendererMap.empty())
//            regionIter=m_regionRenderers.erase(regionIter);
//        else
//            ++regionIter;
//    }
//
//    //add missing chunks
//    for(auto iter=chunks.begin(); iter!=chunks.end(); ++iter)
//    {
//        ChunkRenderType *chunkRenderer=getFreeRenderer();
//
//        if(chunkRenderer==nullptr)
//            continue;
//
//        Key key(iter->first);
//        SharedChunkHandle chunkHandle=m_grid->getChunk(key.regionHash, key.chunkHash);
//
//        //        m_grid->loadChunk(chunkHandle, 0);
//
//        //        chunkRenderer->setRegionHash(key.regionHash);
//        chunkRenderer->setChunk(chunkHandle);
//        //        chunkRenderer->setChunkOffset(iter->second.offset);
//        //        chunkRenderer->update();
//
//        m_chunkQueryOrder[iter->second.queryRing].push_back(chunkRenderer);
//
//        auto regionIter=m_regionRenderers.find(key.regionHash);
//
//        if(regionIter==m_regionRenderers.end())
//        {
//            glm::ivec3 index=m_grid->getRegionIndex(key.regionHash);
//
//            auto interResult=m_regionRenderers.insert(RegionRendererMap::value_type(key.regionHash, RegionRendererType(key.regionHash, index, iter->second.offset)));
//
//            assert(interResult.second);
//            regionIter=interResult.first;
//        }
//
//        regionIter->second.chunkRenderers.insert(ChunkRendererMap::value_type(key.chunkHash, chunkRenderer));
//    }
//
//    //we likely altered the chunks, start the occlussion query over
//    m_currentQueryRing=0;
//
//    return false;
//}


}//namespace voxigen

#endif //_voxigen_chunkSearch_h_