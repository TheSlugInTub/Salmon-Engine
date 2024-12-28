#include <salmon/sprite_animation.h>

void AnimationSys()
{
    for (EntityID ent : SceneView<SpriteAnimation>(engineState.scene))
    {
        auto anim = engineState.scene.Get<SpriteAnimation>(ent);

        if (!anim->playing) { continue; }

        anim->timer -= engineState.deltaTime;

        if (anim->timer <= 0)
        {
            anim->currentSprite++;
            if (anim->currentSprite >= anim->textures.size())
            {
                anim->currentSprite = 0;
            }
            anim->sprite->texture = anim->textures[anim->currentSprite];
            anim->timer = anim->speed;
        }
    }
}

REGISTER_SYSTEM(AnimationSys);
