#include "imglib/filesystem.h"

#include <boost/filesystem.hpp>

namespace imglib
{

//used to hide boost::filesystem access
std::string getExt(char *filename)
{
    boost::filesystem::path filePath(filename);

    return filePath.extension().string();
}

}//namespace imglib

