#include "plugin.h"
#include "D3DSprite.h"

CD3DSprite::CD3DSprite() {
    D3DXCreateSprite(GetD3DDevice(), &m_pSprite);
}

CD3DSprite::~CD3DSprite() {
    m_pSprite->Flush();
    m_pSprite->Release();
}

HRESULT CD3DSprite::QueryInterface(REFIID iid, LPVOID* ppv) {
    return m_pSprite->QueryInterface(iid, ppv);
}

ULONG CD3DSprite::AddRef() {
    return m_pSprite->AddRef();
}

ULONG CD3DSprite::Release() {
    return m_pSprite->Release();
}

HRESULT CD3DSprite::Begin(DWORD Flags) {
    return m_pSprite->Begin(Flags);
}

HRESULT CD3DSprite::Draw(LPDIRECT3DTEXTURE9 pTexture, CONST RECT* pSrcRect, CONST D3DXVECTOR3* pCenter, CONST D3DXVECTOR3* pPosition, D3DCOLOR Color) {
    return m_pSprite->Draw(pTexture, pSrcRect, pCenter, pPosition, Color);
}

HRESULT CD3DSprite::End() {
    return m_pSprite->End();
}

HRESULT CD3DSprite::GetDevice(LPDIRECT3DDEVICE9* ppDevice) {
    return m_pSprite->GetDevice(ppDevice);
}

HRESULT CD3DSprite::GetTransform(D3DXMATRIX* pTransform) {
    return m_pSprite->GetTransform(pTransform);
}

HRESULT CD3DSprite::SetTransform(CONST D3DXMATRIX* pTransform) {
    return m_pSprite->SetTransform(pTransform);
}

HRESULT CD3DSprite::SetWorldViewRH(CONST D3DXMATRIX* pWorld, CONST D3DXMATRIX* pView) {
    return m_pSprite->SetWorldViewRH(pWorld, pView);
}

HRESULT CD3DSprite::SetWorldViewLH(CONST D3DXMATRIX* pWorld, CONST D3DXMATRIX* pView) {
    return m_pSprite->SetWorldViewLH(pWorld, pView);
}

HRESULT CD3DSprite::Flush() {
    return m_pSprite->Flush();
}

HRESULT CD3DSprite::OnLostDevice() {
    return m_pSprite->OnLostDevice();
}

HRESULT CD3DSprite::OnResetDevice() {
    return m_pSprite->OnResetDevice();
}
