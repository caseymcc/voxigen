namespace voxigen
{

template<typename _Region>
RegionHandle<_Region>::RegionHandle(RegionHash regionHash, IGridDescriptors *descriptors/*, GeneratorQueue<_Grid> *generatorQueue, DataStore<_Grid> *dataStore, UpdateQueue *updateQueue*/):
//m_status(Unknown),
m_state(HandleState::Unknown),
m_action(HandleAction::Idle),
m_version(0),
m_hash(regionHash),
m_descriptors(descriptors),
//m_generatorQueue(generatorQueue),
//m_dataStore(dataStore),
//m_updateQueue(updateQueue),
m_cachedOnDisk(false),
m_empty(false)
{
    m_index=descriptors->getRegionIndex(regionHash);
}

template<typename _Region>
void RegionHandle<_Region>::generate(IGridDescriptors *descriptors, Generator *generator, size_t lod)
{
    glm::vec3 startPos=descriptors->getRegionOffset(m_hash);

#ifdef DEBUG_ALLOCATION
    allocated++;
    Log::debug("RegionHandle (%llx) allocating by generate\n", m_hash);
#endif
    glm::ivec3 size=details::regionCellSize<RegionType, ChunkType>();

//    size.z=1;//only heighmap so only 1 z
    m_heightMap.resize(size.x*size.y);

    unsigned int validCells=generator->generateRegion(startPos, size, m_heightMap.data(), m_heightMap.size()*sizeof(Cells), lod);

    if(validCells<=0)
    {
        m_heightMapLod=0;
        release();
        setEmpty(true);
    }
    else
    {
//        m_memoryUsed=m_heightMap.size()*sizeof(typename ChunkType::CellType);
        m_heightMapLod=lod;
        setEmpty(false);
    }
}

template<typename _Region>
void RegionHandle<_Region>::release()
{
    if(!m_heightMap.empty())
    {
#ifdef DEBUG_ALLOCATION
        allocated--;
        Log::debug("RegionHandle (%llx) freeing (%d)\n", m_hash, allocated);
#endif
    }
    m_heightMap.clear();
//    m_memoryUsed=0;
}

template<typename _Region>
bool RegionHandle<_Region>::load(const std::string &directory)
{
    m_directory=directory;

    if(!fs::is_directory(directory))
    {
        if(fs::exists(directory))
            return false;

        fs::create_directory(directory);
    }

    m_configFile=m_directory+"/regionConfig.json";

    if(!fs::exists(m_configFile))
        saveConfig();
    else
        loadConfig();

    loadDataStore();
    verifyDirectory();

//    m_status=Loaded;
    m_state=HandleState::Memory;

    return true;
}

template<typename _Region>
typename RegionHandle<_Region>::DataHandle *RegionHandle<_Region>::newHandle(HashType chunkHash)
{
    glm::ivec3 regionIndex=m_descriptors->getRegionIndex(m_hash);
    glm::ivec3 chunkIndex=m_descriptors->getChunkIndex(chunkHash);

    return new ChunkHandleType(m_hash, regionIndex, chunkHash, chunkIndex);
}

template<typename _Region>
void RegionHandle<_Region>::loadConfig()
{
    generic::JsonDeserializer deserializer;

    deserializer.open(m_configFile.c_str());

    deserializer.openObject();
    if(deserializer.key("version"))
        m_version=deserializer.getUInt();

    if(deserializer.key("chunks"))
    {
        if(deserializer.openArray())
        {
            do
            {
                if(deserializer.openObject())
                {
                    if(deserializer.key("id"))
                    {
                        RegionHash hash=deserializer.getUInt();
                        SharedChunkHandle chunkHandle(newHandle(hash));

                        chunkHandle->setCachedOnDisk(true);

                        if(deserializer.key("empty"))
                            chunkHandle->setEmpty(deserializer.getBool());
                        else
                            chunkHandle->setEmpty(true);

                        this->m_dataHandles.insert(typename SharedDataHandleMap::value_type(hash, chunkHandle));
                    }

                    deserializer.closeObject();
                }
            } while(deserializer.advance());
            deserializer.closeArray();
        }
    }
    deserializer.closeObject();
}

template<typename _Region>
void RegionHandle<_Region>::saveConfig()
{
    saveConfigTo(m_configFile);
}

template<typename _Region>
void RegionHandle<_Region>::saveConfigTo(std::string configFile)
{
    generic::JsonSerializer serializer;

    serializer.open(m_configFile.c_str());

    serializer.startObject();

    serializer.addKey("version");
    serializer.addInt(m_version);

    serializer.addKey("chunks");
    serializer.startArray();
    for(auto &handle:this->m_dataHandles)
    {
        if(handle.second->empty())
        {
            serializer.startObject();
            serializer.addKey("id");
            serializer.addUInt(handle.second->hash());
            serializer.addKey("empty");
            serializer.addBool(handle.second->empty());
            serializer.endObject();
        }
    }
    serializer.endArray();

    serializer.endObject();
}

template<typename _Region>
void RegionHandle<_Region>::addConfig(ChunkHandleType *handle)
{
    if(handle->empty())
    {
        std::string tempConfig=m_configFile+".tmp";
        saveConfigTo(tempConfig);
        fs::copy_file(tempConfig, m_configFile, true);
    }
}

template<typename _Region>
void RegionHandle<_Region>::loadDataStore()
{
    std::vector<std::string> directories=fs::get_directories(m_directory);

    for(auto &entry:directories)
    {
        std::istringstream fileNameStream(entry);
        ChunkHash chunkHash;

        fileNameStream.ignore(6);
        fileNameStream>>std::hex>>chunkHash;

        glm::ivec3 regionIndex=m_descriptors->getRegionIndex(m_hash);
        glm::ivec3 chunkIndex=m_descriptors->getChunkIndex(chunkHash);

        SharedChunkHandle handle=std::make_shared<ChunkHandleType>(m_hash, regionIndex, chunkHash, chunkIndex);

        handle->setCachedOnDisk(true);
        handle->setEmpty(false);

        this->m_dataHandles.insert(typename SharedDataHandleMap::value_type(chunkHash, handle));
    }
}

template<typename _Region>
void RegionHandle<_Region>::verifyDirectory()
{

}

template<typename _Region>
typename RegionHandle<_Region>::SharedChunkHandle RegionHandle<_Region>::getChunk(ChunkHash chunkHash)
{
    SharedChunkHandle chunkHandle=this->getDataHandle(chunkHash);

    glm::ivec3 chunkIndex=m_descriptors->getChunkIndex(chunkHash);
    chunkHandle->setRegionOffset(glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value)*chunkIndex);


//    if(chunkHandle->status!=ChunkHandleType::Memory)
//    {
//        if(chunkHandle->empty) //empty is already loaded
//        {
//            chunkHandle->status=ChunkHandleType::Memory;
//            m_updateQueue->add(Key(hash, chunkHash));
//        }
//        else
//        {
//            //we dont have it in memory so we need to load or generate it
//            if(!chunkHandle->cachedOnDisk) 
//                m_generatorQueue->add(chunkHandle);
//            else
//                m_dataStore->read(chunkHandle);
//        }
//    }

    return chunkHandle;
}

//template<typename _Region>
//void RegionHandle<_Region>::loadChunk(SharedChunkHandle chunkHandle, size_t lod, bool force)
//{
//    if(force || (chunkHandle->state() != HandleState::Memory))
//    {
//        //an action is in process lets not start something else as well
//        if(chunkHandle->action()!=HandleAction::Idle)
//            return;
//
//        if(chunkHandle->empty()) //empty is already loaded
//        {
////            m_dataStore->empty(chunkHandle);
//        }
//        else
//        {
//            //we dont have it in memory so we need to load or generate it
//            if(!chunkHandle->cachedOnDisk()) 
//                m_dataStore->generate(chunkHandle, lod);
//            else
//                m_dataStore->read(chunkHandle, lod);
//        }
//    }
//}
//
//template<typename _Region>
//void RegionHandle<_Region>::cancelLoadChunk(SharedChunkHandle chunkHandle)
//{
//    //if we are not doing anything ignore
//    if(chunkHandle->action()==HandleAction::Idle)
//        return;
//    
//    m_dataStore->cancel(chunkHandle);
//}

} //namespace voxigen
