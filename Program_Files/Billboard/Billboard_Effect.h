/*==============================================================================

    Managed Billboard Target Type [Billboard_Target.h]

    Author : Choi HyungJoon

==============================================================================*/
#ifndef BILLBOARD_EFFECT_H
#define BILLBOARD_EFFECT_H
#include "Billboard_Object.h"
#include "Sprite_Animation.h"

class Billboard_Effect : public Billboard_Object
{
public:
    Billboard_Effect()
        : Billboard_Object(-1, { 0,0,0 }, 1.0f, 1.0f)
    {
        m_IsActive = false;
        m_PlayID = -1;
    }
    virtual ~Billboard_Effect()
    {
        if (m_PlayID != -1)
        {
            SpriteAni_DestroyPlayer(m_PlayID);
        }
    }

    void Reset(int patternID, const DirectX::XMFLOAT3& pos, float scale)
    {
        m_Position = pos;
        m_ScaleX = scale;
        m_ScaleY = scale;

        if (m_PlayID != -1) SpriteAni_DestroyPlayer(m_PlayID);
        m_PlayID = SpriteAni_CreatePlayer(patternID);
        Activate(pos);
    }

    virtual void Update(double elapsed_time) override
    {
        // float dt = static_cast<float>(elapsed_time);

        if (!m_IsActive) return;

        if (m_PlayID != -1)
        {
            if (SpriteAni_IsStopped(m_PlayID))
            {
                SpriteAni_DestroyPlayer(m_PlayID);
                m_PlayID = -1;
                Deactivate();
            }
        }
    }

    virtual void Draw() override
    {
        if (!m_IsActive || m_PlayID == -1)
        {
            return;
        }

        if (SpriteAni_IsStopped(m_PlayID))
        {
            return;
        }

        Billboard_Draw_Animation(m_PlayID, m_Position, m_ScaleX, m_ScaleY);
    }

private:
    int m_PlayID = -1;
};

#endif // BILLBOARD_EFFECT_H