// このファイルはおいがみ様の MMDPlugin ver0.41 64bit版 開発者用
// https://bowlroll.net/file/135503 を改変したものです。
// ビルドするにはこのファイルと同じフォルダに同梱されている
// MMDPlugin.lib を配置してください。
#pragma once
#include <d3d9.h>
#include <memory>
#include <string>
#include <filesystem>

///#define MMDPLUGIN_D3DX9_HOOK
#ifdef MMDPLUGIN_D3DX9_HOOK
#include <d3dx9.h>
#endif // MMDPLUGIN_D3DX9_HOOK


#define MMD_PLUGIN_API __declspec(dllexport)
#ifdef MAKE_MMD_PLUGIN
# define MMD_DLL_FUNC_API __declspec(dllexport)
#else
# define MMD_DLL_FUNC_API __declspec(dllimport)
# pragma comment(lib,"MMDPlugin/MMDPlugin")
#endif // MAKE_MMD_PLUGIN

#if !defined _MATH_DEFINES_DEFINED
    #define _MATH_DEFINES_DEFINED
    // Definitions of useful mathematical constants
    //
    // Define _USE_MATH_DEFINES before including <math.h> to expose these macro
    // definitions for common math constants.  These are placed under an #ifdef
    // since these commonly-defined names are not part of the C or C++ standards
    #define M_E        2.71828182845904523536   // e
    #define M_LOG2E    1.44269504088896340736   // log2(e)
    #define M_LOG10E   0.434294481903251827651  // log10(e)
    #define M_LN2      0.693147180559945309417  // ln(2)
    #define M_LN10     2.30258509299404568402   // ln(10)
    #define M_PI       3.14159265358979323846   // pi
    #define M_PI_2     1.57079632679489661923   // pi/2
    #define M_PI_4     0.785398163397448309616  // pi/4
    #define M_1_PI     0.318309886183790671538  // 1/pi
    #define M_2_PI     0.636619772367581343076  // 2/pi
    #define M_2_SQRTPI 1.12837916709551257390   // 2/sqrt(pi)
    #define M_SQRT2    1.41421356237309504880   // sqrt(2)
    #define M_SQRT1_2  0.707106781186547524401  // 1/sqrt(2)
#endif

extern "C"
{
  MMD_DLL_FUNC_API int getMMDPluginVersion();

  MMD_DLL_FUNC_API HMODULE getMMDPluginModule();

  // MMDのウィンドウハンドルを返します。
  MMD_DLL_FUNC_API HWND getHWND();

  // 重複しないWM_APP+X値を返します。
  // すでにすべての値を使い切っている場合は-1を返します。
  MMD_DLL_FUNC_API int createWM_APP_ID();

  /// <summary>
  /// [mini, maxi]の区間領域を予約します。すでに使われている場合falseを返します。
  /// </summary>
  /// <param name="mini">WM_APP &lt;= mini &lt;= WM_APP + 0x3FFF </param>
  /// <param name="maxi">WM_APP &lt;= maxi &lt;= WM_APP + 0x3FFF</param>
  /// <returns></returns>
  MMD_DLL_FUNC_API bool reserveWM_APP_ID(int mini, int maxi);
}
#pragma warning( push )
#pragma warning( disable : 4100 )
// ReSharper disable CppParameterNeverUsed

struct MMDPluginDLL1
{
public:

  virtual ~MMDPluginDLL1() {}

  // callback
  // すべて実際の関数が呼ばれる前にコールバックされる
  /*
  // 内部実装
  MMDPluginDLL1::QueryInterface(riid, ppvObj);
  return d3dDevice->QueryInterface(riid, ppvObj);
  */
  virtual void QueryInterface(REFIID riid, void** ppvObj) {}

  virtual void AddRef() {}

  virtual void Release() {}

  virtual void TestCooperativeLevel() {}

  virtual void GetAvailableTextureMem() {}

  virtual void EvictManagedResources() {}

  virtual void GetDirect3D(IDirect3D9** ppD3D9) {}

  virtual void GetDeviceCaps(D3DCAPS9* pCaps) {}

  virtual void GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode) {}

  virtual void GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* pParameters) {}

  virtual void SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) {}

  virtual void SetCursorPosition(int X, int Y, DWORD Flags) {}

  virtual void ShowCursor(BOOL bShow) {}

  virtual void CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain) {}

  virtual void GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) {}

  virtual void GetNumberOfSwapChains() {}

  virtual void Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) {}

  virtual void Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindow, CONST RGNDATA* pDirtyRegion) {}

  virtual void GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) {}

  virtual void GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) {}

  virtual void SetDialogBoxMode(BOOL bEnableDialogs) {}

  virtual void SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) {}

  virtual void GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp) {}

  virtual void CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {}

  virtual void CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) {}

  virtual void CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) {}

  virtual void CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) {}

  virtual void CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) {}

  virtual void CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {}

  virtual void CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {}

  virtual void UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) {}

  virtual void UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) {}

  virtual void GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) {}

  virtual void GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) {}

  virtual void StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) {}

  virtual void ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) {}

  virtual void CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {}

  virtual void SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) {}

  virtual void GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) {}

  virtual void SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) {}

  virtual void GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) {}

  virtual void BeginScene() {}

  virtual void EndScene() {}

  virtual void Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {}

  virtual void SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {}

  virtual void GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) {}

  virtual void MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) {}

  virtual void SetViewport(CONST D3DVIEWPORT9* pViewport) {}

  virtual void GetViewport(D3DVIEWPORT9* pViewport) {}

  virtual void SetMaterial(CONST D3DMATERIAL9* pMaterial) {}

  virtual void GetMaterial(D3DMATERIAL9* pMaterial) {}

  virtual void SetLight(DWORD Index, CONST D3DLIGHT9* pLight) {}

  virtual void GetLight(DWORD Index, D3DLIGHT9* pLight) {}

  virtual void LightEnable(DWORD Index, BOOL Enable) {}

  virtual void GetLightEnable(DWORD Index, BOOL* pEnable) {}

  virtual void SetClipPlane(DWORD Index, CONST float* pPlane) {}

  virtual void GetClipPlane(DWORD Index, float* pPlane) {}

  virtual void SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) {}

  virtual void GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) {}

  virtual void CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) {}

  virtual void BeginStateBlock() {}

  virtual void EndStateBlock(IDirect3DStateBlock9** ppSB) {}

  virtual void SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus) {}

  virtual void GetClipStatus(D3DCLIPSTATUS9* pClipStatus) {}

  virtual void GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) {}

  virtual void SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture) {}

  virtual void GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) {}

  virtual void SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) {}

  virtual void GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) {}

  virtual void SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) {}

  virtual void ValidateDevice(DWORD* pNumPasses) {}

  virtual void SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries) {}

  virtual void GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries) {}

  virtual void SetCurrentTexturePalette(UINT PaletteNumber) {}

  virtual void GetCurrentTexturePalette(UINT* PaletteNumber) {}

  virtual void SetScissorRect(CONST RECT* pRect) {}

  virtual void GetScissorRect(RECT* pRect) {}

  virtual void SetSoftwareVertexProcessing(BOOL bSoftware) {}

  virtual void GetSoftwareVertexProcessing() {}

  virtual void SetNPatchMode(float nSegments) {}

  virtual void GetNPatchMode() {}

  virtual void DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {}

  virtual void DrawIndexedPrimitive(D3DPRIMITIVETYPE, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {}

  virtual void DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {}

  virtual void DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {}

  virtual void ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) {}

  virtual void CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) {}

  virtual void SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) {}

  virtual void GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) {}

  virtual void SetFVF(DWORD FVF) {}

  virtual void GetFVF(DWORD* pFVF) {}

  virtual void CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader) {}

  virtual void SetVertexShader(IDirect3DVertexShader9* pShader) {}

  virtual void GetVertexShader(IDirect3DVertexShader9** ppShader) {}

  virtual void SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {}

  virtual void GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {}

  virtual void SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {}

  virtual void GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {}

  virtual void SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {}

  virtual void GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {}

  virtual void SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) {}

  virtual void GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) {}

  virtual void SetStreamSourceFreq(UINT StreamNumber, UINT Setting) {}

  virtual void GetStreamSourceFreq(UINT StreamNumber, UINT* pSetting) {}

  virtual void SetIndices(IDirect3DIndexBuffer9* pIndexData) {}

  virtual void GetIndices(IDirect3DIndexBuffer9** ppIndexData) {}

  virtual void CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader) {}

  virtual void SetPixelShader(IDirect3DPixelShader9* pShader) {}

  virtual void GetPixelShader(IDirect3DPixelShader9** ppShader) {}

  virtual void SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) {}

  virtual void GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) {}

  virtual void SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) {}

  virtual void GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) {}

  virtual void SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount) {}

  virtual void GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) {}

  virtual void DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo) {}

  virtual void DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo) {}

  virtual void DeletePatch(UINT Handle) {}

  virtual void CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery) {}
};

struct MMDPluginDLL2 : public MMDPluginDLL1
{
public:

  // callback
  // すべて実際の関数が呼ばれ後にコールバックされる
  /*
  // 内部実装
  auto res = d3dDevice->QueryInterface(riid, ppvObj);
  MMDPluginDLL2::PostQueryInterface(riid, ppvObj, res);

  */
  virtual void PostQueryInterface(REFIID riid, void** ppvObj, HRESULT& res) {}

  virtual void PostAddRef(ULONG& res) {}

  virtual void PostRelease(ULONG& res) {}

  virtual void PostTestCooperativeLevel(HRESULT& res) {}

  virtual void PostGetAvailableTextureMem(UINT& res) {}

  virtual void PostEvictManagedResources(HRESULT& res) {}

  virtual void PostGetDirect3D(IDirect3D9** ppD3D9, HRESULT& res) {}

  virtual void PostGetDeviceCaps(D3DCAPS9* pCaps, HRESULT& res) {}

  virtual void PostGetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode, HRESULT& res) {}

  virtual void PostGetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* pParameters, HRESULT& res) {}

  virtual void PostSetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap, HRESULT& res) {}

  virtual void PostSetCursorPosition(int X, int Y, DWORD Flags) {}

  virtual void PostShowCursor(BOOL bShow, BOOL& res) {}

  virtual void PostCreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain, HRESULT& res) {}

  virtual void PostGetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain, HRESULT& res) {}

  virtual void PostGetNumberOfSwapChains(UINT& res) {}

  virtual void PostReset(D3DPRESENT_PARAMETERS* pPresentationParameters, HRESULT& res) {}

  virtual void PostPresent(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindow, CONST RGNDATA* pDirtyRegion, HRESULT& res) {}

  virtual void PostGetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer, HRESULT& res) {}

  virtual void PostGetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus, HRESULT& res) {}

  virtual void PostSetDialogBoxMode(BOOL bEnableDialogs, HRESULT& res) {}

  virtual void PostSetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) {}

  virtual void PostGetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp) {}

  virtual void PostCreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle, HRESULT& res) {}

  virtual void PostCreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle, HRESULT& res) {}

  virtual void PostCreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle, HRESULT& res) {}

  virtual void PostCreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle, HRESULT& res) {}

  virtual void PostCreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle, HRESULT& res) {}

  virtual void PostCreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, HRESULT& res) {}

  virtual void PostCreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, HRESULT& res) {}

  virtual void PostUpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint, HRESULT& res) {}

  virtual void PostUpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture, HRESULT& res) {}

  virtual void PostGetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface, HRESULT& res) {}

  virtual void PostGetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface, HRESULT& res) {}

  virtual void PostStretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter, HRESULT& res) {}

  virtual void PostColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color, HRESULT& res) {}

  virtual void PostCreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, HRESULT& res) {}

  virtual void PostSetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget, HRESULT& res) {}

  virtual void PostGetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget, HRESULT& res) {}

  virtual void PostSetDepthStencilSurface(IDirect3DSurface9* pNewZStencil, HRESULT& res) {}

  virtual void PostGetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface, HRESULT& res) {}

  virtual void PostBeginScene(HRESULT& res) {}

  virtual void PostEndScene(HRESULT& res) {}

  virtual void PostClear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil, HRESULT& res) {}

  virtual void PostSetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix, HRESULT& res) {}

  virtual void PostGetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix, HRESULT& res) {}

  virtual void PostMultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix, HRESULT& res) {}

  virtual void PostSetViewport(CONST D3DVIEWPORT9* pViewport, HRESULT& res) {}

  virtual void PostGetViewport(D3DVIEWPORT9* pViewport, HRESULT& res) {}

  virtual void PostSetMaterial(CONST D3DMATERIAL9* pMaterial, HRESULT& res) {}

  virtual void PostGetMaterial(D3DMATERIAL9* pMaterial, HRESULT& res) {}

  virtual void PostSetLight(DWORD Index, CONST D3DLIGHT9* pLight, HRESULT& res) {}

  virtual void PostGetLight(DWORD Index, D3DLIGHT9* pLight, HRESULT& res) {}

  virtual void PostLightEnable(DWORD Index, BOOL Enable, HRESULT& res) {}

  virtual void PostGetLightEnable(DWORD Index, BOOL* pEnable, HRESULT& res) {}

  virtual void PostSetClipPlane(DWORD Index, CONST float* pPlane, HRESULT& res) {}

  virtual void PostGetClipPlane(DWORD Index, float* pPlane, HRESULT& res) {}

  virtual void PostSetRenderState(D3DRENDERSTATETYPE State, DWORD Value, HRESULT& res) {}

  virtual void PostGetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue, HRESULT& res) {}

  virtual void PostCreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB, HRESULT& res) {}

  virtual void PostBeginStateBlock(HRESULT& res) {}

  virtual void PostEndStateBlock(IDirect3DStateBlock9** ppSB, HRESULT& res) {}

  virtual void PostSetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus, HRESULT& res) {}

  virtual void PostGetClipStatus(D3DCLIPSTATUS9* pClipStatus, HRESULT& res) {}

  virtual void PostGetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture, HRESULT& res) {}

  virtual void PostSetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture, HRESULT& res) {}

  virtual void PostGetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue, HRESULT& res) {}

  virtual void PostSetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value, HRESULT& res) {}

  virtual void PostGetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue, HRESULT& res) {}

  virtual void PostSetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value, HRESULT& res) {}

  virtual void PostValidateDevice(DWORD* pNumPasses, HRESULT& res) {}

  virtual void PostSetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries, HRESULT& res) {}

  virtual void PostGetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries, HRESULT& res) {}

  virtual void PostSetCurrentTexturePalette(UINT PaletteNumber, HRESULT& res) {}

  virtual void PostGetCurrentTexturePalette(UINT* PaletteNumber, HRESULT& res) {}

  virtual void PostSetScissorRect(CONST RECT* pRect, HRESULT& res) {}

  virtual void PostGetScissorRect(RECT* pRect, HRESULT& res) {}

  virtual void PostSetSoftwareVertexProcessing(BOOL bSoftware, HRESULT& res) {}

  virtual void PostGetSoftwareVertexProcessing(BOOL& res) {}

  virtual void PostSetNPatchMode(float nSegments, HRESULT& res) {}

  virtual void PostGetNPatchMode(float& res) {}

  virtual void PostDrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount, HRESULT& res) {}

  virtual void PostDrawIndexedPrimitive(D3DPRIMITIVETYPE, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount, HRESULT& res) {}

  virtual void PostDrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride, HRESULT& res) {}

  virtual void PostDrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride, HRESULT& res) {}

  virtual void PostProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags, HRESULT& res) {}

  virtual void PostCreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl, HRESULT& res) {}

  virtual void PostSetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl, HRESULT& res) {}

  virtual void PostGetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl, HRESULT& res) {}

  virtual void PostSetFVF(DWORD FVF, HRESULT& res) {}

  virtual void PostGetFVF(DWORD* pFVF, HRESULT& res) {}

  virtual void PostCreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader, HRESULT& res) {}

  virtual void PostSetVertexShader(IDirect3DVertexShader9* pShader, HRESULT& res) {}

  virtual void PostGetVertexShader(IDirect3DVertexShader9** ppShader, HRESULT& res) {}

  virtual void PostSetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount, HRESULT& res) {}

  virtual void PostGetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount, HRESULT& res) {}

  virtual void PostSetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount, HRESULT& res) {}

  virtual void PostGetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount, HRESULT& res) {}

  virtual void PostSetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount, HRESULT& res) {}

  virtual void PostGetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount, HRESULT& res) {}

  virtual void PostSetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride, HRESULT& res) {}

  virtual void PostGetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride, HRESULT& res) {}

  virtual void PostSetStreamSourceFreq(UINT StreamNumber, UINT Setting, HRESULT& res) {}

  virtual void PostGetStreamSourceFreq(UINT StreamNumber, UINT* pSetting, HRESULT& res) {}

  virtual void PostSetIndices(IDirect3DIndexBuffer9* pIndexData, HRESULT& res) {}

  virtual void PostGetIndices(IDirect3DIndexBuffer9** ppIndexData, HRESULT& res) {}

  virtual void PostCreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader, HRESULT& res) {}

  virtual void PostSetPixelShader(IDirect3DPixelShader9* pShader, HRESULT& res) {}

  virtual void PostGetPixelShader(IDirect3DPixelShader9** ppShader, HRESULT& res) {}

  virtual void PostSetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount, HRESULT& res) {}

  virtual void PostGetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount, HRESULT& res) {}

  virtual void PostSetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount, HRESULT& res) {}

  virtual void PostGetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount, HRESULT& res) {}

  virtual void PostSetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT BoolCount, HRESULT& res) {}

  virtual void PostGetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount, HRESULT& res) {}

  virtual void PostDrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo, HRESULT& res) {}

  virtual void PostDrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo, HRESULT& res) {}

  virtual void PostDeletePatch(UINT Handle, HRESULT& res) {}

  virtual void PostCreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery, HRESULT& res) {}
};

struct MMDPluginDLL3 : public MMDPluginDLL2
{
  virtual const char* getPluginTitle() const = 0;


  /// <summary>
  /// すべてのDLLのオブジェクトを作成し終わったときに呼ばれる
  /// </summary>
  virtual void start() {}

  /// <summary>
  /// MMDが終了し、DLLのオブジェクトを解放する前に呼ばれる。
  /// </summary>
  virtual void stop() {}

  /// <summary>
  /// SetWindowsHookEx(WH_CALLWNDPROC)でフックしたプロシージャです。
  /// </summary>
  /// <param name="param"></param>
  virtual void WndProc(CWPSTRUCT* param) {}

  /// <summary>
  /// SetWindowsHookEx(WH_MSGFILTER)でフックしたプロシージャです。
  /// </summary>
  /// <param name="param"></param>
  virtual void MsgProc(int code, MSG* param) {}

  /// <summary>
  /// SetWindowsHookEx(WH_MOUSE)でフックしたプロシージャです。
  /// </summary>
  /// <param name="param"></param>
  virtual void MouseProc(WPARAM wParam, MOUSEHOOKSTRUCT* param) {}

  /// <summary>
  /// SetWindowsHookEx(WH_GETMESSAGE)でフックしたプロシージャです。
  /// </summary>
  /// <param name="param"></param>
  virtual void GetMsgProc(int code, MSG* param) {}

  /// <summary>
  /// SetWindowsHookEx(WH_KEYBOARD)でフックしたプロシージャです。
  /// </summary>
  /// <param name="param"></param>
  virtual void KeyBoardProc(WPARAM wParam, LPARAM lParam) {}

  /// <summary>
  /// MMD側の呼び出しが制御できるプロシージャ
  /// </summary>
  /// <param name="hwnd"></param>
  /// <param name="uMsg"></param>
  /// <param name="wParam"></param>
  /// <param name="lParam"></param>
  /// <returns>
  /// false : MMDのプロシージャも呼ばれます。LRESULTは無視されます
  /// true  : MMDやその他のプラグインのプロシージャは呼ばれません。LRESULTが全体のプロシージャの戻り値として返されます。
  /// </returns>
  virtual std::pair<bool, LRESULT> WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return { false,0 }; }
};

struct MMDPluginDLL4 : public MMDPluginDLL3
{
#ifdef MMDPLUGIN_D3DX9_HOOK

  virtual void D3DXCreateEffectFromFileExW(
    LPDIRECT3DDEVICE9&		    pDevice,
    LPCWSTR&						 pSrcFile,
    CONST D3DXMACRO*&				pDefines,
    LPD3DXINCLUDE&				pInclude,
    LPCSTR&						  pSkipConstants,
    DWORD&						Flags,
    LPD3DXEFFECTPOOL&				pPool,
    LPD3DXEFFECT*&				ppEffect,
    LPD3DXBUFFER*&				ppCompilationErrors)
  {
  }

  virtual void PostD3DXCreateEffectFromFileExW(
    LPDIRECT3DDEVICE9&		    pDevice,
    LPCWSTR&						 pSrcFile,
    CONST D3DXMACRO*&				pDefines,
    LPD3DXINCLUDE&				pInclude,
    LPCSTR&						  pSkipConstants,
    DWORD&						Flags,
    LPD3DXEFFECTPOOL&				pPool,
    LPD3DXEFFECT*&				ppEffect,
    LPD3DXBUFFER*&				ppCompilationErrors, HRESULT& res)
  {
  }
#endif // MMDPLUGIN_D3DX9_HOOK
};

#ifdef MMDPLUGIN_D3DX9_HOOK
namespace mmp
{
  using HookEffectQueryInterfacePF = HRESULT(WINAPI *)(ID3DXEffect* effect, REFIID iid, LPVOID *ppv);
  MMD_DLL_FUNC_API HookEffectQueryInterfacePF HookEffectQueryInterface(ID3DXEffect* effect, HookEffectQueryInterfacePF f);
  using HookEffectAddRefPF = ULONG(WINAPI *)(ID3DXEffect* effect);
  MMD_DLL_FUNC_API HookEffectAddRefPF HookEffectAddRef(ID3DXEffect* effect, HookEffectAddRefPF f);
  using HookEffectReleasePF = ULONG(WINAPI *)(ID3DXEffect* effect);
  MMD_DLL_FUNC_API HookEffectReleasePF HookEffectRelease(ID3DXEffect* effect, HookEffectReleasePF f);
  using HookEffectGetDescPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXEFFECT_DESC* pDesc);
  MMD_DLL_FUNC_API HookEffectGetDescPF HookEffectGetDesc(ID3DXEffect* effect, HookEffectGetDescPF f);
  using HookEffectGetParameterDescPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, D3DXPARAMETER_DESC* pDesc);
  MMD_DLL_FUNC_API HookEffectGetParameterDescPF HookEffectGetParameterDesc(ID3DXEffect* effect, HookEffectGetParameterDescPF f);
  using HookEffectGetTechniqueDescPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hTechnique, D3DXTECHNIQUE_DESC* pDesc);
  MMD_DLL_FUNC_API HookEffectGetTechniqueDescPF HookEffectGetTechniqueDesc(ID3DXEffect* effect, HookEffectGetTechniqueDescPF f);
  using HookEffectGetPassDescPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hPass, D3DXPASS_DESC* pDesc);
  MMD_DLL_FUNC_API HookEffectGetPassDescPF HookEffectGetPassDesc(ID3DXEffect* effect, HookEffectGetPassDescPF f);
  using HookEffectGetFunctionDescPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hShader, D3DXFUNCTION_DESC* pDesc);
  MMD_DLL_FUNC_API HookEffectGetFunctionDescPF HookEffectGetFunctionDesc(ID3DXEffect* effect, HookEffectGetFunctionDescPF f);
  using HookEffectGetParameterPF = D3DXHANDLE(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, UINT Index);
  MMD_DLL_FUNC_API HookEffectGetParameterPF HookEffectGetParameter(ID3DXEffect* effect, HookEffectGetParameterPF f);
  using HookEffectGetParameterByNamePF = D3DXHANDLE(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, LPCSTR pName);
  MMD_DLL_FUNC_API HookEffectGetParameterByNamePF HookEffectGetParameterByName(ID3DXEffect* effect, HookEffectGetParameterByNamePF f);
  using HookEffectGetParameterBySemanticPF = D3DXHANDLE(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, LPCSTR pSemantic);
  MMD_DLL_FUNC_API HookEffectGetParameterBySemanticPF HookEffectGetParameterBySemantic(ID3DXEffect* effect, HookEffectGetParameterBySemanticPF f);
  using HookEffectGetParameterElementPF = D3DXHANDLE(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, UINT Index);
  MMD_DLL_FUNC_API HookEffectGetParameterElementPF HookEffectGetParameterElement(ID3DXEffect* effect, HookEffectGetParameterElementPF f);
  using HookEffectGetTechniquePF = D3DXHANDLE(WINAPI *)(ID3DXEffect* effect, UINT Index);
  MMD_DLL_FUNC_API HookEffectGetTechniquePF HookEffectGetTechnique(ID3DXEffect* effect, HookEffectGetTechniquePF f);
  using HookEffectGetTechniqueByNamePF = D3DXHANDLE(WINAPI *)(ID3DXEffect* effect, LPCSTR pName);
  MMD_DLL_FUNC_API HookEffectGetTechniqueByNamePF HookEffectGetTechniqueByName(ID3DXEffect* effect, HookEffectGetTechniqueByNamePF f);
  using HookEffectGetPassPF = D3DXHANDLE(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hTechnique, UINT Index);
  MMD_DLL_FUNC_API HookEffectGetPassPF HookEffectGetPass(ID3DXEffect* effect, HookEffectGetPassPF f);
  using HookEffectGetPassByNamePF = D3DXHANDLE(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hTechnique, LPCSTR pName);
  MMD_DLL_FUNC_API HookEffectGetPassByNamePF HookEffectGetPassByName(ID3DXEffect* effect, HookEffectGetPassByNamePF f);
  using HookEffectGetFunctionPF = D3DXHANDLE(WINAPI *)(ID3DXEffect* effect, UINT Index);
  MMD_DLL_FUNC_API HookEffectGetFunctionPF HookEffectGetFunction(ID3DXEffect* effect, HookEffectGetFunctionPF f);
  using HookEffectGetFunctionByNamePF = D3DXHANDLE(WINAPI *)(ID3DXEffect* effect, LPCSTR pName);
  MMD_DLL_FUNC_API HookEffectGetFunctionByNamePF HookEffectGetFunctionByName(ID3DXEffect* effect, HookEffectGetFunctionByNamePF f);
  using HookEffectGetAnnotationPF = D3DXHANDLE(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hObject, UINT Index);
  MMD_DLL_FUNC_API HookEffectGetAnnotationPF HookEffectGetAnnotation(ID3DXEffect* effect, HookEffectGetAnnotationPF f);
  using HookEffectGetAnnotationByNamePF = D3DXHANDLE(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hObject, LPCSTR pName);
  MMD_DLL_FUNC_API HookEffectGetAnnotationByNamePF HookEffectGetAnnotationByName(ID3DXEffect* effect, HookEffectGetAnnotationByNamePF f);
  using HookEffectSetValuePF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, LPCVOID pData, UINT Bytes);
  MMD_DLL_FUNC_API HookEffectSetValuePF HookEffectSetValue(ID3DXEffect* effect, HookEffectSetValuePF f);
  using HookEffectGetValuePF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, LPVOID pData, UINT Bytes);
  MMD_DLL_FUNC_API HookEffectGetValuePF HookEffectGetValue(ID3DXEffect* effect, HookEffectGetValuePF f);
  using HookEffectSetBoolPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, BOOL b);
  MMD_DLL_FUNC_API HookEffectSetBoolPF HookEffectSetBool(ID3DXEffect* effect, HookEffectSetBoolPF f);
  using HookEffectGetBoolPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, BOOL* pb);
  MMD_DLL_FUNC_API HookEffectGetBoolPF HookEffectGetBool(ID3DXEffect* effect, HookEffectGetBoolPF f);
  using HookEffectSetBoolArrayPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, CONST BOOL* pb, UINT Count);
  MMD_DLL_FUNC_API HookEffectSetBoolArrayPF HookEffectSetBoolArray(ID3DXEffect* effect, HookEffectSetBoolArrayPF f);
  using HookEffectGetBoolArrayPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, BOOL* pb, UINT Count);
  MMD_DLL_FUNC_API HookEffectGetBoolArrayPF HookEffectGetBoolArray(ID3DXEffect* effect, HookEffectGetBoolArrayPF f);
  using HookEffectSetIntPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, INT n);
  MMD_DLL_FUNC_API HookEffectSetIntPF HookEffectSetInt(ID3DXEffect* effect, HookEffectSetIntPF f);
  using HookEffectGetIntPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, INT* pn);
  MMD_DLL_FUNC_API HookEffectGetIntPF HookEffectGetInt(ID3DXEffect* effect, HookEffectGetIntPF f);
  using HookEffectSetIntArrayPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, CONST INT* pn, UINT Count);
  MMD_DLL_FUNC_API HookEffectSetIntArrayPF HookEffectSetIntArray(ID3DXEffect* effect, HookEffectSetIntArrayPF f);
  using HookEffectGetIntArrayPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, INT* pn, UINT Count);
  MMD_DLL_FUNC_API HookEffectGetIntArrayPF HookEffectGetIntArray(ID3DXEffect* effect, HookEffectGetIntArrayPF f);
  using HookEffectSetFloatPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, FLOAT f);
  MMD_DLL_FUNC_API HookEffectSetFloatPF HookEffectSetFloat(ID3DXEffect* effect, HookEffectSetFloatPF f);
  using HookEffectGetFloatPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, FLOAT* pf);
  MMD_DLL_FUNC_API HookEffectGetFloatPF HookEffectGetFloat(ID3DXEffect* effect, HookEffectGetFloatPF f);
  using HookEffectSetFloatArrayPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, CONST FLOAT* pf, UINT Count);
  MMD_DLL_FUNC_API HookEffectSetFloatArrayPF HookEffectSetFloatArray(ID3DXEffect* effect, HookEffectSetFloatArrayPF f);
  using HookEffectGetFloatArrayPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, FLOAT* pf, UINT Count);
  MMD_DLL_FUNC_API HookEffectGetFloatArrayPF HookEffectGetFloatArray(ID3DXEffect* effect, HookEffectGetFloatArrayPF f);
  using HookEffectSetVectorPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, CONST D3DXVECTOR4* pVector);
  MMD_DLL_FUNC_API HookEffectSetVectorPF HookEffectSetVector(ID3DXEffect* effect, HookEffectSetVectorPF f);
  using HookEffectGetVectorPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, D3DXVECTOR4* pVector);
  MMD_DLL_FUNC_API HookEffectGetVectorPF HookEffectGetVector(ID3DXEffect* effect, HookEffectGetVectorPF f);
  using HookEffectSetVectorArrayPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, CONST D3DXVECTOR4* pVector, UINT Count);
  MMD_DLL_FUNC_API HookEffectSetVectorArrayPF HookEffectSetVectorArray(ID3DXEffect* effect, HookEffectSetVectorArrayPF f);
  using HookEffectGetVectorArrayPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, D3DXVECTOR4* pVector, UINT Count);
  MMD_DLL_FUNC_API HookEffectGetVectorArrayPF HookEffectGetVectorArray(ID3DXEffect* effect, HookEffectGetVectorArrayPF f);
  using HookEffectSetMatrixPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix);
  MMD_DLL_FUNC_API HookEffectSetMatrixPF HookEffectSetMatrix(ID3DXEffect* effect, HookEffectSetMatrixPF f);
  using HookEffectGetMatrixPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, D3DXMATRIX* pMatrix);
  MMD_DLL_FUNC_API HookEffectGetMatrixPF HookEffectGetMatrix(ID3DXEffect* effect, HookEffectGetMatrixPF f);
  using HookEffectSetMatrixArrayPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix, UINT Count);
  MMD_DLL_FUNC_API HookEffectSetMatrixArrayPF HookEffectSetMatrixArray(ID3DXEffect* effect, HookEffectSetMatrixArrayPF f);
  using HookEffectGetMatrixArrayPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, D3DXMATRIX* pMatrix, UINT Count);
  MMD_DLL_FUNC_API HookEffectGetMatrixArrayPF HookEffectGetMatrixArray(ID3DXEffect* effect, HookEffectGetMatrixArrayPF f);
  using HookEffectSetMatrixPointerArrayPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, CONST D3DXMATRIX** ppMatrix, UINT Count);
  MMD_DLL_FUNC_API HookEffectSetMatrixPointerArrayPF HookEffectSetMatrixPointerArray(ID3DXEffect* effect, HookEffectSetMatrixPointerArrayPF f);
  using HookEffectGetMatrixPointerArrayPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, D3DXMATRIX** ppMatrix, UINT Count);
  MMD_DLL_FUNC_API HookEffectGetMatrixPointerArrayPF HookEffectGetMatrixPointerArray(ID3DXEffect* effect, HookEffectGetMatrixPointerArrayPF f);
  using HookEffectSetMatrixTransposePF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix);
  MMD_DLL_FUNC_API HookEffectSetMatrixTransposePF HookEffectSetMatrixTranspose(ID3DXEffect* effect, HookEffectSetMatrixTransposePF f);
  using HookEffectGetMatrixTransposePF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, D3DXMATRIX* pMatrix);
  MMD_DLL_FUNC_API HookEffectGetMatrixTransposePF HookEffectGetMatrixTranspose(ID3DXEffect* effect, HookEffectGetMatrixTransposePF f);
  using HookEffectSetMatrixTransposeArrayPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix, UINT Count);
  MMD_DLL_FUNC_API HookEffectSetMatrixTransposeArrayPF HookEffectSetMatrixTransposeArray(ID3DXEffect* effect, HookEffectSetMatrixTransposeArrayPF f);
  using HookEffectGetMatrixTransposeArrayPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, D3DXMATRIX* pMatrix, UINT Count);
  MMD_DLL_FUNC_API HookEffectGetMatrixTransposeArrayPF HookEffectGetMatrixTransposeArray(ID3DXEffect* effect, HookEffectGetMatrixTransposeArrayPF f);
  using HookEffectSetMatrixTransposePointerArrayPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, CONST D3DXMATRIX** ppMatrix, UINT Count);
  MMD_DLL_FUNC_API HookEffectSetMatrixTransposePointerArrayPF HookEffectSetMatrixTransposePointerArray(ID3DXEffect* effect, HookEffectSetMatrixTransposePointerArrayPF f);
  using HookEffectGetMatrixTransposePointerArrayPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, D3DXMATRIX** ppMatrix, UINT Count);
  MMD_DLL_FUNC_API HookEffectGetMatrixTransposePointerArrayPF HookEffectGetMatrixTransposePointerArray(ID3DXEffect* effect, HookEffectGetMatrixTransposePointerArrayPF f);
  using HookEffectSetStringPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, LPCSTR pString);
  MMD_DLL_FUNC_API HookEffectSetStringPF HookEffectSetString(ID3DXEffect* effect, HookEffectSetStringPF f);
  using HookEffectGetStringPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, LPCSTR* ppString);
  MMD_DLL_FUNC_API HookEffectGetStringPF HookEffectGetString(ID3DXEffect* effect, HookEffectGetStringPF f);
  using HookEffectSetTexturePF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, LPDIRECT3DBASETEXTURE9 pTexture);
  MMD_DLL_FUNC_API HookEffectSetTexturePF HookEffectSetTexture(ID3DXEffect* effect, HookEffectSetTexturePF f);
  using HookEffectGetTexturePF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, LPDIRECT3DBASETEXTURE9 *ppTexture);
  MMD_DLL_FUNC_API HookEffectGetTexturePF HookEffectGetTexture(ID3DXEffect* effect, HookEffectGetTexturePF f);
  using HookEffectGetPixelShaderPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, LPDIRECT3DPIXELSHADER9 *ppPShader);
  MMD_DLL_FUNC_API HookEffectGetPixelShaderPF HookEffectGetPixelShader(ID3DXEffect* effect, HookEffectGetPixelShaderPF f);
  using HookEffectGetVertexShaderPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, LPDIRECT3DVERTEXSHADER9 *ppVShader);
  MMD_DLL_FUNC_API HookEffectGetVertexShaderPF HookEffectGetVertexShader(ID3DXEffect* effect, HookEffectGetVertexShaderPF f);
  using HookEffectSetArrayRangePF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, UINT uStart, UINT uEnd);
  MMD_DLL_FUNC_API HookEffectSetArrayRangePF HookEffectSetArrayRange(ID3DXEffect* effect, HookEffectSetArrayRangePF f);
  using HookEffectGetPoolPF = HRESULT(WINAPI *)(ID3DXEffect* effect, LPD3DXEFFECTPOOL* ppPool);
  MMD_DLL_FUNC_API HookEffectGetPoolPF HookEffectGetPool(ID3DXEffect* effect, HookEffectGetPoolPF f);
  using HookEffectSetTechniquePF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hTechnique);
  MMD_DLL_FUNC_API HookEffectSetTechniquePF HookEffectSetTechnique(ID3DXEffect* effect, HookEffectSetTechniquePF f);
  using HookEffectGetCurrentTechniquePF = D3DXHANDLE(WINAPI *)(ID3DXEffect* effect);
  MMD_DLL_FUNC_API HookEffectGetCurrentTechniquePF HookEffectGetCurrentTechnique(ID3DXEffect* effect, HookEffectGetCurrentTechniquePF f);
  using HookEffectValidateTechniquePF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hTechnique);
  MMD_DLL_FUNC_API HookEffectValidateTechniquePF HookEffectValidateTechnique(ID3DXEffect* effect, HookEffectValidateTechniquePF f);
  using HookEffectFindNextValidTechniquePF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hTechnique, D3DXHANDLE *pTechnique);
  MMD_DLL_FUNC_API HookEffectFindNextValidTechniquePF HookEffectFindNextValidTechnique(ID3DXEffect* effect, HookEffectFindNextValidTechniquePF f);
  using HookEffectIsParameterUsedPF = BOOL(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, D3DXHANDLE hTechnique);
  MMD_DLL_FUNC_API HookEffectIsParameterUsedPF HookEffectIsParameterUsed(ID3DXEffect* effect, HookEffectIsParameterUsedPF f);
  using HookEffectBeginPF = HRESULT(WINAPI *)(ID3DXEffect* effect, UINT *pPasses, DWORD Flags);
  MMD_DLL_FUNC_API HookEffectBeginPF HookEffectBegin(ID3DXEffect* effect, HookEffectBeginPF f);
  using HookEffectBeginPassPF = HRESULT(WINAPI *)(ID3DXEffect* effect, UINT Pass);
  MMD_DLL_FUNC_API HookEffectBeginPassPF HookEffectBeginPass(ID3DXEffect* effect, HookEffectBeginPassPF f);
  using HookEffectCommitChangesPF = HRESULT(WINAPI *)(ID3DXEffect* effect);
  MMD_DLL_FUNC_API HookEffectCommitChangesPF HookEffectCommitChanges(ID3DXEffect* effect, HookEffectCommitChangesPF f);
  using HookEffectEndPassPF = HRESULT(WINAPI *)(ID3DXEffect* effect);
  MMD_DLL_FUNC_API HookEffectEndPassPF HookEffectEndPass(ID3DXEffect* effect, HookEffectEndPassPF f);
  using HookEffectEndPF = HRESULT(WINAPI *)(ID3DXEffect* effect);
  MMD_DLL_FUNC_API HookEffectEndPF HookEffectEnd(ID3DXEffect* effect, HookEffectEndPF f);
  using HookEffectGetDevicePF = HRESULT(WINAPI *)(ID3DXEffect* effect, LPDIRECT3DDEVICE9* ppDevice);
  MMD_DLL_FUNC_API HookEffectGetDevicePF HookEffectGetDevice(ID3DXEffect* effect, HookEffectGetDevicePF f);
  using HookEffectOnLostDevicePF = HRESULT(WINAPI *)(ID3DXEffect* effect);
  MMD_DLL_FUNC_API HookEffectOnLostDevicePF HookEffectOnLostDevice(ID3DXEffect* effect, HookEffectOnLostDevicePF f);
  using HookEffectOnResetDevicePF = HRESULT(WINAPI *)(ID3DXEffect* effect);
  MMD_DLL_FUNC_API HookEffectOnResetDevicePF HookEffectOnResetDevice(ID3DXEffect* effect, HookEffectOnResetDevicePF f);
  using HookEffectSetStateManagerPF = HRESULT(WINAPI *)(ID3DXEffect* effect, LPD3DXEFFECTSTATEMANAGER pManager);
  MMD_DLL_FUNC_API HookEffectSetStateManagerPF HookEffectSetStateManager(ID3DXEffect* effect, HookEffectSetStateManagerPF f);
  using HookEffectGetStateManagerPF = HRESULT(WINAPI *)(ID3DXEffect* effect, LPD3DXEFFECTSTATEMANAGER *ppManager);
  MMD_DLL_FUNC_API HookEffectGetStateManagerPF HookEffectGetStateManager(ID3DXEffect* effect, HookEffectGetStateManagerPF f);
  using HookEffectBeginParameterBlockPF = HRESULT(WINAPI *)(ID3DXEffect* effect);
  MMD_DLL_FUNC_API HookEffectBeginParameterBlockPF HookEffectBeginParameterBlock(ID3DXEffect* effect, HookEffectBeginParameterBlockPF f);
  using HookEffectEndParameterBlockPF = D3DXHANDLE(WINAPI *)(ID3DXEffect* effect);
  MMD_DLL_FUNC_API HookEffectEndParameterBlockPF HookEffectEndParameterBlock(ID3DXEffect* effect, HookEffectEndParameterBlockPF f);
  using HookEffectApplyParameterBlockPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameterBlock);
  MMD_DLL_FUNC_API HookEffectApplyParameterBlockPF HookEffectApplyParameterBlock(ID3DXEffect* effect, HookEffectApplyParameterBlockPF f);
  using HookEffectDeleteParameterBlockPF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameterBlock);
  MMD_DLL_FUNC_API HookEffectDeleteParameterBlockPF HookEffectDeleteParameterBlock(ID3DXEffect* effect, HookEffectDeleteParameterBlockPF f);
  using HookEffectCloneEffectPF = HRESULT(WINAPI *)(ID3DXEffect* effect, LPDIRECT3DDEVICE9 pDevice, LPD3DXEFFECT* ppEffect);
  MMD_DLL_FUNC_API HookEffectCloneEffectPF HookEffectCloneEffect(ID3DXEffect* effect, HookEffectCloneEffectPF f);
  using HookEffectSetRawValuePF = HRESULT(WINAPI *)(ID3DXEffect* effect, D3DXHANDLE hParameter, LPCVOID pData, UINT ByteOffset, UINT Bytes);
  MMD_DLL_FUNC_API HookEffectSetRawValuePF HookEffectSetRawValue(ID3DXEffect* effect, HookEffectSetRawValuePF f);

}
#endif // MMDPLUGIN_D3DX9_HOOK

// ReSharper restore CppParameterNeverUsed
#pragma warning( pop )

namespace mmp
{
  namespace detail
  {
    struct IFuncBinderImpl
    {
      MMD_DLL_FUNC_API static void addPrev(std::shared_ptr<IFuncBinderImpl> impl, void* addr, void* new_func);
      MMD_DLL_FUNC_API static void deleteFunc(void* new_func);
    protected:
      IFuncBinderImpl() = default;
      virtual ~IFuncBinderImpl() = default;

      virtual void add_prev(std::shared_ptr<IFuncBinderImpl> prev) = 0;
    };


    namespace get_call_type
    {
      template<typename R, typename... ArgTypes>
      static auto args(R(__vectorcall*)(ArgTypes ...)) -> void(*)(ArgTypes ...);

      template<typename R, typename... ArgTypes>
      static auto args(R(__stdcall*)(ArgTypes ...)) -> void(*)(ArgTypes ...);

      template<typename R, typename... ArgTypes>
      static auto args(R(__cdecl*)(ArgTypes ...)) -> void(*)(ArgTypes ...);

      template<typename R, typename... ArgTypes>
      static auto args(R(__thiscall*)(ArgTypes ...)) -> void(*)(ArgTypes ...);

      template<class F>
      using getArgTypeList = decltype(args(std::declval<F>()));
    }


    template<class Result, class Func, class... Args>
    struct FuncBinderImpl : IFuncBinderImpl, std::enable_shared_from_this<FuncBinderImpl<Result, Func, Args...>>
    {
      FuncBinderImpl(std::string module_name, std::string func_name, Func new_func)
        : module_name_(module_name), func_name_(func_name), new_func_(new_func)
      {
        MMD_DLL_FUNC_API void* RewriteFunction(const char* szRewriteModuleName, const char* szRewriteFunctionName, void* pRewriteFunctionPointer, int ordinal);
        oridinal_func_ = static_cast<decltype(oridinal_func_)>(RewriteFunction(module_name.c_str(), func_name.c_str(), new_func, -1));
      }

      std::string module_name_, func_name_;
      Func oridinal_func_, new_func_;

      Result operator()(Args ... args) { return oridinal_func_(args...); }

      void add_prev(std::shared_ptr<IFuncBinderImpl> p) override
      {
        prev_ = std::dynamic_pointer_cast<FuncBinderImpl>(p);
      }

      void reset()
      {
        MMD_DLL_FUNC_API void* RewriteFunction(const char* szRewriteModuleName, const char* szRewriteFunctionName, void* pRewriteFunctionPointer, int ordinal);
        RewriteFunction(module_name_.c_str(), func_name_.c_str(), oridinal_func_, -1);
        if ( prev_ )
        {
          prev_->oridinal_func_ = oridinal_func_;
        }
      }

      ~FuncBinderImpl()
      {
        reset();
      }

      std::shared_ptr<FuncBinderImpl<Result, Func, Args...>> prev_;
    };

    template<class Func, class F>
    class WinAPIHookerImpl;

    template<class Func, class...Args>
    class WinAPIHookerImpl<Func, void(*)(Args ...)>
    {
    public:
      WinAPIHookerImpl() = default;
      WinAPIHookerImpl(const WinAPIHookerImpl&) = delete;

      ~WinAPIHookerImpl()
      {
        reset();
      }

      using Result = std::invoke_result_t<Func(Args ...)>;

      bool hook(const char* module_name, const char* func_name, Func new_func)
      {
        is_init = true;
        impl_ = std::make_shared<FuncBinderImpl<Result, Func, Args...>>(module_name, func_name, new_func);
        IFuncBinderImpl::addPrev(impl_, impl_->oridinal_func_, new_func);
        return true;
      }

      void reset()
      {
        if ( is_init == false || !impl_ ) return;
        IFuncBinderImpl::deleteFunc(impl_->new_func_);
        impl_->reset();
        is_init = false;
      }

      Result operator()(Args ... args) { return (*impl_)(args...); }

    private:
      bool is_init = true;
      std::shared_ptr<FuncBinderImpl<Result, Func, Args...>> impl_;
    };
  }

  template<class T>
  using WinAPIHooker = detail::WinAPIHookerImpl<T, detail::get_call_type::getArgTypeList<T>>;
}

namespace mmp
{
  constexpr float eps = 1e-7f;

  inline bool compare(float a, float b)
  {
    return a - eps <= b && b <= a + eps;
  }

  struct Float4
  {
    float x, y, z, w;

	Float4() {}

	Float4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}

    bool operator==(const Float4& o) const
    {
      return compare(x, o.x) && compare(y, o.y) && compare(z, o.z) && compare(w, o.w);
    }

    bool operator!=(const Float4& o) const
    {
      return !(*this == o);
    }

	Float4& operator += (const Float4& o)
	{
		x += o.x;	y += o.y;	z += o.z;	w += o.w;
		return *this;
	}

	Float4& operator -= (const Float4& o)
	{
		x -= o.x;	y -= o.y;	z -= o.z;	w -= o.w;
		return *this;
	}

	Float4& operator *= (float f)
	{
		x *= f;	y *= f;	z *= f;	w *= f;
		return *this;
	}

	Float4& operator /= (float f)
	{
		x /= f;	y /= f;	z /= f;	w /= f;
		return *this;
	}

	Float4 operator + (const Float4& o) const
	{
		Float4 ret;
		ret.x = x + o.x;	ret.y = y + o.y;	ret.z = z + o.z;	ret.w = w + o.w;
		return ret;
	}

	Float4 operator - (const Float4& o) const
	{
		Float4 ret;
		ret.x = x - o.x;	ret.y = y - o.y;	ret.z = z - o.z;	ret.w = w - o.w;
		return ret;
	}

	Float4 operator * (float f) const
	{
		Float4 ret;
		ret.x = x * f;	ret.y = y * f;	ret.z = z * f;	ret.w = w * f;
		return ret;
	}

	Float4 operator / (float f) const
	{
		Float4 ret;
		ret.x = x / f;	ret.y = y / f;	ret.z = z / f;	ret.w = w / f;
		return ret;
	}

	// ベクトルの長さ
	float magnitude() const
	{
		return std::sqrtf(x*x + y*y + z*z + w*w);
	}

	// ベクトルの長さの2乗
	float sqrMagnitude() const
	{
		return (x*x + y*y + z*z + w*w);
	}

	// 内積
	float dot(const Float4& o) const
	{
		return (x*o.x + y*o.y + z*o.z + w*o.w);
	}

	// 自身を正規化
	Float4& normalize()
	{
		*this /= magnitude();
		return *this;
	}

	// 正規化
	Float4 normalized() const
	{
		return (*this / magnitude());
	}

	// 長さが0かどうか
	bool isZero() const
	{
		return compare(x, 0) && compare(y, 0) && compare(z, 0) && compare(w, 0);
	}
  };

  struct Float3
  {
    float x, y, z;

	Float3() {}

	Float3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    bool operator==(const Float3& o) const
    {
      return compare(x, o.x) && compare(y, o.y) && compare(z, o.z);
    }

    bool operator!=(const Float3& o) const
    {
      return !(*this == o);
    }

	Float3& operator += (const Float3& o)
	{
		x += o.x;	y += o.y;	z += o.z;
		return *this;
	}

	Float3& operator -= (const Float3& o)
	{
		x -= o.x;	y -= o.y;	z -= o.z;
		return *this;
	}

	Float3& operator *= (float f)
	{
		x *= f;	y *= f;	z *= f;
		return *this;
	}

	Float3& operator /= (float f)
	{
		x /= f;	y /= f;	z /= f;
		return *this;
	}

	Float3 operator + (const Float3& o) const
	{
		Float3 ret;
		ret.x = x + o.x;	ret.y = y + o.y;	ret.z = z + o.z;
		return ret;
	}

	Float3 operator - (const Float3& o) const
	{
		Float3 ret;
		ret.x = x - o.x;	ret.y = y - o.y;	ret.z = z - o.z;
		return ret;
	}

	Float3 operator * (float f) const
	{
		Float3 ret;
		ret.x = x * f;	ret.y = y * f;	ret.z = z * f;
		return ret;
	}

	Float3 operator / (float f) const
	{
		Float3 ret;
		ret.x = x / f;	ret.y = y / f;	ret.z = z / f;
		return ret;
	}

	// ベクトルの長さ
	float magnitude() const
	{
		return std::sqrtf(x*x + y*y + z*z);
	}

	// ベクトルの長さの2乗
	float sqrMagnitude() const
	{
		return (x*x + y*y + z*z);
	}

	// 内積
	float dot(const Float3& o) const
	{
		return (x*o.x + y*o.y + z*o.z);
	}

	// 外積
	Float3 cross(const Float3& o) const
	{
		Float3 ret;
		ret.x = y*o.z - z*o.y;
		ret.y = z*o.x - x*o.z;
		ret.z = x*o.y - y*o.x;
		return ret;
	}

	// 自身を正規化
	Float3& normalize()
	{
		*this /= magnitude();
		return *this;
	}

	// 正規化
	Float3 normalized() const
	{
		return (*this / magnitude());
	}

	// 長さが0かどうか
	bool isZero() const
	{
		return compare(x, 0) && compare(y, 0) && compare(z, 0);
	}

	// 行列を掛ける
	Float3 transform(const D3DMATRIX &m) const
	{
		Float3 ret;
		ret.x = x * m._11 + y * m._21 + z * m._31 + m._41;
		ret.y = x * m._12 + y * m._22 + z * m._32 + m._42;
		ret.z = x * m._13 + y * m._23 + z * m._33 + m._43;
		return ret;
	}

	// 行列を掛ける
	Float4 transform4(const D3DMATRIX &m) const
	{
		Float4 ret;
		ret.x = x * m._11 + y * m._21 + z * m._31 + m._41;
		ret.y = x * m._12 + y * m._22 + z * m._32 + m._42;
		ret.z = x * m._13 + y * m._23 + z * m._33 + m._43;
		ret.w = x * m._14 + y * m._24 + z * m._34 + m._44;
		return ret;
	}
  };
  
  // ベクトルに行列を掛ける
  Float3* Float3Transform(Float3 *pOut, const Float3 *pV, const D3DMATRIX *pM)
  {
	*pOut = pV->transform(*pM);
	return pOut;
  }
  // ベクトルに行列を掛ける
  Float4* Float3Transform(Float4 *pOut, const Float3 *pV, const D3DMATRIX *pM)
  {
	*pOut = pV->transform4(*pM);
	return pOut;
  }

  // ラジアンを角度に変換
  float Rad2Deg(float rad)
  {
	  return rad * 180.0f / (float)M_PI;
  }

  // 角度をラジアンに変換
  float Deg2Rad(float deg)
  {
	  return deg * (float)M_PI / 180.0f;
  }

  // なす角を求める
  float SubtendedDeg(const Float3 *pV1, const Float3 *pV2)
  {
	  // 正規化して
	  auto v1 = pV1->normalized();
	  auto v2 = pV2->normalized();

	  // 内積
	  auto cos = v1.dot(v2);

	  // 角度にして返す
	  return Rad2Deg(acos(cos));
  }
  // なす角を求める
  float SubtendedDeg(const Float4 *pV1, const Float4 *pV2)
  {
	  // 正規化して
	  auto v1 = pV1->normalized();
	  auto v2 = pV2->normalized();

	  // 内積
	  auto cos = v1.dot(v2);

	  // 角度にして返す
	  return Rad2Deg(acos(cos));
  }

  // 回転行列からオイラー角(ラジアン)へ変換する
  Float3* Matrix2Rad(Float3 *pV, const D3DMATRIX &m)
  {
	  // X軸(1, 0, 0) を回転した時の成分 (_11, _12, _13)
	  // Y軸(0, 1, 0) を回転した時の成分 (_21, _22, _23)
	  // Z軸(0, 0, 1) を回転した時の成分 (_31, _32, _33)
	  pV->x = -asin(m._32);			// Z軸のY成分からX回転を求める
	  pV->y = -atan2(m._31, m._33);	// Z軸のXZ成分からY回転を求める
	  pV->z = -atan2(m._12, m._22);	// X軸とY軸のY成分からZ回転を求める

										// Xが90±0.00005度の場合
	  if (fabs(cos(pV->x)) < 1.0e-6f)
	  {
		  pV->z = (m._12 > 0.0f) ? -(float)M_PI : +(float)M_PI;	// X軸のY成分でZの方向を決める
		  pV->y = (m._31 > 0.0f) ? -(float)M_PI : +(float)M_PI;	// Z軸のX成分でYの方向を決める
	  }

	  return pV;
  }

  // 回転行列からオイラー角(角度)へ変換する
  Float3* Matrix2Deg(Float3 *pV, const D3DMATRIX &m)
  {
	  Matrix2Rad(pV, m);

	  pV->x = Rad2Deg(pV->x);
	  pV->y = Rad2Deg(pV->y);
	  pV->z = Rad2Deg(pV->z);
	  return pV;
  }

  // 84バイト
  struct CameraKeyFrameData
  {
    int frame_no;				// フレーム番号0～
    int pre_index;				// 前のキーフレーム 最初のフレームも0
    int next_index;				// 次のキーフレームがあるときに0以外になる
    float length;				// 距離 表示されている数値に-1を掛けた値が入っている
    Float3 xyz;					// カメラ中心
    Float3 rxyz;				// 角度(ラジアン) X軸は+-反転
    char hokan1_x[6];			// 補間曲線操作 x, y, z, 回転, 距離, 視野角
    char hokan1_y[6];
    char hokan2_x[6];
    char hokan2_y[6];
    int is_perspective;			// 0でパースあり 1でなし 値的には is_orthographic かと
    int view_angle;				// 視野角(度数)
    int is_selected;			// 1で選択している。0で選択していない
    int looking_model_index;	// ボーン追従のモデルのインデックス なしの場合は-1
    int looking_bone_index;		// ボーンのインデックス 0～
	
	void SetLook(int m, int b)	{ looking_model_index = m; looking_bone_index = b; }
	void ClearLook()			{ looking_model_index = -1; looking_bone_index = 0; }

	// fromPを元にtoPを初期化する(リンクや選択状態等以外の情報をコピーする)
	static void InitFrom(CameraKeyFrameData *toP, CameraKeyFrameData *fromP)
	{
		// MMDでキーフレームを打った時、補間曲線操作は前のフレームのものを引き継がないので、ここでも引き継がない
		toP->xyz					= fromP->xyz;
		toP->rxyz					= fromP->rxyz;
		toP->is_perspective			= fromP->is_perspective;
		toP->view_angle				= fromP->view_angle;
		toP->looking_model_index	= fromP->looking_model_index;
		toP->looking_bone_index		= fromP->looking_bone_index;
	}
  };
  
  struct Color3
  {
    float r, g, b;

    bool operator==(const Color3& o) const
    {
      return compare(r, o.r) && compare(g, o.g) && compare(b, o.b);
    }

    bool operator!=(const Color3& o) const
    {
      return !(*this == o);
    }

    int GetR() { return (int)(r * 256); }
    int GetG() { return (int)(g * 256); }
    int GetB() { return (int)(b * 256); }

    void SetR(int val) { r = val / 256.0f; }
    void SetG(int val) { g = val / 256.0f; }
    void SetB(int val) { b = val / 256.0f; }
    void SetRGB(int rv, int gv, int bv) { SetR(rv); SetG(gv); SetB(bv); }
  };
  
  struct Color4
  {
    float r, g, b, a;

    bool operator==(const Color4& o) const
    {
      return compare(r, o.r) && compare(g, o.g) && compare(b, o.b) && compare(a, o.a);
    }

    bool operator!=(const Color4& o) const
    {
      return !(*this == o);
    }

    int GetR() { return (int)(r * 256); }
    int GetG() { return (int)(g * 256); }
    int GetB() { return (int)(b * 256); }
    int GetA() { return (int)(a * 256); }

    void SetR(int val) { r = val / 256.0f; }
    void SetG(int val) { g = val / 256.0f; }
    void SetB(int val) { b = val / 256.0f; }
    void SetA(int val) { a = val / 256.0f; }
    void SetRGB(int rv, int gv, int bv) { SetR(rv); SetG(gv); SetB(bv); }
    void SetRGBA(int rv, int gv, int bv, int av) { SetR(rv); SetG(gv); SetB(bv); SetA(av); }
  };

  // 40バイト
  struct LightKeyFrameData
  {
    int frame_no;				// フレーム番号0～
    int pre_index;				// 前のキーフレーム 最初のフレームも0
    int next_index;				// 次のキーフレームがあるときに0以外になる
    Float3 xyz;					// 位置
    Color3 rgb;					// 色 256が1.0f
    int is_selected;			// 1で選択している。0で選択していない

	// fromPを元にtoPを初期化する(リンクや選択状態等以外の情報をコピーする)
	static void InitFrom(LightKeyFrameData *toP, LightKeyFrameData *fromP)
	{
		toP->xyz		= fromP->xyz;
		toP->rgb		= fromP->rgb;
	}
  };

  // 24バイト
  struct SelfShadowKeyFrameData
  {
    int frame_no;				// フレーム番号0～
    int pre_index;				// 前のキーフレーム 最初のフレームも0
    int next_index;				// 次のキーフレームがあるときに0以外になる
    
    enum class ModeType : int32_t
    {
      None,		// 影なし
      Mode1,	// モード1
      Mode2		// モード2
    };
    
    ModeType mode;				// モード
    float range;				// 影範囲 0.1 - (表示値 * 0.00001)
    int is_selected;			// 1で選択している。0で選択していない

	// fromPを元にtoPを初期化する(リンクや選択状態等以外の情報をコピーする)
	static void InitFrom(SelfShadowKeyFrameData *toP, SelfShadowKeyFrameData *fromP)
	{
		toP->mode		= fromP->mode;
		toP->range		= fromP->range;
	}
  };

  // 36バイト
  struct GravityKeyFrameData
  {
    int frame_no;				// フレーム番号0～
    int pre_index;				// 前のキーフレーム 最初のフレームも0
    int next_index;				// 次のキーフレームがあるときに0以外になる
    float accel;				// 加速度
    Float3 xyz;					// 向き (0,0,0)が入力された場合は(0,0.1,0)になる
    int noise;					// ノイズ値
    char is_noise;				// ノイズ付加 1でON
    char is_selected;			// 1で選択している。0で選択していない
    char pad[2];				// パディング

	// fromPを元にtoPを初期化する(リンクや選択状態等以外の情報をコピーする)
	static void InitFrom(GravityKeyFrameData *toP, GravityKeyFrameData *fromP)
	{
		toP->accel		= fromP->accel;
		toP->xyz		= fromP->xyz;
		toP->noise		= fromP->noise;
		toP->is_noise	= fromP->is_noise;
	}
  };

  // 60バイト
  struct AccessoryKeyFrameData
  {
    int frame_no;				// フレーム番号0～
    int pre_index;				// 前のキーフレーム 最初のフレームも0
    int next_index;				// 次のキーフレームがあるときに0以外になる
    char is_visible;			// 表示 1でON
    char is_shadow;				// 影 1でON
    char pad[2];				// パディング
    int looking_model_index;	// ボーン追従のモデルのインデックス 地面の場合は-1
    int looking_bone_index;		// ボーンのインデックス 0～
    int is_selected;			// 1で選択している。0で選択していない
    Float3 xyz;					// X Y Z
    Float3 rxyz;				// Rx Ry Rz
    float si;					// Si
    float tr;					// Tr
	
	void SetLook(int m, int b)	{ looking_model_index = m; looking_bone_index = b; }
	void ClearLook()			{ looking_model_index = -1; looking_bone_index = 0; }
	
	// fromPを元にtoPを初期化する(リンクや選択状態等以外の情報をコピーする)
	static void InitFrom(AccessoryKeyFrameData *toP, AccessoryKeyFrameData *fromP)
	{
		auto is_selected = toP->is_selected;
		memcpy(&toP->is_visible, &fromP->is_visible, sizeof(AccessoryKeyFrameData) - offsetof(AccessoryKeyFrameData, is_visible));
		toP->is_selected = is_selected;
	}
  };
  
  struct MMDAccessoryData
  {
    void* __unknown_pointer10[4];// [2]にはテクスチャのフルパス(wchar_t)っぽいものが見える
    wchar_t dir_path[256];		// アクセサリの入っているフォルダ名
    int __unknown10[10];
    char name[100];				// ファイル名
    wchar_t file_path[256];		// ファイル名を含むパス
	char __unknown40;
	char render_order;			// アクセサリ描画順(アクセサリ設定の順番) 0～ アクセサリ操作のコンボボックスの並びはこれの昇順
	char __unknown41[2];
	int __unknown42[3];
	char is_timeline_select;	// カメラモードのタイムラインでこのアクセサリの行が選択状態なら1
	char pad[3];
	int __unknown43[3];
  };

  struct BoneCurrentData
  {
	// 行列や位置、クォータニオンはフレームを動かした後再描画を待たないと更新されない
	// RedrawWindow() もしくは WM_PAINT が処理されるのを待つ
    char name_jp[20];
    char name_en[20];
    wchar_t *wname_jp;					// ポインタなので構造体の先頭がwchar_tの名前と言うだけかもしれない こっちは20文字以上でも最後まで入っている 1300文字くらいまでは確認
    wchar_t *wname_en;
    int parent_bone;					// 親ボーンのインデックス
    D3DMATRIX mat[4];					// 行列 [0]は初期値XYZを現在のワールド位置に変換する行列(ローカル位置やローカル回転も含んだ行列) [0]と[3]は同じ? [1]は単位行列?
    float init_x, init_y, init_z;		// 初期値XYZ(ボーンの「位置」で設定している値)
    float x, y, z;						// 現在のフレームでのローカル位置
    float local_rotation_q[4];			// 現在のフレームでのローカル回転
    float ik_rotation_q[4];				// 現在のフレームでのIK込みでのローカル回転
    float x2, y2, z2;					// x, y, z と同じ
    float __unknown30_rotation_q[4];
    int __unknown30[17];
    int to_bone;						// 接続先ボーンのインデックス なしの場合は-1 ConnectToBone が立っている時のみ有効 ボーン総数より大きい値を入力されている場合は不定
    Float3 to_offset;					// ボーン位置からの相対オフセット ConnectToBone が立っていない時のみ有効 xが-1された値で入っている (0,0,0)を入力した場合(-1,0,0)
    int __unknown38[3];
    int append_parent_bone;				// 回転付与/移動付の「付与親ボーンのインデックス」
    int __unknown39;
    int level;							// 物理後変形の「変形階層」

    enum BoneFlag : short
    {
      ConnectToBone				= 1<< 0, // 0x0001  : 接続先(PMD子ボーン指定)表示方法 -> 0:座標オフセットで指定 1:ボーンで指定
      Rotation					= 1<< 1, // 0x0002  : 回転可能(回転 ボタン)
      Translation				= 1<< 2, // 0x0004  : 移動可能(移動 ボタン)
      Visible					= 1<< 3, // 0x0008  : 表示(表示 ボタン)
      Controllable				= 1<< 4, // 0x0010  : 操作可(操作 ボタン)
      IK						= 1<< 5, // 0x0020  : IK(IK ボタン)
      __None__					= 1<< 6,
      AppendLocal				= 1<< 7, // 0x0080  : ローカル付与 | 付与対象 0:ユーザー変形値／IKリンク／多重付与 1:親のローカル変形量(付与親：L ボタン)
      AppendRotation			= 1<< 8, // 0x0100  : 回転付与(付与：回転+ ボタン)
      AppendTranslation			= 1<< 9, // 0x0200  : 移動付与(付与：移動+ ボタン)
      FixAxis					= 1<<10, // 0x0400  : 軸固定(軸制限 ボタン)
      LocalFrame				= 1<<11, // 0x0800  : ローカル軸(ローカル軸 ボタン)
      AfterPhysics				= 1<<12, // 0x1000  : 物理後変形(物理後 ボタン)
      External					= 1<<13, // 0x2000  : 外部親変形(外部親 ボタン)
    };
    BoneFlag flg;						// ボーンフラグ
    char pad[2];						// パディング

    float append_ratio;					// 回転付与/移動付の「付与率」

    Float3 fix_axis;					// 軸固定の「軸の方向ベクトル」そのままではなく正規化された値が入っている

    Float3 local_axis_x;				// ローカル軸の「X軸の方向ベクトル」 PMXEで計算済みの値
    Float3 local_axis_z;				// ローカル軸の「Z軸の方向ベクトル」 PMXEで計算済みの値

    int external_key;					// 外部親変形の「親Key」

    int ik_target_bone;					// IKターゲットボーンのインデックス
    int ik_loop_count;					// IKループ回数
    float ik_unit_angle;				// IKループ計算時の1回あたりの単位角度(ラジアン)
    int ik_link_count;					// IKリンク数 : 後続のik_link[]要素数
    int __unknown48;

    // IKリンク
    struct IkLink
    {
      int link_bone;					// リンクボーンのインデックス
      bool is_limit;					// 角度制限 0:OFF 1:ON
      Float3 low;						// 下限角度x,y,z(ラジアン)
      Float3 high;						// 上限角度x,y,z(ラジアン)
    }* ik_link;							// IKリンクの先頭ポインタ [0]～[ik_link_count-1]まで

    int __unknown49[8];
	int parentable_bone_index;			// 外部親設定の「対象ボーン」でのインデックス 「対象ボーン」でないボーンは-1が入っている 「対象ボーン」の0番は「ルート」なのでここには 1～parentable_bone_count-1までの値が入っている
	int __unknown50;
  };

  static_assert(sizeof(BoneCurrentData) == 624, "");

  // 頂点モーフ
  struct MorphDataVertex
  {
    int index;							// モーフ対象の頂点インデックス
    Float3 v;							// 座標オフセット量(x,y,z)
  };

  // UVモーフ
  struct MorphDataUv
  {
    int index;							// モーフ対象の頂点インデックス
    Float4 uv;							// UVオフセット量(x,y,z,w) ※通常UVはz,wが不要項目になるがモーフとしてのデータ値は記録しておく
  };

  // ボーンモーフ
  struct MorphDataBone
  {
    int index;							// モーフ対象のボーンインデックス
    Float3 m;							// 移動量(x,y,z)
    Float4 q;							// 回転量-クォータニオン(x,y,z,w)
  };

  // 材質モーフ
  struct MorphDataMaterial
  {
    int index;							// モーフ対象の材質インデックス -1:全材質対象
    int type;							// オフセット演算形式 0:乗算 1:加算
    Color4 diffuse;						// 拡散色/非透過度 (R,G,B,A)
    Color4 specular;					// 反射色 (R,G,B) Aは未使用
    float specular_intensity;			// 反射強度
    Color4 ambient;						// 環境色 (R,G,B)
    Color4 edge;						// エッジ色 (R,G,B,A)
    float edge_size;					// エッジサイズ
    Color4 tex;							// テクスチャ係数 (R,G,B,A)
    Color4 sphere;						// スフィアテクスチャ係数 (R,G,B,A)
    Color4 toon;						// Toonテクスチャ係数 (R,G,B,A)
  };

  // グループモーフ
  struct MorphDataGroup
  {
    int index;							// モーフ対象のモーフインデックス グループモーフのグループ化は非対応
    float rate;							// モーフ率 : グループモーフのモーフ値 * モーフ率 = 対象モーフのモーフ値
  };

  // モーフ
  struct MorphCurrentData
  {
    char name_jp[20];
    char name_en[20];
    wchar_t *wname_jp;					// ポインタなので構造体の先頭がwchar_tの名前と言うだけかもしれない こっちは20文字以上でも最後まで入っている 1300文字くらいまでは確認
    wchar_t *wname_en;
    int __unknown0;
    int data_size_vertex;				// 頂点モーフのデータ数
    int data_size_uv;					// UVモーフのデータ数 
    int data_size_uv1;					// 追加UV1モーフのデータ数 
    int data_size_uv2;					// 追加UV2モーフのデータ数 
    int data_size_uv3;					// 追加UV3モーフのデータ数 
    int data_size_uv4;					// 追加UV4モーフのデータ数 
    int data_size_bone;					// ボーンモーフのデータ数
    int data_size_group;				// グループモーフのデータ数
    int data_size_material;				// 材質モーフのデータ数
    char panel;							// 操作パネル 1:眉(左下) 2:目(左上) 3:口(右上) 4:その他(右下)  | 0:システム予約
    char type;							// モーフ種類 - 0:グループ, 1:頂点, 2:ボーン, 3:UV, 4:追加UV1, 5:追加UV2, 6:追加UV3, 7:追加UV4, 8:材質
    char __unknown10[6];
    MorphDataVertex *data_vertex;		// 頂点モーフデータ [data_size_vertex] 個の配列
    MorphDataBone *data_bone;			// ボーンモーフデータ [data_size_bone] 個の配列
    MorphDataGroup *data_group;			// グループモーフデータ [data_size_group] 個の配列
    MorphDataUv *data_uv;				// UVモーフデータ [data_size_uv] 個の配列
    MorphDataUv *data_uv1;				// 追加UV1モーフデータ [data_size_uv1] 個の配列
    MorphDataUv *data_uv2;				// 追加UV2モーフデータ [data_size_uv2] 個の配列
    MorphDataUv *data_uv3;				// 追加UV3モーフデータ [data_size_uv3] 個の配列
    MorphDataUv *data_uv4;				// 追加UV4モーフデータ [data_size_uv4] 個の配列
    MorphDataMaterial *data_material;	// 材質モーフデータ [data_size_material] 個の配列
    void *data__unknown_0;
    void *data__unknown_1;
  };

  static_assert(sizeof(MorphCurrentData) == 192, "");

  // モーフのタイムライン情報
  struct MorphTimelineData
  {
	  char name_jp[20];
	  char name_en[20];
	  char __unknown0[4];
	  char is_timeline_select;			// タイムラインのモーフ行選択状態(何故か MorphCurrentData に含まれずここにある?)
	  char __unknown1;
  };

  static_assert(sizeof(MorphTimelineData) == 46, "");

  struct MMDModelData
  {
    int __unknown10[2224];
    char name_jp[50];
    char name_en[50];
    char comment_jp[256];
    char comment_en[292]; // もしかしたら別の領域に分かれてるかも
    wchar_t file_path[256];
    BoneCurrentData* bone_current_data;		// ボーン情報の配列 [0]～[bone_count-1]まで
    int __unknown20[2];
    MorphCurrentData* morph_current_data;	// 表情モーフ情報の配列 [0]～[morph_count-1]まで
    int __unknown25[6];
    char keyframe_editor_toplevel_rows;
    void* __unknown30[1];
	MorphTimelineData* morph_timeline_data;	// 表情モーフのタイムライン情報配列 [0]～[morph_count-1]まで

    struct BoneKeyFrame
    {
      int frame_no;							// フレーム番号0～
      int pre_index;						// 前のキーフレーム 最初のフレームも0
      int next_index;						// 次のキーフレームがあるときに0以外になる
      char interpolation_curve_x1[4];
      char interpolation_curve_y1[4];
      char interpolation_curve_x2[4];
      char interpolation_curve_y2[4];
      float x, y, z;						// X Y Z
      float rotation_q[4];
      int is_selected;						// 1で選択している。0で選択していない
	  
	// fromPを元にtoPを初期化する(リンクや選択状態等以外の情報をコピーする)
      static void InitFrom(BoneKeyFrame *toP, BoneKeyFrame *fromP)
      {
      	auto is_selected = toP->is_selected;
      	memcpy(&toP->interpolation_curve_x1, &fromP->interpolation_curve_x1, sizeof(BoneKeyFrame) - offsetof(BoneKeyFrame, interpolation_curve_x1));
      	toP->is_selected = is_selected;
      }
    };
    // [0]～[bone_count-1]までが各ボーンの0フレームのキーフレーム そこからリンクドリストでボーンのフレームが設定されている
    // 空きを探す場合は[bone_count]からフレーム番号(frame_no)が0の要素を探す
    BoneKeyFrame(&bone_keyframe)[600000];

    struct MorphKeyFrame
    {
      int frame_no;							// フレーム番号0～
      int pre_index;						// 前のキーフレーム 最初のフレームも0(だが非表示モーフの最初のフレームに変な値が入っていた事も…)
      int next_index;						// 次のキーフレームがあるときに0以外になる
      float value;							// モーフ値
      char is_selected;						// 1で選択している。0で選択していない
	  
	// fromPを元にtoPを初期化する(リンクや選択状態等以外の情報をコピーする)
      static void InitFrom(MorphKeyFrame *toP, MorphKeyFrame *fromP)
      {
      	toP->value = fromP->value;
      }
    };
	// [0]～[morph_count-1]までが各モーフの0フレームのキーフレーム そこからリンクドリストでモーフのフレームが設定されている
    // 空きを探す場合は[morph_count]からフレーム番号(frame_no)が0の要素を探す
	// 表示枠に追加されていないモーフもこのテーブルは存在する
    MorphKeyFrame(&morph_keyframe)[20000];

	// 表示・IK・外観 のキーフレーム
    struct ConfigurationKeyFrame
    {
      int frame_no;							// フレーム番号0～
      int pre_index;						// 前のキーフレーム 最初のフレームも0
      int next_index;						// 次のキーフレームがあるときに0以外になる
      char is_visible;						// 「表示」のON/OFF 0:非表示 1:表示
      char* is_ik_enabled;					// IKが有効かどうか 0:OFF 1:ON [0]～[ik_count-1]まで
	  int is_selected;						// 1で選択している。0で選択していない
      int __unknown;

	  // 外部親設定の対象ボーン [0]～[parentable_bone_count-1]
	  // [0]は「ルート」
	  // bone_current_data->parentable_bone_index がこの配列の要素番号になる
      struct RelationSetting
      {
        int parent_model_index;				// 外部親モデル -1:なし -2:地面 0～:モデルのインデックス
        int parent_bone_index;				// 外部親ボーン モデルがなし/地面の場合は0が入っている
		void Set(int m, int b)	{ parent_model_index = m; parent_bone_index = b; }
		void Clear()			{ parent_model_index = -1; parent_bone_index = 0; }
      }*relation_setting;
	  
	// fromPを元にtoPを初期化する(リンクや選択状態等以外の情報をコピーする)
      static void InitFrom(ConfigurationKeyFrame *toP, ConfigurationKeyFrame *fromP, MMDModelData *modelP)
      {
		toP->is_visible		= fromP->is_visible;
		memcpy(toP->is_ik_enabled, fromP->is_ik_enabled, modelP->ik_count);
		toP->__unknown		= fromP->__unknown;
		memcpy(toP->relation_setting, fromP->relation_setting, modelP->parentable_bone_count * sizeof(RelationSetting));
      }
	};
	ConfigurationKeyFrame(&configuration_keyframe)[1000];

    int __unknown40[600];
    char render_order;						// モデル描画順 1～ モデル追従等のコンボボックスの並びはこれの昇順
    int morph_count;						// モーフの総数
    int bone_count;							// ボーンの総数 表示枠に表示されていないボーンも含む
    int ik_count;							// IKの個数
    char __unknown45;
    char is_visible;
    int selected_bone;
    char *bone_timeline_select;				// タイムラインのボーン行選択状態配列 [0]～[bone_count-1]まで
	int __unknown50[2];
    int selected_morph_indices[4];			// 「表情操作」で選択されているモーフの番号(番号はPmxEditorのモーフタブでの番号と同じ) [0]まゆ [1]目 [2]リップ [3]その他
    int __unknown59[257];
    int vscroll;
    int last_frame_number;
    int __unknown61[150475];                // [13]～は pmx のフルパス(wchar)や、その後ろにはトゥーンファイル名等が見える そのあとはゼロっぽい
    int parentable_bone_count;				// 外部親設定の「対象ボーン」数 configuration_keyframe->relation_setting の要素数
  };

  // ReSharper disable CppZeroConstantCanBeReplacedWithNullptr
  static constexpr int a = offsetof(MMDModelData, parentable_bone_count );
  static_assert(8896 == offsetof(MMDModelData, name_jp), "");
  static_assert(10104 == offsetof(MMDModelData, keyframe_editor_toplevel_rows), "");
  static_assert(12560 == offsetof(MMDModelData, bone_count), "");
  static_assert(12572 == offsetof(MMDModelData, selected_bone), "");
  static_assert(12592 == offsetof(MMDModelData, selected_morph_indices), "");
  static_assert(615544 == offsetof( MMDModelData, parentable_bone_count ), "");
  // ReSharper restore CppZeroConstantCanBeReplacedWithNullptr

  struct MMDMainData
  {
    int __unknown10[2];
    int mouse_x, mouse_y;
    int pre_mouse_x, pre_mouse_y;
    int key_up;
    int key_down;
    int key_left;
    int key_right;
    int key_shift;							// keyは0から3までの数値を取る。押している間は3になる。
    int key_space;
    int key_f9;
    int key_x_or_f11;						// f11の場合は2になる
    int key_z;
    int key_c;
    int key_v;
    int key_d;
    int key_a;
    int key_b;
    int key_g;
    int key_s;
    int key_i;
    int key_h;
    int key_k;
    int key_p;
    int key_u;
    int key_j;
    int key_f;
    int key_r;
    int key_l;
    int key_close_bracket;
    int key_backslash;
    int key_tab;
    int __unknown20[14];
    int key_enter;
    int key_ctrl;
    int key_alt;
    int __unknown30;
    void* __unknown_pointer;
    int __unknown40[148];
    bool is_camera_edit_mode;				// true:カメラ編集モード false:モデル編集モード
    int __unknown41[7];
    Float3 rxyz;
    int __unknown49[1];
    float counter_f;
    int counter;
    int __unknown50[2];
    int is_playing;							// 再生中かどうか 0:停止中 1:再生中
    Float3 xyz;
    int __unknown60[22];
    CameraKeyFrameData(&camera_key_frame)[10000];
    LightKeyFrameData(&light_key_frame)[10000];						// 照明のキーフレームポインタ 1要素は40バイト
    SelfShadowKeyFrameData(&self_shadow_key_frame)[10000];			// セルフ影のキーフレームポインタ 1要素は24バイト
    GravityKeyFrameData(&gravity_key_frame)[10000];					// 重力のキーフレームポインタ 1要素は36バイト
    typedef AccessoryKeyFrameData AccessoryKeyFrameDataAry[10000];
    AccessoryKeyFrameDataAry* accessory_key_frames[255];			// アクセサリのキーフレームポインタの配列
    MMDModelData* model_data[255];			// 起動時nullモデルを読み込むと順番にポインタが入る モデルを削除するとその要素はnullになり、別のモデルを読み込むとそこに挿入される 「モデル操作」の並びとこの配列は必ずしも一致しない
    int select_model;						// 現在「モデル操作」で選択されているmodel_data[]の要素番号 カメラ編集に変えた場合も最後に選択されていた番号が入っている モデルを削除した場合も自動でカメラ編集になるが、消されたモデルの要素番号のまま

    enum class SelectBoneType : int
    {
      Select,
      Box,
      Camera,
      Rotate,
      Move
    };

    SelectBoneType select_bone_type; // 0:選択、1:BOX選択、2:カメラモード、3:回転、4:移動
    int __unknown70[4];
    float __unknown71;
    int mouse_over_move; // xyz回転(9,10,11)、xyz移動(12,13,14)
    int __unknown80[17];
    int left_frame;
    int __unknown90;
    int pre_left_frame;
    int now_frame;							// 現在のフレーム
    int __unknown100[160800];
    char __unknown101;
    unsigned char edit_interpolation_curve[4]; // x1 y1 x2 y2
    int __unknown103[15];
    int accessory_timeline_top;				// タイムラインを下にスクロールした時に隠れている行数 アクセサリが3個あって、下にスクロールさせて3番目のみ表示していたら上2行が隠れているのでこの値は「2」になっている 「一番上(重力の1つ下)に表示されている行番号」と言う方が分かりやすい? 初期値や全アクセサリを消した場合は「0」だが、その状態でタイムラインを操作すると「-1」が入る
    int accessory_num;						// アクセサリ数+1が入っている 初期値は1
    int accessory_timeline[200];			// タイムラインに表示されているアクセサリの accessory_data や accessory_key_frames のインデックス 空行の場合は-1 上下にスクロールするとその分ズレる [0]が一番上(重力の1つ下)に表示されているアクセサリと言う事
    MMDAccessoryData* accessory_data[255];	// アクセサリを読み込むと順番にポインタが入る
    int max_use_frame;						// 最大使用フレーム カメラ/照明/セルフ影/重力/モデルボーン 全てで最大のフレーム このフレームまで再生される キーフレームを消してもタイムラインが縮まないが、この値を書き換えてやれば縮む
    int select_accessory;					// 「アクセサリ操作」パネル選択中のアクセサリの accessory_data や accessory_key_frames のインデックス コンボボックスの順番ではない アクセサリが1つも読み込まれていない場合も0が入っている
    int __unknown104[29];
    float view_angle;						// 視野角(度数) 書き換えても反映されるわけではない
    int __unknown105[290];
    float play_sec;							// 再生中の先頭からの秒数 *30 でフレーム数になる 停止中は値は変わらない
    int __unknown106[1933];
    char is_camera_select;					// カメラモードのタイムラインで「カメラ」行が選択状態なら1
    char is_light_select;					// カメラモードのタイムラインで「照明」行が選択状態なら1
    char is_self_shadow_select;				// カメラモードのタイムラインで「セルフ影」行が選択状態なら1
    char is_gravity_select;					// カメラモードのタイムラインで「重力」行が選択状態なら1
    char __unknown109[124];
	int __unknown110[67];
	char accessory_timeline_select[200];	// カメラモードのタイムラインでアクセサリの行選択状態なら1 上下にスクロールするとその分ズレる(そのアクセサリ自身が選択されているかどうかは MMDAccessoryData の is_timeline_select を見ること) [0]が一番上(重力の1つ下)に表示されているアクセサリと言う事
    int __unknown111[203];
    int output_size_x;
    int output_size_y;
    float length;
    unsigned char __unknown120[32];
    wchar_t pmm_path[256];					// MikuMikuDance.exe のパス
	int __unknown130[10];
	char accessory_draw_after_model;		// 「アクセサリ設定」の「X番目より後のアクセサリはモデル描写後に描写する」の値
	char __unknown131[3];
	int __unknown132[16];
    bool is_english_mode;					// 英語モードかどうか true:英語 false:日本語
  };


  // ReSharper disable CppZeroConstantCanBeReplacedWithNullptr
  static_assert(5200 == offsetof(MMDMainData, now_frame), "");
  static_assert(648405 == offsetof(MMDMainData, edit_interpolation_curve), "");
  static_assert(660344 == offsetof(MMDMainData, is_camera_select), "");
  static_assert(661760 == offsetof(MMDMainData, length), "");
  static_assert(661796 == offsetof(MMDMainData, pmm_path), "");
  // ReSharper restore CppZeroConstantCanBeReplacedWithNullptr

  inline MMDMainData* getMMDMainData()
  {
    auto pointer = (BYTE**)((BYTE*)GetModuleHandleW(nullptr) + 0x1445F8);
    if ( IsBadReadPtr(pointer, sizeof(INT_PTR)) != 0 )
    {
      std::wstring error = L"内部エラー\nポインタの読み込みに失敗しました\npointer=" + std::to_wstring((INT_PTR)pointer);
      MessageBoxW(nullptr, error.c_str(), L"エラー", MB_OK);
      return nullptr;
    }
    return (MMDMainData*)*pointer;
  }
  
  // バージョン番号を取得 Ver.9.31 かどうかの確認用
  inline double getMMDVersion()
  {
    auto pointer = (double*)((BYTE*)GetModuleHandleW(nullptr) + 0x132EA8);
    if ( IsBadReadPtr(pointer, sizeof(INT_PTR)) != 0 )
    {
      std::wstring error = L"内部エラー\nポインタの読み込みに失敗しました\npointer=" + std::to_wstring((INT_PTR)pointer);
      MessageBoxW(nullptr, error.c_str(), L"エラー", MB_OK);
      return 0.0;
    }
    return *pointer;
  }
  
  // バージョン番号を取得 Ver.9.31 かどうかの確認用
  inline bool MMDVersionOk()
  {
    return getMMDVersion() == 9.31;
  }

  // モデルを現在のフレームで再表示する関数(?)を呼び出す
  inline void redrawModel()
  {
    auto base = (BYTE*)GetModuleHandleW(nullptr);
    auto func = (void(*)())(base + 0x225B0);
	func();
  }
}

namespace mmp
{
  MMD_PLUGIN_API MMDPluginDLL3* getDLL3Object(const char* dll_title);

  namespace filesystem = std::filesystem;

  inline filesystem::path getDLLPath(HMODULE module)
  {
    wchar_t module_path[MAX_PATH + 1];
    GetModuleFileNameW(module, module_path, MAX_PATH);
    return module_path;
  }
}

extern "C"
{
  MMD_PLUGIN_API int version();

  MMD_PLUGIN_API MMDPluginDLL1* create1(IDirect3DDevice9* device);
  MMD_PLUGIN_API void destroy1(MMDPluginDLL1* p);

  MMD_PLUGIN_API MMDPluginDLL2* create2(IDirect3DDevice9* device);
  MMD_PLUGIN_API void destroy2(MMDPluginDLL2* p);

  MMD_PLUGIN_API MMDPluginDLL3* create3(IDirect3DDevice9* device);
  MMD_PLUGIN_API void destroy3(MMDPluginDLL3* p);

  MMD_PLUGIN_API MMDPluginDLL4* create4(IDirect3DDevice9* device);
  MMD_PLUGIN_API void destroy4(MMDPluginDLL4* p);

  // 配列の場合
  MMD_PLUGIN_API MMDPluginDLL3** createArray3(IDirect3DDevice9* device, int* out_array_size);
  MMD_PLUGIN_API void destroyArray3(MMDPluginDLL3** p);

  MMD_PLUGIN_API MMDPluginDLL4** createArray4(IDirect3DDevice9* device, int* out_array_size);
  MMD_PLUGIN_API void destroyArray4(MMDPluginDLL4** p);

}
