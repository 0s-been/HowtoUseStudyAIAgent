#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <wrl/client.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <typeindex>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
