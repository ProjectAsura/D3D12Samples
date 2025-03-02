//-----------------------------------------------------------------------------
// File : DredReport.h
// Desc : Device Removed Extended Data Reporter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

#include <d3d12.h>

void ReportDRED(HRESULT hr, ID3D12Device* pDevice);
