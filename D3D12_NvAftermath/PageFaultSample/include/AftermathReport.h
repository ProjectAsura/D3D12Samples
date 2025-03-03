#pragma once

struct ID3D12Device;
struct ID3D12Resource;

bool InitNvAftermath(ID3D12Device* pDevice);
void TermNvAftermath();

void AddResource(ID3D12Resource* pResource);

void ReportNvAftermath();
