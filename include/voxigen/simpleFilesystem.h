#ifndef _voxigen_simpleFileSystem_h_
#define _voxigen_simpleFileSystem_h_

#include "voxigen/voxigen_export.h"

#include <string>
#include <vector>

namespace voxigen
{
namespace fs
{

bool VOXIGEN_EXPORT exists(std::string name);
bool VOXIGEN_EXPORT is_directory(std::string name);
bool VOXIGEN_EXPORT create_directory(std::string name);
void VOXIGEN_EXPORT copy_file(std::string src, std::string dest, bool replace);
std::string VOXIGEN_EXPORT extension(std::string file);

std::vector<std::string> VOXIGEN_EXPORT get_directories(std::string directory);

}
}//namespace voxigen::fs

#endif //_voxigen_simpleCamera_h_