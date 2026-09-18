#pragma once

#include "d3d9_include.h"
#include "d3d9_format.h"
#include "../dxvk/dxvk_device.h"
#include "../dxvk/dxvk_context.h"

namespace dxvk {

  /**
   * \brief Push constants of the packed 16-bit decoder (d3d9_convert_packed16.comp)
   */
  struct D3D9Packed16Args {
    int32_t  srcOffsetX, srcOffsetY;
    int32_t  dstOffsetX, dstOffsetY;
    uint32_t extentW,    extentH;
    uint32_t srcPitch;      // in texels
    uint32_t forceAlpha;    // X4R4G4B4 / X1R5G5B5
    uint32_t format;        // 0 = A4R4G4B4, 1 = A1R5G5B5, 2 = R5G6B5
  };

  class D3D9FormatHelper {

  public:

    D3D9FormatHelper(const Rc<DxvkDevice>& device);

    void Flush();

    /**
     * \brief Compute shader decoding packed 16-bit formats into BGRA8
     *
     * Bound on the caller's context (slots Packed16ImageSlot / Packed16BufferSlot);
     * the format is selected through D3D9Packed16Args::format.
     */
    const Rc<DxvkShader>& Packed16Shader() const { return m_packed16Shader; }

    enum Packed16Slots : uint32_t {
      Packed16ImageSlot  = 1200,
      Packed16BufferSlot = 1201,
    };

    void ConvertFormat(
            D3D9_CONVERSION_FORMAT_INFO   conversionFormat,
      const Rc<DxvkImage>&                dstImage,
            VkImageSubresourceLayers      dstSubresource,
      const DxvkBufferSlice&              srcSlice);

  private:

    void ConvertGenericFormat(
            D3D9_CONVERSION_FORMAT_INFO   videoFormat,
      const Rc<DxvkImage>&                dstImage,
            VkImageSubresourceLayers      dstSubresource,
      const DxvkBufferSlice&              srcSlice,
            VkFormat                      bufferFormat,
            uint32_t                      specConstantValue,
            VkExtent2D                    macroPixelRun);

    enum BindingIds : uint32_t {
      Image  = 0,
      Buffer = 1,
    };

    void InitShaders();

    Rc<DxvkShader> InitShader(SpirvCodeBuffer code);
    Rc<DxvkShader> InitPacked16Shader(SpirvCodeBuffer code);

    void FlushInternal();

    Rc<DxvkDevice>    m_device;
    Rc<DxvkContext>   m_context;

    size_t            m_transferCommands = 0;

    std::array<Rc<DxvkShader>, D3D9ConversionFormat_Count> m_shaders;
    Rc<DxvkShader>    m_packed16Shader;

  };
  
}