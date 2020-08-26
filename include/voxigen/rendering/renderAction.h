#ifndef _voxigen_renderAction_h_
#define _voxigen_renderAction_h_

namespace voxigen
{

enum class RenderAction
{
    Idle,
    HandleBusy, //handle busy (loading, generating, etc...)
    Meshing
};

inline std::string getActionName(RenderAction action)
{
    switch(action)
    {
    case RenderAction::Idle:
        return "Idle";
        break;
    case RenderAction::HandleBusy:
        return "HandleBusy";
        break;
    case RenderAction::Meshing:
        return "Meshing";
        break;
    }
    return "Invalid";
}

enum class MeshState
{
    Invalid,
    Meshing,
    Uploading,
    Ready
};


inline std::string getMeshStateName(MeshState state)
{
    switch(state)
    {
    case MeshState::Invalid:
        return "Invalid";
        break;
    case MeshState::Meshing:
        return "Meshing";
        break;
    case MeshState::Uploading:
        return "Uploading";
        break;
    case MeshState::Ready:
        return "Ready";
        break;
    }
    return "Invalid";
}

}//namespace voxigen

#endif //_voxigen_renderAction_h_