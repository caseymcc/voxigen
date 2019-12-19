#ifndef _voxigen_handleState_h_
#define _voxigen_handleState_h_

namespace voxigen
{

enum class HandleState
{
    Unknown,
    Cached,
    Init,
    Memory
};

inline std::string getHandleStateName(HandleState state)
{
    switch(state)
    {
    case HandleState::Unknown:
        return "Unknown";
        break;
    case HandleState::Cached:
        return "Cached";
        break;
    case HandleState::Memory:
        return "Memory";
        break;
    }
    return "Invalid";
}

enum class HandleAction
{
    Idle,
    Reading,
    Writing,
    Generating,
    Updating,
    Releasing
};

inline std::string getHandleActionName(HandleAction action)
{
    switch(action)
    {
    case HandleAction::Idle:
        return "Idle";
        break;
    case HandleAction::Reading:
        return "Reading";
        break;
    case HandleAction::Writing:
        return "Writing";
        break;
    case HandleAction::Generating:
        return "Generating";
        break;
    case HandleAction::Updating:
        return "Updating";
        break;
    case HandleAction::Releasing:
        return "Releasing";
        break;
    }
    return "Invalid";
}

} //namespace voxigen

#endif //_voxigen_handleState_h_