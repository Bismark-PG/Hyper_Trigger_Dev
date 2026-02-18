/*==============================================================================

	Managed Billboard Target Type [Billboard_Target.h]

	Author : Choi HyungJoon

==============================================================================*/
#ifndef BILLBOARD_TARGET_H
#define BILLBOARD_TARGET_H
#include "Billboard_Object.h"

class Billboard_Target : public Billboard_Object
{
public:
    Billboard_Target(int texID, const DirectX::XMFLOAT3& pos, float scaleX, float scaleY)
        : Billboard_Object(texID, pos, scaleX, scaleY)
    {
    }

    virtual void Update(double elapsed_time) override
    {
        // float dt = static_cast<float>(elapsed_time);
    }

    void OnHit()
    {
        Deactivate();
    }
};

#endif // BILLBOARD_TARGET_H