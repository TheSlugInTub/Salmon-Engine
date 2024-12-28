#include <salmon/sprite_animation.h>
#include <salmon/utils.h>

void AnimationSys()
{
    for (EntityID ent : SceneView<SpriteAnimator>(engineState.scene))
    {
        auto anim = engineState.scene.Get<SpriteAnimator>(ent);

        if (!anim->playing || !anim->currentAnim)
        {
            continue;
        }

        anim->timer -= engineState.deltaTime;

        if (anim->timer <= 0)
        {
            anim->currentSprite++;
            if (anim->currentSprite >= anim->currentAnim->textures.size())
            {
                anim->currentSprite = 0;
            }
            anim->sprite->texture = anim->currentAnim->textures[anim->currentSprite];
            anim->timer = anim->currentAnim->speed;
        }
    }
}

REGISTER_SYSTEM(AnimationSys);

void PlaySpriteAnimation(SpriteAnimator* spriteAnim, const std::string& name)
{
    int index = -1;
    for (int i = 0; i < spriteAnim->animations.size(); ++i)
    {
        if (spriteAnim->animations[i].name == name)
        {
            index = i;
        }
    }
    if (index == -1)
    {
        assert("Animation was not found with the specific name you specified in the functon call "
               "of PlaySpriteAnimation, please input a valid animation name or make an animation "
               "with this specific name.");
    }

    spriteAnim->currentAnim = &spriteAnim->animations[index];
    spriteAnim->currentSprite = 0;
    spriteAnim->playing = true;
    spriteAnim->timer = 0;
}
