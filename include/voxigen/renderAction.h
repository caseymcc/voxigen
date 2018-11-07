#ifndef _voxigen_renderAction_h_
#define _voxigen_renderAction_h_

namespace voxigen
{

enum class RenderAction
{
    Idle,
    RequestRenderer,
    Meshing,
    Updating
};

inline std::string getActionName(RenderAction action)
{
    switch(action)
    {
    case RenderAction::Idle:
        return "Idle";
        break;
    case RenderAction::RequestRenderer:
        return "RequestRenderer";
        break;
    case RenderAction::Meshing:
        return "Meshing";
        break;
    case RenderAction::Updating:
        return "Updating";
        break;
    }
    return "Invalid";
}


}//namespace voxigen

#endif //_voxigen_renderAction_h_