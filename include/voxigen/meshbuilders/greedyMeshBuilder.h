#ifndef _voxigen_greedyMeshBuilder_h_
#define _voxigen_greedyMeshBuilder_h_

namespace greedy
{
struct Quad
{
    Quad(const glm::ivec &start, const glm::ivec &delta):start(start), delta(delta) {}

    glm::ivec start;
    glm::ivec delta;
};
}

//template<typename _Chunk, typename _ChunkMesh, bool firstRow=false>
//void scan(typename _Chunk::Cells *cells, size_t size; size_t pitch, unsigned int *mask)
//{
//    size_t index=0;
//    size_t maskIndex=0;
//
//    typename _Chunk::CellType &cell=cells[index];
//    unsigned int prevCellType=type(cell);
//    unsigned int cellType;
//
//    for(size_t i=1; i<size; ++i)
//    {
//        cellType=type(cell);
//
//        mask[index]=0;
//        if(prevCellType!=cellType)
//            maskIndex=index;
//        mask[maskIndex]++;
//
//        prevCellType=cellType;
//        index+=pitch;
//    }
//}
//
//template<typename _Chunk, typename _ChunkMesh>
//void buildGreedyMesh(_ChunkMesh &mesh, _Chunk *chunk)
//{
//    typename _Chunk::Cells &cells=chunk->getCells();
//
//    size_t stride=glm::pow(2u, (unsigned int)chunk->getLod());
////    glm::ivec3 size(_Chunk::sizeX::value/stride, _Chunk::sizeY::value/stride, _Chunk::sizeZ::value/stride);
//
//    size_t maskSize=std::max(size.x*size.y, std::max(size.x*size.z, size.y*size.z));
//    std::vector<unsigned int> mask(maskSize);
//    
//    size_t size[3]={_Chunk::sizeX::value/stride, _Chunk::sizeY::value/stride, _Chunk::sizeZ::value/stride};
//    size_t pitch[3]={1, size[0], size[0]*size[1]};
//
//    size_t quadIndex=0;
//    std::vector<greedy::Quad> quads(maskSize);
//    size_t toggle=0;
//
//    for(size_t z=0; z<size[2]; ++z)
//    {
//        size_t pitch=size[1]*size[0];
//        size_t prevRowIndex=0;
//        size_t index=size[0];
//
//        for(size_t y=1; y<size[1]; ++y)
//        {
//            size_t *prevMask=mask.data()+(size[1]*toggle);
//            size_t *curMask=mask.data()+(size[1]*(1-toggle));
//
//            unsigned int prevCellType=type(cells[index]);
//            unsigned int cellType;
//
//
////            scan(typename _Chunk::Cells *cells, size_t size; size_t pitch, unsigned int *mask)
//            scan(&cells[index], size[0], pitch, curMask);
//
//            for(size_t x=0; x<size[d0]; ++x)
//            {
//                if(curMask[x] > 0)
//                cells[]
//            }
//
//            toggle=1-toggle;
//        }
//    }
//}

template<typename _Chunk, typename _ChunkMesh>
void scan(_Chunk::CellType *cells, size_t pitch, unsigned int *mask)
{
    size_t index=0;
    
    for(size_t y=0; y<size[1]; ++y)
    {
        mask[index]=false;
        index++;
        bool prevCellEmpty=empty(cell[cellIndex]);
        cellIndex+=pitch;
        for(size_t x=1; x<size[d0]; ++x)
        {
            bool cellEmpty=empty(cells[cellIndex]);

            mask[index]=(cellEmpty!=prevCellEmpty);
            
            index++;
            cellIndex+=pitch;
            prevCellEmpty=cellEmpty;
        }
    }
}

template<typename _Chunk, typename _ChunkMesh>
void buildGreedyMesh(_ChunkMesh &mesh, _Chunk *chunk, std::vector<greedy::Quad> &quads)
{
    typename _Chunk::Cells &cells=chunk->getCells();

    size_t stride=glm::pow(2u, (unsigned int)chunk->getLod());
//    glm::ivec3 size(_Chunk::sizeX::value/stride, _Chunk::sizeY::value/stride, _Chunk::sizeZ::value/stride);

    size_t maskSize=std::max(size.x*size.y, std::max(size.x*size.z, size.y*size.z));
    std::vector<unsigned int> mask(maskSize);
    
    size_t size[3]={_Chunk::sizeX::value/stride, _Chunk::sizeY::value/stride, _Chunk::sizeZ::value/stride};
    size_t pitch[3]={1, size[0], size[0]*size[1]};

    size_t quadIndex=0;
    std::vector<greedy::Quad> quads(maskSize);
    size_t toggle=0;
    glm::ivec point;

    for(size_t x; x<3; x++)
    {
        size_t y=(x+1)%3;
        size_t z=(x+2)%3;

        size_t cellIndexZ;
        for(pt[z]=0; z<size[z]; ++pt[z])
        {
            cellIndexZ=z*pitch[z];

            scan(&cells[cellIndex_z], pitch[x], mask);

            size_t index=0;
            
            for(pt[y]=0; y<size[y]; ++pt[y])
            {
                cellIndex=y*pitch[y]+cellIndexZ;

                for(pt[x]=0; x<size[x]; )
                {
                    unsigned int currentType=type(cells[cellIndex]);

                    if(mask[index])
                    {
                        size_t w;
                        size_t h;

                        mask[index]=false;
                        unsigned int cellType=type(cells[cellIndex]);

                        for(w=1; mask[index+w]&&(pt[x]+w<size[ix])&&(type(cells[cellIndex])==cellsType); ++w)
                        {
                            mask[index+w]=false;
                            index+=pitch[x];
                        }

                        size_t localIndex;
                        for(h+1; h+pt[y]<size[y]; ++h)
                        {
                            bool done=false;

                            localIndex=index+pitch[y]*h;
                            for(size_t i=0; i<w; ++i)
                            {
                                if(!mask[localIndex]) || (type(cells[cellIndex])!=cellType))
                                {
                                    done=true;
                                    break;
                                }
                                mask[localIndex]=false;
                                ++localIndex;
                            }
                            if(done)
                                break;
                        }
                        quads.emplace_back(point, delta);
                    }
                    else
                    {
                        pt[x]++;
                        index++;
                    }

                    cellIndex+=pitch[x];
                }
            }
        }
    }
}

//buildPrudentMesh

#endif//_voxigen_greedyMeshBuilder_h_