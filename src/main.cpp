#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include <iostream>

using namespace Microsoft::WRL;

int main() {
  // Enable the D3D12 debug layer if in debug mode
#if defined(_DEBUG)
  {
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
      debugController->EnableDebugLayer();
    }
  }
#endif

  // Create the DXGI factory
  ComPtr<IDXGIFactory4> factory;
  HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
  if (FAILED(hr)) {
    std::cerr << "Failed to create DXGI factory." << std::endl;
    return -1;
  }

  // Create the D3D12 device
  ComPtr<ID3D12Device> device;
  hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
  if (FAILED(hr)) {
    std::cerr << "Failed to create D3D12 device." << std::endl;
    return -1;
  }

  // Describe and create the command queue
  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

  ComPtr<ID3D12CommandQueue> commandQueue;
  hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
  if (FAILED(hr)) {
    std::cerr << "Failed to create command queue." << std::endl;
    return -1;
  }

  std::cout << "Command queue created successfully." << std::endl;

  // Describe and create a default heap
  D3D12_HEAP_PROPERTIES heapProperties = {};
  heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
  heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

  D3D12_HEAP_DESC heapDesc = {};
  heapDesc.SizeInBytes = 1024 * 64; // 64 KB
  heapDesc.Properties = heapProperties;
  heapDesc.Alignment = 0;
  heapDesc.Flags = D3D12_HEAP_FLAG_NONE;

  ID3D12Heap* defaultHeap;
  hr = device->CreateHeap(&heapDesc, IID_PPV_ARGS(&defaultHeap));
  if (FAILED(hr)) {
    return 1;
  }

  // Describe the resource
  D3D12_RESOURCE_DESC resourceDesc = {};
  resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  resourceDesc.Alignment = 0;
  resourceDesc.Width = 1024 * 64; // 64 KB
  resourceDesc.Height = 1;
  resourceDesc.DepthOrArraySize = 1;
  resourceDesc.MipLevels = 1;
  resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
  resourceDesc.SampleDesc.Count = 1;
  resourceDesc.SampleDesc.Quality = 0;
  resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

  ID3D12Resource* defaultResource;
  hr = device->CreatePlacedResource(
      defaultHeap,
      0,
      &resourceDesc,
      D3D12_RESOURCE_STATE_COPY_DEST,
      nullptr,
      IID_PPV_ARGS(&defaultResource)
                                    );
  if (FAILED(hr)) {
    return 1;
  }
  // Describe and create an upload heap
  heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

  heapDesc.SizeInBytes = 1024 * 64; // 64 KB
  heapDesc.Properties = heapProperties;
  heapDesc.Alignment = 0;
  heapDesc.Flags = D3D12_HEAP_FLAG_NONE;

  ID3D12Heap* uploadHeap;
  hr = device->CreateHeap(&heapDesc, IID_PPV_ARGS(&uploadHeap));
  if (FAILED(hr)) {
    return 1;
  }

  // Describe the resource for upload
  resourceDesc.Width = 1024 * 64; // 64 KB

  ID3D12Resource* uploadResource;
  hr = device->CreatePlacedResource(
      uploadHeap,
      0,
      &resourceDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&uploadResource)
                                    );
  if (FAILED(hr)) {
    return 1;
  }
  // Create a command allocator and command list
  ID3D12CommandAllocator* commandAllocator;
  ID3D12GraphicsCommandList* commandList;

  hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
  if (FAILED(hr)) {
    return 1;
  }

  hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
  if (FAILED(hr)) {
    return 1;
  }

  // Map the upload resource to get a CPU pointer to the data
  void* pMappedData;
  D3D12_RANGE readRange = {}; // We do not intend to read this resource on the CPU
  hr = uploadResource->Map(0, &readRange, &pMappedData);
  if (FAILED(hr)) {
    return 1;
  }

  char someData[] = {0,1,2,3,4,5,6,7,8,9,};
  // Copy data to the upload resource
  memcpy(pMappedData, someData, 10); // Assume someData and dataSize are defined
  uploadResource->Unmap(0, nullptr);

  // Record commands to copy the data from the upload resource to the default resource
  commandList->CopyResource(defaultResource, uploadResource);

  // Describe and create a readback heap
  D3D12_HEAP_PROPERTIES readbackHeapProperties = {};
  readbackHeapProperties.Type = D3D12_HEAP_TYPE_READBACK;
  readbackHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  readbackHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

  D3D12_HEAP_DESC readbackHeapDesc = {};
  readbackHeapDesc.SizeInBytes = 1024 * 64; // 64 KB
  readbackHeapDesc.Properties = readbackHeapProperties;
  readbackHeapDesc.Alignment = 0;
  readbackHeapDesc.Flags = D3D12_HEAP_FLAG_NONE;

  ID3D12Heap* readbackHeap;
  hr = device->CreateHeap(&readbackHeapDesc, IID_PPV_ARGS(&readbackHeap));
  if (FAILED(hr)) {
    return 1;
  }
  // Describe the resource for readback
  D3D12_RESOURCE_DESC readbackResourceDesc = {};
  readbackResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  readbackResourceDesc.Alignment = 0;
  readbackResourceDesc.Width = 1024 * 64; // 64 KB
  readbackResourceDesc.Height = 1;
  readbackResourceDesc.DepthOrArraySize = 1;
  readbackResourceDesc.MipLevels = 1;
  readbackResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
  readbackResourceDesc.SampleDesc.Count = 1;
  readbackResourceDesc.SampleDesc.Quality = 0;
  readbackResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  readbackResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

  ID3D12Resource* readbackResource;
  hr = device->CreatePlacedResource(
      readbackHeap,
      0,
      &readbackResourceDesc,
      D3D12_RESOURCE_STATE_COPY_DEST,
      nullptr,
      IID_PPV_ARGS(&readbackResource)
                                    );
  if (FAILED(hr)) {
    return 1;
  }
  // Record commands to copy the data from the default resource to the readback resource
  commandList->CopyResource(readbackResource, defaultResource);

  // Close the command list and execute it
  hr = commandList->Close();
  if (FAILED(hr)) {
    return 1;
  }

  ID3D12CommandList* ppCommandLists[] = { commandList };
  commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

  // Create a fence and wait for the GPU to finish executing the command list
  ID3D12Fence* fence;
  UINT64 fenceValue = 1;
  hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
  if (FAILED(hr)) {
    return 1;
  }

  HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
  if (fenceEvent == nullptr) {
    return 1;
  }

  hr = commandQueue->Signal(fence, fenceValue);
  if (FAILED(hr)) {
    return 1;
  }

  if (fence->GetCompletedValue() < fenceValue) {
    hr = fence->SetEventOnCompletion(fenceValue, fenceEvent);
    if (FAILED(hr)) {
      return 1;
    }
    WaitForSingleObject(fenceEvent, INFINITE);
  }

  commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

  CloseHandle(fenceEvent);
  fence->Release();
  commandAllocator->Release();
  commandList->Release();

  // Map the readback resource to a CPU pointer
  void* pReadbackData;
  D3D12_RANGE readbackRange = { 0, 1024 * 64 }; // The range of the resource to read
  hr = readbackResource->Map(0, &readbackRange, &pReadbackData);
  if (FAILED(hr)) {
    return 1;
  }

  // Read the data from the readback resource
  char someOutputData[10]{};
  memcpy(someOutputData, pReadbackData, 10); // Assume someOutputData and dataSize are defined

  // Unmap the readback resource
  D3D12_RANGE writeRange = { 0, 0 }; // We are not going to write to this resource, so we specify a null range
  readbackResource->Unmap(0, &writeRange);

  std::cout << someOutputData << std::endl;

  return 0;
}
