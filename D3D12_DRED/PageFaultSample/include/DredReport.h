﻿//-----------------------------------------------------------------------------
// File : DredReport.h
// Desc : Device Removed Extended Data Reporter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <d3d12.h>

//-----------------------------------------------------------------------------
//! @brief      DRED情報を出力します.
//! 
//! @param[in]      hr          エラー発生時のHRESULT
//! @param[in]      pDevice     デバイス.
//-----------------------------------------------------------------------------
void ReportDRED(HRESULT hr, ID3D12Device* pDevice);
