#pragma once
#define INPUTLAYOUT_POSITION(offset) { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0 }
#define INPUTLAYOUT_NORMAL(offset) { "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0 }
#define INPUTLAYOUT_TEXCOORD(offset) { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0 }