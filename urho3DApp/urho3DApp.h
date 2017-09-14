#ifndef _voxigen_urho3DApp_h_ 
#define _voxigen_urho3DApp_h_

///////////////////////////////////////////////
//force Bullet to be included before DebugNew.h
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/RigidBody.h>
///////////////////////////////////////////////
#include "voxigen/regularGrid.h"
#include "voxigen/cell.h"

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Scene/Scene.h>

#include "WorldComponent.h"

typedef voxigen::RegularGrid<voxigen::Cell, 64, 64, 16> World;

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

    Urho3D::SharedPtr<Urho3D::WorldComponent<World>> m_worldComponent;
    World *m_world;

    unsigned int m_frameCount;
    float m_time;

};

#endif //_voxigen_urho3DApp_h_