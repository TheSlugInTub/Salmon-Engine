#include <scene_manager.h>
#include <ecs.h>
#include <utils.h>
#include <components.h>
#include <renderer.h>

void SpriteRendererSys()
{
    for (EntityID ent : SceneView<Transform, SpriteRenderer>(*SceneManager::currentScene))
    {
        Renderer::Render(ent, Utils::globalCamera->GetViewMatrix(), Utils::globalCamera->GetProjMatrix());
    }
}

void RigidBody2DSys()
{
    for (EntityID ent : SceneView<Transform, RigidBody2D>(*SceneManager::currentScene))
    {
        auto rigid = SceneManager::currentScene->Get<RigidBody2D>(ent);
        auto trans = SceneManager::currentScene->Get<Transform>(ent);

        if (rigid->start)
        {
            b2BodyDef bodyDef1;
            bodyDef1.type = rigid->type;
            bodyDef1.position.Set(trans->position.x, trans->position.y);
            rigid->body = Utils::globalWorld->CreateBody(&bodyDef1);

            b2PolygonShape dynamicBox1;
            dynamicBox1.SetAsBox(rigid->bodyScale.x, rigid->bodyScale.y);

            b2FixtureDef fixtureDef1;
            fixtureDef1.shape = &dynamicBox1;
            fixtureDef1.density = 1.0f;
            fixtureDef1.friction = 0.8f;

            fixtureDef1.filter.categoryBits = 0x0001;
            fixtureDef1.filter.maskBits = 0xFFFF; // Collide with everything

            rigid->body->CreateFixture(&fixtureDef1);
            rigid->start = false;
        }

        trans->position.x = rigid->body->GetPosition().x;
        trans->position.y = rigid->body->GetPosition().y;    
        trans->rotation.z = rigid->body->GetAngle();

        Renderer::RenderBody(ent, Utils::globalCamera->GetViewMatrix(), Utils::globalCamera->GetProjMatrix());
    }
}

REGISTER_SYSTEM(SpriteRendererSys);
REGISTER_SYSTEM(RigidBody2DSys);