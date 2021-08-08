#pragma once

class CSprite2d;

class CMenuNew {
public:
    CSprite2d* MenuSprites[4];
    float mouseMult;
    CVector2D MousePos;
    CVector2D OldMousePos;

public:
    CMenuNew();
    void Init();
    void Update();
    void SetMouseMult(float m);
    void Shutdown();
    void Process();
    void LimitMousePosition();
};

extern CMenuNew MenuNew;