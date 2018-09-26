#ifndef _voxigen_regular2DGrid_h_
#define _voxigen_regular2DGrid_h_

#include <vector>
#include <glm/glm.hpp>

namespace voxigen
{

template<typename _Cell>
class Regular2DGrid
{
public:
    typedef _Cell CellType;

//    Regular2DGrid() {};
    Regular2DGrid(size_t width, size_t height):m_size(width, height), m_cells(width*height){}
    Regular2DGrid(size_t width, size_t height, CellType value):m_size(width, height), m_cells(width*height, value) {}

    ~Regular2DGrid() {};

    glm::ivec2 findEmpty(const glm::ivec2 &size);
    bool fill(const glm::ivec2 &index, const glm::ivec2 &size, const CellType &value);

private:
    bool isEmpty(const glm::ivec2 &index, const glm::ivec2 &size);

    std::vector<CellType> m_cells;

    glm::ivec2 m_size;
};


template<typename _Cell>
bool Regular2DGrid<_Cell>::isEmpty(const glm::ivec2 &index, const glm::ivec2 &size)
{
    size_t posX;
    size_t posY=index.y*m_size.x;

    if(index.x+size.x>m_size.x)
        return false;
    if(index.y+size.y>m_size.y)
        return false;

    for(size_t y=0; y<size.y; ++y)
    {
        posX=index.x;
        for(size_t x=0; x<size.x; ++x)
        {
            if(!empty(m_cells[posY+posX]))
                return false;
            ++posX;
        }
        posY+=m_size.x;
    }
    return true;
}

template<typename _Cell>
bool Regular2DGrid<_Cell>::fill(const glm::ivec2 &index, const glm::ivec2 &size, const CellType &value)
{
    size_t posX;
    size_t posY=index.y*m_size.x;

    if(index.x+size.x>m_size.x)
        return false;
    if(index.y+size.y>m_size.y)
        return false;

    for(size_t y=0; y<size.y; ++y)
    {
        posX=index.x;
        for(size_t x=0; x<size.x; ++x)
        {
            m_cells[posY+posX]=value;
            ++posX;
        }
        posY+=m_size.x;
    }
    return true;
}

template<typename _Cell>
glm::ivec2 Regular2DGrid<_Cell>::findEmpty(const glm::ivec2 &size)
{
    glm::ivec2 index={0, 0};
    
    bool valid=false;

    while((index.x<m_size.x) && (index.y<m_size.y))
    {
        size_t posX=index.x;
        size_t posY=index.y*m_size.x;

        if(isEmpty(index, size))
        {
            valid=true;
            break;
        }
        
        if(index.x<index.y)
            index.x++;
        else if(index.x==index.y)
        {
            index.x++;
            index.y=0;
        }
        else
        {
            index.y++;
            if(index.x==index.y)
                index.x=0;
        }
    }

    if(valid)
    {
//        fill(index, size);
        return index;
    }

    return glm::ivec2(-1, -1);
}

}//namespace voxigen

#endif//_voxigen_regular2DGrid_h_
