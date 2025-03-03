//-----------------------------------------------------------------------------
// File : AftermathHelper.h
// Desc : NVIDIA Aftermath Helpers.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <string>
#include <vector>


struct ID3D12Device;

//-----------------------------------------------------------------------------
//! @brief      GPU�N���b�V���g���b�J�[�����������܂�.
//! 
//! @param[in]      appName                 �A�v����.
//! @param[in]      appVersion              �A�v���o�[�W����.
//! @param[in]      outputDir               �o�̓f�B���N�g��.
//! @param[in]      shaderBinaryDirs        �V�F�[�_�o�C�i���f�B���N�g��.
//! @param[in]      shaderPdbDirs           �V�F�[�_PDB�f�B���N�g��.
//! @note       InitAftermath() �����O�ɃR�[�����Ă�������.
//-----------------------------------------------------------------------------
void InitGpuCrashTracker(
    const char* appName,
    const char* appVersion,
    const char* outputDir,
    const std::vector<std::string>& shaderBinaryDirs,
    const std::vector<std::string>& shaderPdbDirs);

//-----------------------------------------------------------------------------
//! @brief      GPU�N���b�V���g���b�J�[���I�����܂�.
//-----------------------------------------------------------------------------
void TermGpuCrashTracker();

//-----------------------------------------------------------------------------
//! @brief      Aftermath�����������܂�.
//! 
//! @param[in]      pDevice     �f�o�C�X�ł�.
//! @retval true    �������ɐ���.
//! @retval false   �������Ɏ��s.
//-----------------------------------------------------------------------------
bool InitAftermath(ID3D12Device* pDevice);

//-----------------------------------------------------------------------------
//! @brief      Aftermath���I�����܂�.
//-----------------------------------------------------------------------------
void TermAftermath();

//-----------------------------------------------------------------------------
//! @brief      GPU�N���b�V���_���v
//-----------------------------------------------------------------------------
void ReportAftermath();
