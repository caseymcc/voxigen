#ifndef _voxigen_urho3DApp_h_ 
#define _voxigen_urho3DApp_h_

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Scene/Scene.h>

class Urho3DApp: public Urho3D::Application
{
public:
    Urho3DApp(Urho3D::Context *context);

    /// Setup before engine initialization. Modifies the engine parameters.
    virtual void Setup();
    /// Setup after engine initialization. Creates the logo, console & debug HUD.
    virtual void Start();
    /// Cleanup after the main loop. Called by Application.
    virtual void Stop();

    void createScene();

    void onBeginFrame(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void onKeyDown(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void onUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void onClosePressed(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

private:
    void CreateConsoleAndDebugHud();

    Urho3D::SharedPtr<Urho3D::Scene> m_scene;
    Urho3D::SharedPtr<Urho3D::Node> m_cameraNode;

    unsigned int m_frameCount;
    float m_time;

};

#endif //_voxigen_urho3DApp_h_