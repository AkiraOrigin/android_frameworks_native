/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <gui/IGraphicBufferProducer.h>

namespace android::libguiextension {

// The gppservice shipped on SM8650 OPLUS devices uses a libgui from Android 16, whose
// QueueBufferOutput has no bufferReplacedFrameId. The CONNECT and QUEUE_BUFFER replies are
// the only IGraphicBufferProducer parcels that differ, in both directions:
// - GppLegacyProducer is handed to gppservice as the output of the GPP session and writes
//   those replies in the old layout.
// - GppLegacySessionProducer wraps the producer of the GPP session the app renders into
//   and reads those replies in the old layout.
// Everything else is forwarded as is.

template <typename Base>
class GppForwardingProducer : public Base {
public:
    explicit GppForwardingProducer(const sp<IGraphicBufferProducer>& base) : mBase(base) {}

    using QueueBufferInput = IGraphicBufferProducer::QueueBufferInput;
    using QueueBufferOutput = IGraphicBufferProducer::QueueBufferOutput;
    using DisconnectMode = IGraphicBufferProducer::DisconnectMode;
    using RequestBufferOutput = IGraphicBufferProducer::RequestBufferOutput;
    using DequeueBufferInput = IGraphicBufferProducer::DequeueBufferInput;
    using DequeueBufferOutput = IGraphicBufferProducer::DequeueBufferOutput;
    using AttachBufferOutput = IGraphicBufferProducer::AttachBufferOutput;
    using CancelBufferInput = IGraphicBufferProducer::CancelBufferInput;
    using QueryOutput = IGraphicBufferProducer::QueryOutput;
    using SurfaceConfig = IGraphicBufferProducer::SurfaceConfig;

    status_t getConfigForSurface(SurfaceConfig* outConfig) override {
        return mBase->getConfigForSurface(outConfig);
    }
    status_t requestBuffer(int slot, sp<GraphicBuffer>* buf) override {
        return mBase->requestBuffer(slot, buf);
    }
    status_t extendSlotCount(int size) override { return mBase->extendSlotCount(size); }
    status_t setMaxDequeuedBufferCount(int maxDequeuedBuffers) override {
        return mBase->setMaxDequeuedBufferCount(maxDequeuedBuffers);
    }
    status_t setAsyncMode(bool async) override { return mBase->setAsyncMode(async); }
    status_t dequeueBuffer(int* slot, sp<Fence>* fence, uint32_t w, uint32_t h,
                           PixelFormat format, uint64_t usage, uint64_t* outBufferAge,
                           FrameEventHistoryDelta* outTimestamps) override {
        return mBase->dequeueBuffer(slot, fence, w, h, format, usage, outBufferAge,
                                    outTimestamps);
    }
    status_t detachBuffer(int slot) override { return mBase->detachBuffer(slot); }
    status_t detachNextBuffer(sp<GraphicBuffer>* outBuffer, sp<Fence>* outFence) override {
        return mBase->detachNextBuffer(outBuffer, outFence);
    }
    status_t attachBuffer(int* outSlot, const sp<GraphicBuffer>& buffer) override {
        return mBase->attachBuffer(outSlot, buffer);
    }
    status_t queueBuffer(int slot, const QueueBufferInput& input,
                         QueueBufferOutput* output) override {
        return mBase->queueBuffer(slot, input, output);
    }
    status_t cancelBuffer(int slot, const sp<Fence>& fence) override {
        return mBase->cancelBuffer(slot, fence);
    }
    int query(int what, int* value) override { return mBase->query(what, value); }
    status_t connect(const sp<IProducerListener>& listener, int api,
                     bool producerControlledByApp, QueueBufferOutput* output) override {
        return mBase->connect(listener, api, producerControlledByApp, output);
    }
    status_t disconnect(int api, DisconnectMode mode = DisconnectMode::Api) override {
        return mBase->disconnect(api, mode);
    }
    status_t setSidebandStream(const sp<NativeHandle>& stream) override {
        return mBase->setSidebandStream(stream);
    }
    void allocateBuffers(uint32_t width, uint32_t height, PixelFormat format,
                         uint64_t usage) override {
        mBase->allocateBuffers(width, height, format, usage);
    }
    status_t allowAllocation(bool allow) override { return mBase->allowAllocation(allow); }
    status_t setGenerationNumber(uint32_t generationNumber) override {
        return mBase->setGenerationNumber(generationNumber);
    }
    String8 getConsumerName() const override { return mBase->getConsumerName(); }
    status_t setSharedBufferMode(bool sharedBufferMode) override {
        return mBase->setSharedBufferMode(sharedBufferMode);
    }
    status_t setAutoRefresh(bool autoRefresh) override {
        return mBase->setAutoRefresh(autoRefresh);
    }
    status_t setDequeueTimeout(nsecs_t timeout) override {
        return mBase->setDequeueTimeout(timeout);
    }
    status_t setLegacyBufferDrop(bool drop) override { return mBase->setLegacyBufferDrop(drop); }
    status_t getLastQueuedBuffer(sp<GraphicBuffer>* outBuffer, sp<Fence>* outFence,
                                 float outTransformMatrix[16]) override {
        return mBase->getLastQueuedBuffer(outBuffer, outFence, outTransformMatrix);
    }
    status_t getLastQueuedBuffer(sp<GraphicBuffer>* outBuffer, sp<Fence>* outFence,
                                 Rect* outRect, uint32_t* outTransform) override {
        return mBase->getLastQueuedBuffer(outBuffer, outFence, outRect, outTransform);
    }
    void getFrameTimestamps(FrameEventHistoryDelta* outDelta) override {
        mBase->getFrameTimestamps(outDelta);
    }
    status_t getUniqueId(uint64_t* outId) const override { return mBase->getUniqueId(outId); }
    status_t getConsumerUsage(uint64_t* outUsage) const override {
        return mBase->getConsumerUsage(outUsage);
    }
    status_t setAutoPrerotation(bool autoPrerotation) override {
        return mBase->setAutoPrerotation(autoPrerotation);
    }
    status_t setFrameRate(float frameRate, int8_t compatibility,
                          int8_t changeFrameRateStrategy) override {
        return mBase->setFrameRate(frameRate, compatibility, changeFrameRateStrategy);
    }
    status_t setProducerThrottlingEnabled(bool enabled) override {
        return mBase->setProducerThrottlingEnabled(enabled);
    }
    status_t isProducerThrottlingEnabled(bool* outEnabled) const override {
        return mBase->isProducerThrottlingEnabled(outEnabled);
    }
#if COM_ANDROID_GRAPHICS_LIBGUI_FLAGS(BQ_EXTENDEDALLOCATE)
    status_t setAdditionalOptions(const std::vector<gui::AdditionalOptions>& options) override {
        return mBase->setAdditionalOptions(options);
    }
#endif
    status_t setPresentMode(int32_t mode) override { return mBase->setPresentMode(mode); }
    status_t requestBuffers(const std::vector<int32_t>& slots,
                            std::vector<RequestBufferOutput>* outputs) override {
        return mBase->requestBuffers(slots, outputs);
    }
    status_t dequeueBuffers(const std::vector<DequeueBufferInput>& inputs,
                            std::vector<DequeueBufferOutput>* outputs) override {
        return mBase->dequeueBuffers(inputs, outputs);
    }
    status_t detachBuffers(const std::vector<int32_t>& slots,
                           std::vector<status_t>* results) override {
        return mBase->detachBuffers(slots, results);
    }
    status_t attachBuffers(const std::vector<sp<GraphicBuffer>>& buffers,
                           std::vector<AttachBufferOutput>* outputs) override {
        return mBase->attachBuffers(buffers, outputs);
    }
    status_t queueBuffers(const std::vector<QueueBufferInput>& inputs,
                          std::vector<QueueBufferOutput>* outputs) override {
        return mBase->queueBuffers(inputs, outputs);
    }
    status_t cancelBuffers(const std::vector<CancelBufferInput>& inputs,
                           std::vector<status_t>* results) override {
        return mBase->cancelBuffers(inputs, results);
    }
    status_t query(const std::vector<int32_t>& inputs,
                   std::vector<QueryOutput>* outputs) override {
        return mBase->query(inputs, outputs);
    }

protected:
    const sp<IGraphicBufferProducer> mBase;
};

// The output producer handed to gppservice. Writes the CONNECT and QUEUE_BUFFER replies in
// the old layout, otherwise gppservice misreads them and fails every queueBuffer with
// NO_MEMORY.
class GppLegacyProducer : public GppForwardingProducer<BnGraphicBufferProducer> {
public:
    using GppForwardingProducer::GppForwardingProducer;

    status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply,
                        uint32_t flags = 0) override;
};

// The producer of the GPP session the app renders into. Reads the CONNECT and QUEUE_BUFFER
// replies in the old layout, otherwise Surface picks up a bogus maxBufferCount and slot
// expansion state from them and its buffers stop matching the ones of the swapchain.
class GppLegacySessionProducer : public GppForwardingProducer<IGraphicBufferProducer> {
public:
    using GppForwardingProducer::GppForwardingProducer;

    status_t connect(const sp<IProducerListener>& listener, int api,
                     bool producerControlledByApp, QueueBufferOutput* output) override;
    status_t queueBuffer(int slot, const QueueBufferInput& input,
                         QueueBufferOutput* output) override;

protected:
    IBinder* onAsBinder() override { return IInterface::asBinder(mBase).get(); }
};

} // namespace android::libguiextension
