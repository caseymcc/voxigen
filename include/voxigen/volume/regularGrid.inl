namespace voxigen
{

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::RegularGrid():
m_dataStore(&m_descriptors)
//m_dataStore(&m_descriptors, &m_processQueue, &m_generatorQueue, &m_updateQueue),
//m_generatorQueue(&m_descriptors, &m_updateQueue),
//m_processQueue(&m_descriptors)
//m_chunkHandler(&m_descriptors)
{
//    chunkUpdateCallback=std::bind(&RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::defaultChunkUpdateCallback, this);

//    if(_Thread)
//    {
//        m_processThreadRunning=true;
//        m_processThread=std::thread(std::bind(&RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::processThread, this));
//    }

    getProcessThread().setChunkRequestCallback(std::bind(&RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::processRequest, this, std::placeholders::_1));
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::~RegularGrid()
{
    m_dataStore.terminate();
//    m_generatorQueue.terminate();

//    if(_Thread)
//    {
//        //shutdown processing thread
//        {
//            std::unique_lock<std::mutex> lock(m_processMutex);
//
//            m_processThreadRunning=false;
//            m_processQueue.trigger(lock);//force update
//        }
//        m_processThread.join();
//    }
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::create(const std::string &directory, const std::string &name, const glm::ivec3 &size, const std::string &generatorName, LoadProgress &progress)
{
    m_name=name;
    m_directory=directory;

    m_descriptors.create(name, 0, size);// , glm::ivec3(_RegionSizeX, _RegionSizeY, _RegionSizeZ), glm::ivec3(_ChunkSizeX, _ChunkSizeY, _ChunkSizeZ));
    m_descriptors.init();
    m_descriptors.m_generator=generatorName;

    m_generator=createClass<Generator>(generatorName);
    m_generator->saveDescriptors(&m_descriptors);
//    m_generatorQueue.setGenerator(m_generator.get());

    std::string configFile=directory+"/gridConfig.json";
    m_descriptors.save(configFile);

    std::string regionDirectory=directory+"/regions";
    //    fs::path regionPath(regionDirectory);
    std::string regionPath(regionDirectory);

    fs::create_directory(regionPath.c_str());

//    m_generator->create(&m_descriptors, progress);

    std::string generatorDirectory=directory+"/generator";

    m_generator->load(&m_descriptors, generatorDirectory, progress);//will create if not there and save
//    m_generator->save(generatorDirectory);

    m_dataStore.initialize();
//    m_generatorQueue.initialize();
}

//Default processing thread, can be turned off with _Thread template variable
template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::processThread()
{
    std::unique_lock<std::mutex> lock(m_processMutex);
//    std::unique_lock<std::mutex> &lock=m_processQueue.getLock();

    while(m_processThreadRunning)
    {
//        process::Type type;
//        size_t lod;
//        SharedChunkHandle chunkHandle=m_processQueue.getNextProcessRequest(lock, type, lod);
        SharedChunkHandle chunkHandle;

        ProcessRequest *request=m_processQueue.getNextProcessRequest();
        
        if(!request)
        {
//            m_processEvent.wait(lock);
            if(m_processQueue.empty())
            {
                //check for more items, this has a lock
                m_processQueue.updatePriorityQueue();
                
                if(m_processQueue.empty())
                {
                    //update completed before wait
                    m_processQueue.updateCompletedQueue();
                    m_processQueue.wait(lock);
                }
            }
            continue;
        }

//        lock.unlock();

//        bool addChunk=false;
//        ProcessRequest *request=sharedRequest.get();

        switch(request->type)
        {
        case process::GenerateRegion:
            processGenerateRegion(request);
            break;
        case process::Generate:
            processGenerate(request);
            break;
        case process::Read:
            processRead(request);
            break;
        case process::Write:
            processWrite(request);
            break;
        case process::Update:
            processUpdate(request);
            break;
        case process::Release:
            processRelease(request);
            break;
        }

        m_processQueue.addCompletedRequest(request);
        if(m_processQueue.isCompletedQueueEmpty())
        {
            m_processQueue.updateCompletedQueue();
        }
    }
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
bool RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::processRequest(process::Request *request)
{
    bool processed=false;

    switch(request->type)
    {
//    case process::Type::GenerateRegion:
//        processGenerateRegion(request);
//        break;
    case process::Type::Generate:
        processed=processGenerate(request);
        break;
    case process::Type::Read:
        processed=processRead(request);
        break;
    case process::Type::Write:
        processed=processWrite(request);
        break;
//    case process::Type::Update:
//        processed=processUpdate(request);
//        break;
//    case process::Type::Release:
//        processRelease(request);
//        break;
    default:
        break;
    }

    return processed;
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
bool RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::processGenerateRegion(process::Request *request)
{
//    RegionHandleType *regionHandle=request->handle.region;

#ifdef LOG_PROCESS_QUEUE
    Log::debug("ProcessThread - Region %llx (%d, %d) generate", regionHandle, regionHandle->hash(), regionHandle->hash());
#endif//LOG_PROCESS_QUEUE
//    regionHandle->generate(&m_descriptors, m_generator.get(), request->lod);
    return true;
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
bool RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::processGenerate(process::Request *request)
{
    ChunkHandleType *chunkHandle=(ChunkHandleType *)request->data.chunk.handle;

#ifdef LOG_PROCESS_QUEUE
    Log::debug("ProcessThread - Chunk %llx (%d, %d) generate", chunkHandle, chunkHandle->regionHash(), chunkHandle->hash());
#endif//LOG_PROCESS_QUEUE
    chunkHandle->generate(&m_descriptors, m_generator.get(), request->data.chunk.lod);
    return true;
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
bool RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::processRead(process::Request *request)
{
    ChunkHandleType *chunkHandle=(ChunkHandleType *)request->data.chunk.handle;

#ifdef LOG_PROCESS_QUEUE
    Log::debug("ProcessThread - Chunk %llx (%d, %d) read", chunkHandle, chunkHandle->regionHash(), chunkHandle->hash());
#endif//LOG_PROCESS_QUEUE
    m_dataStore.readChunk(chunkHandle);
    return true;
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
bool RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::processWrite(process::Request *request)
{
    ChunkHandleType *chunkHandle=(ChunkHandleType *)request->data.chunk.handle;

#ifdef LOG_PROCESS_QUEUE
    Log::debug("ProcessThread - Chunk %llx (%d, %d) write", chunkHandle, chunkHandle->regionHash(), chunkHandle->hash());
#endif//LOG_PROCESS_QUEUE
    m_dataStore.writeChunk(chunkHandle);
    return true;
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
bool RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::processUpdate(process::Request *request)
{
    ChunkHandleType *chunkHandle=(ChunkHandleType *)request->data.chunk.handle;

#ifdef LOG_PROCESS_QUEUE
    Log::debug("ProcessThread - Chunk %llx (%d, %d) update", chunkHandle, chunkHandle->regionHash(), chunkHandle->hash());
#endif//LOG_PROCESS_QUEUE
    return true;
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
bool RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::processRelease(process::Request *request)
{
//    typedef ReleaseRequest<RegionType, ChunkType> ReleaseRequest;
//
//    ReleaseRequest *releaseRequest=static_cast<ReleaseRequest *>(request);
//    SharedChunkHandle chunkHandle=releaseRequest->chunkHandle.lock();
//
//    if(!chunkHandle)
//        return;
//
//#ifdef LOG_PROCESS_QUEUE
//    Log::debug("ProcessThread - Chunk %llx (%d, %d) release", chunkHandle, chunkHandle->regionHash(), chunkHandle->hash());
//#endif//LOG_PROCESS_QUEUE
//    chunkHandle->release();
    return true;
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
bool RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::load(const std::string &directory, LoadProgress &progress)
{
    m_directory=directory;

    std::string configFile=directory+"/gridConfig.json";
    
    if(!m_descriptors.load(configFile))
        return false;

    m_generator=createClass<Generator>(m_descriptors.m_generator);
//    m_generatorQueue.setGenerator(m_generator.get());

    std::string regionDirectory=directory+"/regions";
    m_dataStore.load(regionDirectory);

    std::string generatorDirectory=directory+"/generator";
//    m_generator->initialize(&m_descriptors);
    if(!m_generator->load(&m_descriptors, generatorDirectory, progress))
    {
        //had to generate something, need to save
        m_generator->save(generatorDirectory);
    }

    m_dataStore.initialize();
//    m_generatorQueue.initialize();

    return true;
//    std::string chunkDirectory=directory+"/chunks";
//
//    m_chunkHandler.load(chunkDirectory);
//    m_chunkHandler.initialize();
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
bool RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::save()
{
    std::string configFile=m_directory+"/gridConfig.json";
    m_descriptors.save(configFile);

    std::string generatorDirectory=directory+"/generator";
    m_generator->save(generatorDirectory);

    return true;
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
bool RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::saveTo(const std::string &directory)
{
    std::string configFile=directory+"/gridConfig.json";
    m_descriptors.save(configFile);
    return true;
}

//template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
//Biome &RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getBiome(glm::ivec3 cell)
//{}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::SharedRegionHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getRegion(const glm::ivec3 &index)
{
    if(!details::validIndex(index, getRegionCount()))
        return SharedRegionHandle();

    RegionHash hash=m_descriptors.regionHash(index);

    return m_dataStore.getRegion(hash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
bool RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::loadRegion(RegionHandleType *handle, size_t lod, bool force)
{
    return m_dataStore.loadRegion(handle, lod, force);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
bool RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::cancelLoadRegion(RegionHandleType * handle)
{
    return m_dataStore.cancelLoadRegion(handle);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::SharedRegionHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getRegion(RegionHash hash)
{
    return m_dataStore.getRegion(hash);
}

//template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
//bool RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::updatePosition(const glm::ivec3 &region, const glm::ivec3 &chunk)
//{
////    return m_processQueue.updatePosition(region, chunk);
////    getProcessThread().updatePosition(region, chunk);
//    return true;
//}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::size() const
{
    return m_descriptors.m_size;
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::regionCellSize()
{
    return glm::ivec3(regionCellSizeX::value, regionCellSizeY::value, regionCellSizeZ::value);
}

//template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
//typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::SharedChunkHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunk(const glm::ivec3 &cell)
//{
//    glm::ivec3 chunkIndex=cell/m_descriptors.m_chunkSize;
//
//    ChunkHash chunkHash=getChunkHash(chunkIndex);
//
//    return m_dataStore.getChunk(chunkHash);
//}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::SharedChunkHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunk(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex)
{
    if(!details::validIndex(regionIndex, getRegionCount()))
        return SharedChunkHandle();
    if(!details::validIndex(chunkIndex, getChunkSize()))
        return SharedChunkHandle();

    RegionHash regionHash=getRegionHash(regionIndex);
    ChunkHash chunkHash=getChunkHash(chunkIndex);

    return m_dataStore.getChunk(regionHash, chunkHash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::SharedChunkHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunk(Key &key)
{
    return m_dataStore.getChunk(key.regionHash, key.chunkHash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
typename RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::SharedChunkHandle RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunk(RegionHash regionHash, ChunkHash chunkHash)
{
    return m_dataStore.getChunk(regionHash, chunkHash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
bool RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::loadChunk(ChunkHandleType *chunkHandle, size_t lod, bool force)
{
    return m_dataStore.loadChunk(chunkHandle, lod, force);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
bool RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::cancelLoadChunk(ChunkHandleType *chunkHandle)
{
    return m_dataStore.cancelLoadChunk(chunkHandle);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::releaseChunk(ChunkHandleType *chunkHandle)
{
    //send to thread to release as it could be processing it as well
//    m_processQueue.addRelease(chunkHandle);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getUpdated(std::vector<RegionHash> &updatedRegions, std::vector<Key> &updatedChunks, RequestQueue &requests)
{
//    std::vector<Key> updatedChunks;
//
//    updatedChunks=m_updateQueue.get();
//    return updatedChunks;

//    if(m_processQueue.isCompletedQueueEmpty())
//        m_processQueue.updateCompletedQueue();

//    typename ProcessQueueType::RequestQueue completedQueue;
    RequestQueue completedQueue;

//    m_processQueue.getCompletedQueue(completedQueue);
    getProcessThread().updateQueues(completedQueue);

    for(auto &request:completedQueue)
    {
//        typename ProcessQueueType::SharedChunkHandle chunkHandle=request->getChunkHandle();
//
//        if(!chunkHandle)
//            continue;
        if(request->type==process::Type::GenerateRegion)
        {
            handleGenerateRegionComplete(request, updatedRegions);
        }
        else if(request->type==process::Type::Generate)
        {
            handleGenerateComplete(request, updatedChunks);
        }
        else if(request->type==process::Type::Read)
        {
            handleReadComplete(request, updatedChunks);
        }
        else if(request->type==process::Type::Write)
        {
#ifdef DEBUG_REQUESTS
            Log::debug("getUpdated release request %llx", request);
#endif
            getProcessThread().releaseRequest(request);
        }
//        else if(request->type==process::Type::Update)
//        {
//            handleUpdateComplete(request, updatedChunks);
//        }
        else
        {
#ifdef DEBUG_MESH
            Log::debug("RegularGrid completed request %llx", request);
#endif
            requests.push_back(request);
        }
//        else if(request->type==process::Release)
//        {
//            handleReleaseComplete(request);
//        }

        
//        m_processQueue.releaseRequest(request);
    }
    completedQueue.clear();
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::handleGenerateRegionComplete(ProcessRequest *request, std::vector<RegionHash> &updated)
{
    RegionHandleType *handle=(RegionHandleType *)request->data.region.handle;

#ifdef LOG_PROCESS_QUEUE
    Log::debug("MainThread - RegionHandle %llx (%d) generate complete", handle, handle->hash());
#endif//LOG_PROCESS_QUEUE
    handle->setState(HandleState::Memory);
    handle->setAction(HandleAction::Idle);

    updated.push_back(handle->hash());
#ifdef DEBUG_REQUESTS
    Log::debug("handleGenerateRegionComplete release request %llx", request);
#endif
    getProcessThread().releaseRequest(request);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::handleGenerateComplete(ProcessRequest *request, std::vector<Key> &updatedChunks)
{
    ChunkHandleType *chunkHandle=(ChunkHandleType *)request->data.chunk.handle;

#ifdef LOG_PROCESS_QUEUE
    Log::debug("MainThread - ChunkHandle %llx (%d, %d) generate complete", chunkHandle, chunkHandle->regionHash(), chunkHandle->hash());
#endif//LOG_PROCESS_QUEUE
    chunkHandle->setState(HandleState::Memory);
    chunkHandle->setAction(HandleAction::Idle);

    updatedChunks.push_back(chunkHandle->key());
#ifdef DEBUG_REQUESTS
    Log::debug("handleGenerateComplete release request %llx", request);
#endif
    getProcessThread().releaseRequest(request);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::handleReadComplete(ProcessRequest *request, std::vector<Key> &updatedChunks)
{
    ChunkHandleType *chunkHandle=(ChunkHandleType *)request->data.chunk.handle;

#ifdef LOG_PROCESS_QUEUE
    Log::debug("MainThread - ChunkHandle %llx (%d, %d) read complete", chunkHandle, chunkHandle->regionHash(), chunkHandle->hash());
#endif//LOG_PROCESS_QUEUE

    chunkHandle->setState(HandleState::Memory);
    chunkHandle->setAction(HandleAction::Idle);
    updatedChunks.push_back(chunkHandle->key());
#ifdef DEBUG_REQUESTS
    Log::debug("handleReadComplete release request %llx", request);
#endif
    getProcessThread().releaseRequest(request);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::handleUpdateComplete(ProcessRequest *request, std::vector<Key> &updatedChunks)
{
    ChunkHandleType *chunkHandle=(ChunkHandleType *)request->data.chunk.handle;

#ifdef LOG_PROCESS_QUEUE
    Log::debug("MainThread - ChunkHandle %llx (%d, %d) update complete", chunkHandle, chunkHandle->regionHash(), chunkHandle->hash());
#endif//LOG_PROCESS_QUEUE

    chunkHandle->setState(HandleState::Memory);
    chunkHandle->setAction(HandleAction::Idle);
    updatedChunks.push_back(chunkHandle->key());
#ifdef DEBUG_REQUESTS
    Log::debug("handleUpdateComplete release request %llx", request);
#endif
    getProcessThread().releaseRequest(request);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::handleReleaseComplete(ProcessRequest *request)
{
    ChunkHandleType *chunkHandle=(ChunkHandleType *)request->data.chunk.handle;

#ifdef LOG_PROCESS_QUEUE
    Log::debug("MainThread - ChunkHandle %llx (%d, %d) release complete", chunkHandle, chunkHandle->regionHash(), chunkHandle->hash());
#endif//LOG_PROCESS_QUEUE

    chunkHandle->setState(HandleState::Unknown);
    chunkHandle->setAction(HandleAction::Idle);
#ifdef DEBUG_REQUESTS
    Log::debug("handleReleaseComplete release request %llx", request);
#endif
    getProcessThread().releaseRequest(request);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
RegionHash RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getRegionHash(const glm::ivec3 &index)
{
    return m_descriptors.regionHash(index);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getRegionIndex(const glm::vec3 &position)
{
    return m_descriptors.regionIndex(position);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getRegionIndex(RegionHash hash)
{
    return m_descriptors.regionIndex(hash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
ChunkHash RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunkHash(const glm::ivec3 &index) const
{
    return m_descriptors.chunkHash(index);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunkIndex(const glm::vec3 &position)
{
    glm::ivec3 pos=glm::ivec3(glm::floor(position));

    return pos/m_descriptors.m_chunkSize;
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunkIndex(ChunkHash hash)
{
    return m_descriptors.chunkIndex(hash);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
Key RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getHashes(const glm::vec3 &gridPosition)
{
    glm::ivec3 position=glm::ivec3(glm::floor(gridPosition));
    glm::ivec3 regionCellSize(regionCellSizeX::value, regionCellSizeY::value, regionCellSizeZ::value);
    glm::ivec3 regionIndex=position/regionCellSize;
    glm::ivec3 chunkIndex=(position-(regionIndex*regionCellSize))/m_descriptors.m_chunkSize;

    return Key(m_descriptors.regionHash(regionIndex), m_descriptors.chunkHash(chunkIndex));
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
Key RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getHashes(const glm::ivec3 &regionIndex, const glm::ivec3 &chunkIndex)
{
    return Key(m_descriptors.regionHash(regionIndex), m_descriptors.chunkHash(chunkIndex));
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
ChunkHash RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunkHash(RegionHash regionHash, const glm::vec3 &gridPosition)
{
    glm::ivec3 position=glm::floor(gridPosition);
    glm::ivec3 regionCellSize(regionCellSizeX::value, regionCellSizeY::value, regionCellSizeZ::value);
    glm::ivec3 regionIndex=getRegionIndex(regionHash);

    return m_descriptors.chunkHash(position-(regionIndex*regionCellSize));
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
ChunkHash RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunkHashFromRegionPos(const glm::vec3 &regionPosition)
{
    glm::ivec3 position=glm::floor(regionPosition);

    return m_descriptors.chunkHash(position/m_descriptors.m_chunkSize);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
ChunkHash RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunkHash(const glm::vec3 &gridPosition) const
{
    glm::ivec3 position=glm::floor(gridPosition);
    glm::ivec3 regionCellSize(regionCellSizeX::value, regionCellSizeY::value, regionCellSizeZ::value);
    glm::ivec3 regionIndex=position/regionCellSize;
    glm::ivec3 chunkIndex=position-(regionIndex*regionCellSize);

    return m_descriptors.chunkHash(chunkIndex);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
glm::ivec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunkSize()
{
    return glm::ivec3(_ChunkSizeX, _ChunkSizeY, _ChunkSizeZ);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
size_t RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::getChunkRequestSize()
{
    return m_processQueue.getRequestSize();
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
void RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::setChunkRequestSize(size_t size)
{
    //only englarging as needed, doesn't support shrink yet
    if(size>m_processQueue.getRequestSize())
        m_processQueue.setRequestSize(size);
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
glm::vec3 RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::gridPosToRegionPos(RegionHash regionHash, const glm::vec3 &gridPosition)
{
    glm::vec3 pos=gridPosition-m_descriptors.regionOffset(regionHash);

    return pos;
}

template<typename _Cell, size_t _ChunkSizeX, size_t _ChunkSizeY, size_t _ChunkSizeZ, size_t _RegionSizeX, size_t _RegionSizeY, size_t _RegionSizeZ, bool _Thread>
bool RegularGrid<_Cell, _ChunkSizeX, _ChunkSizeY, _ChunkSizeZ, _RegionSizeX, _RegionSizeY, _RegionSizeZ, _Thread>::alignPosition(glm::ivec3 &regionIndex, glm::vec3 &position)
{
    bool updateRegion=false;
    glm::ivec3 regionCellSize(regionCellSizeX::value, regionCellSizeY::value, regionCellSizeZ::value);

    if(position.x<0)
    {
        regionIndex.x--;
        position.x+=regionCellSize.x;
        updateRegion=true;
    }
    else if(position.x>regionCellSize.x)
    {
        regionIndex.x++;
        position.x-=regionCellSize.x;
        updateRegion=true;
    }

    if(position.y<0)
    {
        regionIndex.y--;
        position.y+=regionCellSize.y;
        updateRegion=true;
    }
    else if(position.y>regionCellSize.y)
    {
        regionIndex.y++;
        position.y-=regionCellSize.y;
        updateRegion=true;
    }

    if(position.z<0)
    {
        regionIndex.z--;
        position.z+=regionCellSize.z;
        updateRegion=true;
    }
    else if(position.z>regionCellSize.z)
    {
        regionIndex.z++;
        position.z-=regionCellSize.z;
        updateRegion=true;
    }

    return updateRegion;
}


}//namespace voxigen

