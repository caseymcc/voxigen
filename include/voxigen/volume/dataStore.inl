
namespace voxigen
{

template<typename _Grid>
DataStore<_Grid>::DataStore(GridDescriptors<_Grid> *descriptors):
m_descriptors(descriptors)
{
    m_version=0;
}

template<typename _Grid>
void DataStore<_Grid>::initialize()
{
//    m_ioThreadRun=true;
//    m_ioThread=std::thread(std::bind(&DataStore<_Grid>::ioThread, this));
}

template<typename _Grid>
void DataStore<_Grid>::terminate()
{
    //thread flags are not atomic so we need the mutexes to coordinate the setting, 
    //otherwise would have to loop re-notifiying thread until it stopped
//    {
//        std::unique_lock<std::mutex> lock(m_ioMutex);
//        m_ioThreadRun=false;
//    }
//
//    m_ioEvent.notify_all();
//    m_ioThread.join();
}

template<typename _Grid>
typename DataStore<_Grid>::DataHandle *DataStore<_Grid>::newHandle(HashType hash)
{
//    return new RegionHandleType(hash, m_descriptors, m_generatorQueue, this, m_updateQueue);
    return new RegionHandleType(hash, m_descriptors);
}

template<typename _Grid>
bool DataStore<_Grid>::load(const std::string &directory)
{
    m_directory=directory;

    if(!fs::is_directory(directory))
    {
        if(fs::exists(directory))
            return false;

        fs::create_directory(directory);
    }

    m_configFile=m_directory+"/cacheConfig.json";

    if(!fs::exists(m_configFile))
        saveConfig();
    else
        loadConfig();

    loadDataStore();
    verifyDirectory();
    return true;
}

template<typename _Grid>
void DataStore<_Grid>::loadConfig()
{
    JsonUnserializer serializer;

    serializer.open(m_configFile.c_str());

    serializer.openObject();
    if(serializer.key("version"))
        m_version=serializer.getUInt();

    if(serializer.key("regions"))
    {
        if(serializer.openArray())
        {
            do
            {
                if(serializer.openObject())
                {
                    if(serializer.key("id"))
                    {
                        RegionHash hash=serializer.getUInt();
                        SharedRegionHandle regionHandle(newHandle(hash));

                        regionHandle->setCachedOnDisk(true);

                        if(serializer.key("empty"))
                            regionHandle->setEmpty(serializer.getBool());
                        else
                            regionHandle->setEmpty(true);

                        this->m_dataHandles.insert(typename SharedDataHandleMap::value_type(hash, regionHandle));
                    }

                    serializer.closeObject();
                }
            } while(serializer.advance());
            serializer.closeArray();
        }
    }
    serializer.closeObject();
}

template<typename _Grid>
void DataStore<_Grid>::saveConfig()
{
    saveConfigTo(m_configFile);
}

template<typename _Grid>
void DataStore<_Grid>::saveConfigTo(std::string configFile)
{
    JsonSerializer serializer;

    serializer.open(configFile.c_str());

    serializer.startObject();

    serializer.addKey("version");
    serializer.addInt(m_version);

    serializer.addKey("regions");
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

template<typename _Grid>
void DataStore<_Grid>::addConfig(SharedDataHandle handle)
{
    if(handle->empty)
    {
        std::string tempConfig=m_configFile+".tmp";

        saveConfigTo(tempConfig);
//        JsonUnserializer unserializer;
//        JsonSerializer serializer;
//
//        unserializer.open(m_configFile.c_str());
//        serializer.open(tempConfig.c_str());
//
//        unserializer.openObject();
//        serializer.startObject();
//
//        if(unserializer.key("version"))
//        {
//            unsigned int version=unserializer.getUInt();
//            serializer.addKey("version");
//            serializer.addUInt(version);
//        }
//
//        if(unserializer.key("regions"))
//        {
//            serializer.addKey("regions");
//            if(unserializer.openArray())
//            {
//                serializer.startArray();
//                do
//                {
//                    if(unserializer.openObject())
//                    {
//                        serializer.startObject();
//
//                        if(unserializer.key("id"))
//                        {
//                            RegionHash hash=unserializer.GetUint();
//
//                            serializer.addKey("id");
//                            serializer.addUInt(hash);
//                            
//                            bool empty=true;
//
//                            if(unserializer.key("empty"))
//                                empty=unserializer.GetBool();
//
//                            serializer.addKey("empty");
//                            serializer.addBool(empty);
//                        }
//
//                        unserializer.closeObject();
//                        serializer.endObject();
//                    }
//                } while(unserializer.advance());
//                
//                unserializer.closeArray();
//                serializer.endArray();
//            }
//        }
//        
//        unserializer.closeObject();
//        serializer.endObject();

//        fs::path configPath(m_configFile);
//        fs::path tempPath(tempConfig);
//        copy_file(tempPath, configPath, fs::copy_option::overwrite_if_exists);
        fs::copy_file(tempConfig, m_configFile, true);
    }
}

template<typename _Grid>
void DataStore<_Grid>::addConfig(ChunkHandleType *handle)
{
    //TODO - fix
    //lazy programming for the moment, see remarks in ioThread below
    if(handle->empty())
    {
        SharedRegionHandle regionHandle=getRegion(handle->regionHash());

        regionHandle->addConfig(handle);
    }
}

template<typename _Grid>
void DataStore<_Grid>::loadDataStore()
{
    std::vector<std::string> directories=fs::get_directories(m_directory);

    for(auto &entry:directories)
    {
        std::istringstream fileNameStream(entry);
        RegionHash hash;

        fileNameStream>>std::hex>>hash;

        SharedRegionHandle handle(newHandle(hash));

        handle->setCachedOnDisk(true);
        handle->setEmpty(false);

        this->m_dataHandles.insert(typename SharedDataHandleMap::value_type(hash, handle));
    }
}

template<typename _Grid>
void DataStore<_Grid>::verifyDirectory()
{

}

template<typename _Grid>
typename DataStore<_Grid>::SharedRegionHandle DataStore<_Grid>::getRegion(RegionHash hash)
{
    SharedRegionHandle regionHandle=this->getDataHandle(hash);

//    if(regionHandle->state()!=HandleState::Memory)
//    {
//        std::ostringstream directoryName;
//
//        directoryName<<std::hex<<hash;
//        std::string directory=m_directory+"/"+directoryName.str();
//
//        regionHandle->load(directory);
//    }
    return regionHandle;
}

template<typename _Grid>
bool DataStore<_Grid>::loadRegion(RegionHandleType *handle, size_t lod, bool force)
{
    bool value=false;

    if(force||(handle->state()!=HandleState::Memory))
    {
        //an action is in process lets not start something else as well
        if(handle->action()!=HandleAction::Idle)
            return value;

        if(!handle->empty())
            value=generateRegion(handle, lod);
    }

    return value;
}

template<typename _Grid>
bool DataStore<_Grid>::cancelLoadRegion(RegionHandleType *handle)
{
    if(handle->action()==HandleAction::Idle)
        return false;

//    cancelRegion(handle);
    return false;
}

template<typename _Grid>
typename DataStore<_Grid>::SharedChunkHandle DataStore<_Grid>::getChunk(RegionHash regionHash, ChunkHash chunkHash)
{
    return getRegion(regionHash)->getChunk(chunkHash);
}

template<typename _Grid>
bool DataStore<_Grid>::loadChunk(ChunkHandleType *chunkHandle, size_t lod, bool force)
{
    bool value=false;

//    return getRegion(handle->regionHash())->loadChunk(handle, lod, force);
    if(force||(chunkHandle->state()!=HandleState::Memory))
    {
        //an action is in process lets not start something else as well
        if(chunkHandle->action()!=HandleAction::Idle)
        {
#ifdef LOG_PROCESS_QUEUE
            Log::debug("MainThread - ChunkHandle %llx (%d, %d) load failed, not idle\n", chunkHandle, chunkHandle->regionHash(), chunkHandle->hash());
#endif//LOG_PROCESS_QUEUE
            return value;
        }
        
        if(!chunkHandle->empty())
        {
            //we dont have it in memory so we need to load or generate it
            if(!chunkHandle->cachedOnDisk())
            {
//                value=generate(chunkHandle, lod);
                value=getProcessThread().requestChunkGenerate(chunkHandle, lod);
                if(value)
                    chunkHandle->setAction(HandleAction::Generating);
#ifdef LOG_PROCESS_QUEUE
                Log::debug("MainThread - DataStore::loadChunk - ChunkHandle %llx (%d, %d) %s - request generate %d\n", chunkHandle, chunkHandle->regionHash(), chunkHandle->hash(),
                    getHandleActionName(chunkHandle->getAction()).c_str(), value);
#endif//LOG_PROCESS_QUEUE
            }
            else
            {
//                value=read(chunkHandle, lod);
                value=getProcessThread().requestChunkRead(chunkHandle, lod);
                if(value)
                    chunkHandle->setAction(HandleAction::Reading);
#ifdef LOG_PROCESS_QUEUE
                Log::debug("MainThread - DataStore::loadChunk - ChunkHandle %llx (%d, %d) %s - request read %d\n", chunkHandle, chunkHandle->regionHash(), chunkHandle->hash(),
                    getHandleActionName(chunkHandle->getAction()).c_str(), value);
#endif//LOG_PROCESS_QUEUE
            }
        }
    }

    return value;
}

template<typename _Grid>
bool DataStore<_Grid>::cancelLoadChunk(ChunkHandleType *chunkHandle)
{
//    return getRegion(handle->regionHash())->cancelLoadChunk(handle);
    //if we are not doing anything ignore
    if(chunkHandle->action()==HandleAction::Idle)
        return false;

//    return cancel(chunkHandle);
    if(chunkHandle->action() == HandleAction::Reading)
        return getProcessThread().cancelChunkRead(chunkHandle);
    else if(chunkHandle->action()==HandleAction::Writing)
        return getProcessThread().cancelChunkWrite(chunkHandle);
    else if(chunkHandle->action()==HandleAction::Generating)
        return getProcessThread().cancelChunkGenerate(chunkHandle);

    assert(false);
    return false;
}

template<typename _Grid>
void DataStore<_Grid>::ioThread()
{
    std::unique_lock<std::mutex> lock(m_ioMutex);

    //TODO - need batch system for cache updates
    //I am so hacking this up, going to store the config as a json doc for updating
    //need batching system for this, delayed writes (get multiple changes to chunks
    //in one write)
//    {
//        FILE *filePtr=fopen(m_configFile.c_str(), "rb");
//        char readBuffer[65536];
//
//        rapidjson::FileReadStream readStream(filePtr, readBuffer, sizeof(readBuffer));
//
//        m_configDocument.ParseStream(readStream);
//        fclose(filePtr);
//    }

    std::vector<SharedIORequest> requests;

    requests.reserve(10);
    while(m_ioThreadRun)
    {
        if(m_ioQueue.empty())
        {
            m_ioEvent.wait(lock);
            continue;
        }

        SharedIORequest request;
        size_t count=0;

        while((!m_ioQueue.empty()&&count<10))
        {

            requests.push_back(m_ioQueue.top());
            m_ioQueue.pop();
            count++;
        }

        if(!requests.empty())
        {
            lock.unlock();//drop lock while working

            for(size_t i=0; i<requests.size(); ++i)
            {
                switch(requests[i]->type)
                {
                case IORequestType::Read:
                    readChunk(requests[i].get());
                    break;
                case IORequestType::Write:
                    writeChunk(requests[i].get());
                    break;
                }
            }
            requests.clear();

            lock.lock();
        }
    }
}

template<typename _Grid>
void DataStore<_Grid>::readChunk(IORequestType *request)
{
    IOReadRequestType *readRequest=(IOReadRequestType *)request;

    //readChunk(readRequest->chunkHandle.get());
}

template<typename _Grid>
void DataStore<_Grid>::readChunk(ChunkHandleType *chunkHandle)
{
//    IOReadRequestType *readRequest=(IOReadRequestType *)request;
//    SharedChunkHandle chunkHandle=readRequest->chunkHandle.lock();

    if(!chunkHandle) //chunk no longer needed, drop it
        return;

    if(!chunkHandle->empty())
    {
        std::ostringstream fileNameStream;

        fileNameStream<<"/chunk_"<<std::right<<std::setfill('0')<<std::setw(8)<<std::hex<<chunkHandle->hash()<<".chk";
        std::string fileName=m_directory+fileNameStream.str();

        chunkHandle->read(m_descriptors, fileName);
//        glm::ivec3 chunkIndex=m_descriptors->chunkIndex(chunkHandle->hash);
//        glm::vec3 offset=glm::ivec3(ChunkType::sizeX::value, ChunkType::sizeY::value, ChunkType::sizeZ::value)*chunkIndex;
//
//        chunkHandle->chunk=std::make_unique<ChunkType>(chunkHandle->hash, 0, chunkIndex, offset);
//
//        auto &cells=chunkHandle->chunk->getCells();
//        std::ifstream file;
//
//        file.open(fileName, std::ofstream::in|std::ofstream::binary);
//        //        for(auto block:blocks)
//        //            block->deserialize(file);
//        file.read((char *)cells.data(), cells.size()*sizeof(_Grid::CellType));
//        file.close();
    }

//    chunkHandle->status=ChunkHandleType::Memory;
//    addToUpdatedQueue(chunkHandle->hash);
//    m_updateQueue->add(chunkHandle->hash());
}

template<typename _Grid>
void DataStore<_Grid>::writeChunk(IORequestType *request)
{
    IOWriteRequestType *writeRequest=(IOWriteRequestType *)request;

    writeChunk(writeRequest->chunkHandle.get());

    //drop shared_ptr
    writeRequest->chunkHandle.reset();
}

template<typename _Grid>
void DataStore<_Grid>::writeChunk(ChunkHandleType *chunkHandle)
{
//    IOWriteRequestType *writeRequest=(IOWriteRequestType *)request;
//    SharedChunkHandle chunkHandle=writeRequest->chunkHandle;

    if(!chunkHandle->empty())
    {
        std::ostringstream fileNameStream;

        fileNameStream<<"/chunk_"<<std::right<<std::setfill('0')<<std::setw(8)<<std::hex<<chunkHandle->hash()<<".chk";
        std::string fileName=m_directory+fileNameStream.str();

        chunkHandle->write(m_descriptors, fileName);
//        auto &cells=chunkHandle->chunk->getCells();
//        std::ofstream file;
//
//        file.open(fileName, std::ofstream::out|std::ofstream::trunc|std::ofstream::binary);
//        //        for(auto block:blocks)
//        //            block->serialize(file);
//        file.write((char *)cells.data(), cells.size()*sizeof(_Grid::CellType));
//        file.close();
    }

//    chunkHandle->cachedOnDisk=true;
    
    //need to alter this to save in batched and update config then
    addConfig(chunkHandle);

    //drop shared_ptr
//    writeRequest->chunkHandle.reset();
}


template<typename _Grid>
bool DataStore<_Grid>::generateRegion(RegionHandleType *handle, size_t lod)
{
#ifdef LOG_PROCESS_QUEUE
    Log::debug("MainThread - RegionHandle %llx (%d, %d) generating\n", handle.get(), chunkHandle->regionHash(), chunkHandle->hash());
#endif//LOG_PROCESS_QUEUE

//    bool value=m_processQueue->addGenerateRegion(handle, lod);
//
//    if(value)
//        handle->setAction(HandleAction::Generating);
//    return value;
    return false;
}

template<typename _Grid>
bool DataStore<_Grid>::generate(ChunkHandleType *chunkHandle, size_t lod)
{
#ifdef LOG_PROCESS_QUEUE
    Log::debug("MainThread - ChunkHandle %llx (%d, %d) generating\n", chunkHandle, chunkHandle->regionHash(), chunkHandle->hash());
#endif//LOG_PROCESS_QUEUE

//    bool value=m_processQueue->addGenerate(chunkHandle, lod);
//
//    if(value)
//        chunkHandle->setAction(HandleAction::Generating);
//    return value;
    return false;
}

template<typename _Grid>
bool DataStore<_Grid>::read(ChunkHandleType *chunkHandle, size_t lod)
{
#ifdef LOG_PROCESS_QUEUE
    Log::debug("MainThread -  ChunkHandle %llx (%d, %d) reading", chunkHandle, chunkHandle->regionHash(), chunkHandle->hash());
#endif//LOG_PROCESS_QUEUE
    
    getProcessThread()->requestChunkRead(chunkHandle, lod);
    return true;
}

template<typename _Grid>
bool DataStore<_Grid>::write(ChunkHandleType *chunkHandle)
{
#ifdef LOG_PROCESS_QUEUE
    Log::debug("MainThread -  ChunkHandle %llx (%d, %d) writing", chunkHandle, chunkHandle->regionHash(), chunkHandle->hash());
#endif//LOG_PROCESS_QUEUE
    
    getProcessThread()->requestChunkWrite(chunkHandle, lod);
    return true;
}

template<typename _Grid>
bool DataStore<_Grid>::empty(ChunkHandleType *chunkHandle)
{
    return false;
}

template<typename _Grid>
bool DataStore<_Grid>::cancel(ChunkHandleType *chunkHandle)
{
#ifdef LOG_PROCESS_QUEUE
    Log::debug("MainThread - ChunkHandle %llx (%d, %d) canceling", chunkHandle, chunkHandle->regionHash(), chunkHandle->hash());
#endif//LOG_PROCESS_QUEUE

//    bool value=m_processQueue->addCancel(chunkHandle);
//
//    return value;
    assert(false);
    return false;
}


} //namespace voxigen

