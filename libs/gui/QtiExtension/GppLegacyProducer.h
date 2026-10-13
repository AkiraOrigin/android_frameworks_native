/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <gui/IGraphicBufferProducer.h>

namespace android::libguiextension {

// Wraps the output IGraphicBufferProducer that is handed to gppservice.
//
// The gppservice shipped on SM8650 OPLUS devices uses a libgui from Android 16, whose
// QueueBufferOutput has no bufferReplacedFrameId. Every call is forwarded to the wrapped
// producer as is, only the QueueBufferOutput replies of CONNECT and QUEUE_BUFFER are
// written in the layout that libgui expects. Otherwise it misreads them and fails every
// queueBuffer with NO_MEMORY.
class GppLegacyProducer : public BnGraphicBufferProducer {
public:
    explicit GppLegacyProducer(const sp<IGraphicBufferProducer>& base) : mBase(base) {}

    status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply,
                        uint32_t flags = 0) override;

    status_t getConfigForSurface(SurfaceConfig* outConfig) override;
    status_t requestBuffer(int slot, sp<GraphicBuffer>* buf) override;
    status_t extendSlotCount(int size) override;
    status_t setMaxDequeuedBufferCount(int maxDequeuedBuffers) override;
    status_t setAsyncMode(bool async) override;
    status_t dequeueBuffer(int* slot, sp<Fence>* fence, uint32_t w, uint32_t h,
                           PixelFormat format, uint64_t usage, uint64_t* outBufferAge,
                           FrameEventHistoryDelta* outTimestamps) override;
    status_t detachBuffer(int slot) override;
    status_t detachNextBuffer(sp<GraphicBuffer>* outBuffer, sp<Fence>* outFence) override;
    status_t attachBuffer(int* outSlot, const sp<GraphicBuffer>& buffer) override;
    status_t queueBuffer(int slot, const QueueBufferInput& input,
                         QueueBufferOutput* output) override;
    status_t cancelBuffer(int slot, const sp<Fence>& fence) override;
    int query(int what, int* value) override;
    status_t connect(const sp<IProducerListener>& listener, int api,
                     bool producerControlledByApp, QueueBufferOutput* output) override;
    status_t disconnect(int api, DisconnectMode mode = DisconnectMode::Api) override;
    status_t setSidebandStream(const sp<NativeHandle>& stream) override;
    void allocateBuffers(uint32_t width, uint32_t height, PixelFormat format,
                         uint64_t usage) override;
    status_t allowAllocation(bool allow) override;
    status_t setGenerationNumber(uint32_t generationNumber) override;
    String8 getConsumerName() const override;
    status_t setSharedBufferMode(bool sharedBufferMode) override;
    status_t setAutoRefresh(bool autoRefresh) override;
    status_t setDequeueTimeout(nsecs_t timeout) override;
    status_t setLegacyBufferDrop(bool drop) override;
    status_t getLastQueuedBuffer(sp<GraphicBuffer>* outBuffer, sp<Fence>* outFence,
                                 float outTransformMatrix[16]) override;
    status_t getLastQueuedBuffer(sp<GraphicBuffer>* outBuffer, sp<Fence>* outFence,
                                 Rect* outRect, uint32_t* outTransform) override;
    void getFrameTimestamps(FrameEventHistoryDelta* outDelta) override;
    status_t getUniqueId(uint64_t* outId) const override;
    status_t getConsumerUsage(uint64_t* outUsage) const override;
    status_t setAutoPrerotation(bool autoPrerotation) override;
    status_t setFrameRate(float frameRate, int8_t compatibility,
                          int8_t changeFrameRateStrategy) override;
    status_t setProducerThrottlingEnabled(bool enabled) override;
    status_t isProducerThrottlingEnabled(bool* outEnabled) const override;
#if COM_ANDROID_GRAPHICS_LIBGUI_FLAGS(BQ_EXTENDEDALLOCATE)
    status_t setAdditionalOptions(const std::vector<gui::AdditionalOptions>& options) override;
#endif
    status_t setPresentMode(int32_t mode) override;
    status_t requestBuffers(const std::vector<int32_t>& slots,
                            std::vector<RequestBufferOutput>* outputs) override;
    status_t dequeueBuffers(const std::vector<DequeueBufferInput>& inputs,
                            std::vector<DequeueBufferOutput>* outputs) override;
    status_t detachBuffers(const std::vector<int32_t>& slots,
                           std::vector<status_t>* results) override;
    status_t attachBuffers(const std::vector<sp<GraphicBuffer>>& buffers,
                           std::vector<AttachBufferOutput>* outputs) override;
    status_t queueBuffers(const std::vector<QueueBufferInput>& inputs,
                          std::vector<QueueBufferOutput>* outputs) override;
    status_t cancelBuffers(const std::vector<CancelBufferInput>& inputs,
                           std::vector<status_t>* results) override;
    status_t query(const std::vector<int32_t>& inputs,
                   std::vector<QueryOutput>* outputs) override;

private:
    const sp<IGraphicBufferProducer> mBase;
};

} // namespace android::libguiextension
