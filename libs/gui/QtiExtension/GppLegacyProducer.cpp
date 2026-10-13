/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#undef LOG_TAG
#define LOG_TAG "SurfaceExtension-GPP"

#include "GppLegacyProducer.h"

#include <binder/Parcel.h>
#include <gui/IProducerListener.h>
#include <log/log.h>
#include <utils/Flattenable.h>

namespace android::libguiextension {

namespace {

// Transaction codes from IGraphicBufferProducer.cpp, which are the same in Android 16.
constexpr uint32_t kConnect = IBinder::FIRST_CALL_TRANSACTION + 8;
constexpr uint32_t kQueueBuffer = IBinder::FIRST_CALL_TRANSACTION + 5;

// QueueBufferOutput as flattened by Android 16 libgui: the same fields without
// bufferReplacedFrameId, which Android 17 inserted after bufferReplaced.
class LegacyQueueBufferOutput : public Flattenable<LegacyQueueBufferOutput> {
public:
    explicit LegacyQueueBufferOutput(const IGraphicBufferProducer::QueueBufferOutput& output)
          : mOutput(output) {}

    size_t getFlattenedSize() const {
        return sizeof(mOutput.width) + sizeof(mOutput.height) + sizeof(mOutput.transformHint) +
                sizeof(mOutput.numPendingBuffers) + sizeof(mOutput.nextFrameNumber) +
                sizeof(mOutput.bufferReplaced) + sizeof(mOutput.maxBufferCount) +
                sizeof(mOutput.isSlotExpansionAllowed) + sizeof(mOutput.result) +
                mOutput.frameTimestamps.getFlattenedSize();
    }

    size_t getFdCount() const { return mOutput.frameTimestamps.getFdCount(); }

    status_t flatten(void*& buffer, size_t& size, int*& fds, size_t& count) const {
        if (size < getFlattenedSize()) {
            return NO_MEMORY;
        }

        FlattenableUtils::write(buffer, size, mOutput.width);
        FlattenableUtils::write(buffer, size, mOutput.height);
        FlattenableUtils::write(buffer, size, mOutput.transformHint);
        FlattenableUtils::write(buffer, size, mOutput.numPendingBuffers);
        FlattenableUtils::write(buffer, size, mOutput.nextFrameNumber);
        FlattenableUtils::write(buffer, size, mOutput.bufferReplaced);
        FlattenableUtils::write(buffer, size, mOutput.maxBufferCount);
        FlattenableUtils::write(buffer, size, mOutput.isSlotExpansionAllowed);

        status_t result = mOutput.frameTimestamps.flatten(buffer, size, fds, count);
        if (result != NO_ERROR) {
            return result;
        }
        FlattenableUtils::write(buffer, size, mOutput.result);
        return NO_ERROR;
    }

    status_t unflatten(void const*& /*buffer*/, size_t& /*size*/, int const*& /*fds*/,
                       size_t& /*count*/) {
        return INVALID_OPERATION;
    }

private:
    const IGraphicBufferProducer::QueueBufferOutput& mOutput;
};

} // namespace

status_t GppLegacyProducer::onTransact(uint32_t code, const Parcel& data, Parcel* reply,
                                       uint32_t flags) {
    switch (code) {
        case kConnect: {
            CHECK_INTERFACE(IGraphicBufferProducer, data, reply);
            sp<IProducerListener> listener;
            if (data.readInt32() == 1) {
                listener = IProducerListener::asInterface(data.readStrongBinder());
            }
            int api = data.readInt32();
            bool producerControlledByApp = data.readInt32();
            QueueBufferOutput output;
            status_t res = connect(listener, api, producerControlledByApp, &output);
            reply->write(LegacyQueueBufferOutput(output));
            reply->writeInt32(res);
            return NO_ERROR;
        }
        case kQueueBuffer: {
            CHECK_INTERFACE(IGraphicBufferProducer, data, reply);
            int buf = data.readInt32();
            QueueBufferInput input(data);
            QueueBufferOutput output;
            status_t result = queueBuffer(buf, input, &output);
            reply->write(LegacyQueueBufferOutput(output));
            reply->writeInt32(result);
            return NO_ERROR;
        }
        default:
            return BnGraphicBufferProducer::onTransact(code, data, reply, flags);
    }
}

status_t GppLegacyProducer::getConfigForSurface(SurfaceConfig* outConfig) {
    return mBase->getConfigForSurface(outConfig);
}

status_t GppLegacyProducer::requestBuffer(int slot, sp<GraphicBuffer>* buf) {
    return mBase->requestBuffer(slot, buf);
}

status_t GppLegacyProducer::extendSlotCount(int size) {
    return mBase->extendSlotCount(size);
}

status_t GppLegacyProducer::setMaxDequeuedBufferCount(int maxDequeuedBuffers) {
    return mBase->setMaxDequeuedBufferCount(maxDequeuedBuffers);
}

status_t GppLegacyProducer::setAsyncMode(bool async) {
    return mBase->setAsyncMode(async);
}

status_t GppLegacyProducer::dequeueBuffer(int* slot, sp<Fence>* fence, uint32_t w, uint32_t h,
                                          PixelFormat format, uint64_t usage,
                                          uint64_t* outBufferAge,
                                          FrameEventHistoryDelta* outTimestamps) {
    return mBase->dequeueBuffer(slot, fence, w, h, format, usage, outBufferAge, outTimestamps);
}

status_t GppLegacyProducer::detachBuffer(int slot) {
    return mBase->detachBuffer(slot);
}

status_t GppLegacyProducer::detachNextBuffer(sp<GraphicBuffer>* outBuffer, sp<Fence>* outFence) {
    return mBase->detachNextBuffer(outBuffer, outFence);
}

status_t GppLegacyProducer::attachBuffer(int* outSlot, const sp<GraphicBuffer>& buffer) {
    return mBase->attachBuffer(outSlot, buffer);
}

status_t GppLegacyProducer::queueBuffer(int slot, const QueueBufferInput& input,
                                        QueueBufferOutput* output) {
    return mBase->queueBuffer(slot, input, output);
}

status_t GppLegacyProducer::cancelBuffer(int slot, const sp<Fence>& fence) {
    return mBase->cancelBuffer(slot, fence);
}

int GppLegacyProducer::query(int what, int* value) {
    return mBase->query(what, value);
}

status_t GppLegacyProducer::connect(const sp<IProducerListener>& listener, int api,
                                    bool producerControlledByApp, QueueBufferOutput* output) {
    return mBase->connect(listener, api, producerControlledByApp, output);
}

status_t GppLegacyProducer::disconnect(int api, DisconnectMode mode) {
    return mBase->disconnect(api, mode);
}

status_t GppLegacyProducer::setSidebandStream(const sp<NativeHandle>& stream) {
    return mBase->setSidebandStream(stream);
}

void GppLegacyProducer::allocateBuffers(uint32_t width, uint32_t height, PixelFormat format,
                                        uint64_t usage) {
    mBase->allocateBuffers(width, height, format, usage);
}

status_t GppLegacyProducer::allowAllocation(bool allow) {
    return mBase->allowAllocation(allow);
}

status_t GppLegacyProducer::setGenerationNumber(uint32_t generationNumber) {
    return mBase->setGenerationNumber(generationNumber);
}

String8 GppLegacyProducer::getConsumerName() const {
    return mBase->getConsumerName();
}

status_t GppLegacyProducer::setSharedBufferMode(bool sharedBufferMode) {
    return mBase->setSharedBufferMode(sharedBufferMode);
}

status_t GppLegacyProducer::setAutoRefresh(bool autoRefresh) {
    return mBase->setAutoRefresh(autoRefresh);
}

status_t GppLegacyProducer::setDequeueTimeout(nsecs_t timeout) {
    return mBase->setDequeueTimeout(timeout);
}

status_t GppLegacyProducer::setLegacyBufferDrop(bool drop) {
    return mBase->setLegacyBufferDrop(drop);
}

status_t GppLegacyProducer::getLastQueuedBuffer(sp<GraphicBuffer>* outBuffer, sp<Fence>* outFence,
                                                float outTransformMatrix[16]) {
    return mBase->getLastQueuedBuffer(outBuffer, outFence, outTransformMatrix);
}

status_t GppLegacyProducer::getLastQueuedBuffer(sp<GraphicBuffer>* outBuffer, sp<Fence>* outFence,
                                                Rect* outRect, uint32_t* outTransform) {
    return mBase->getLastQueuedBuffer(outBuffer, outFence, outRect, outTransform);
}

void GppLegacyProducer::getFrameTimestamps(FrameEventHistoryDelta* outDelta) {
    mBase->getFrameTimestamps(outDelta);
}

status_t GppLegacyProducer::getUniqueId(uint64_t* outId) const {
    return mBase->getUniqueId(outId);
}

status_t GppLegacyProducer::getConsumerUsage(uint64_t* outUsage) const {
    return mBase->getConsumerUsage(outUsage);
}

status_t GppLegacyProducer::setAutoPrerotation(bool autoPrerotation) {
    return mBase->setAutoPrerotation(autoPrerotation);
}

status_t GppLegacyProducer::setFrameRate(float frameRate, int8_t compatibility,
                                         int8_t changeFrameRateStrategy) {
    return mBase->setFrameRate(frameRate, compatibility, changeFrameRateStrategy);
}

status_t GppLegacyProducer::setProducerThrottlingEnabled(bool enabled) {
    return mBase->setProducerThrottlingEnabled(enabled);
}

status_t GppLegacyProducer::isProducerThrottlingEnabled(bool* outEnabled) const {
    return mBase->isProducerThrottlingEnabled(outEnabled);
}

#if COM_ANDROID_GRAPHICS_LIBGUI_FLAGS(BQ_EXTENDEDALLOCATE)
status_t GppLegacyProducer::setAdditionalOptions(
        const std::vector<gui::AdditionalOptions>& options) {
    return mBase->setAdditionalOptions(options);
}
#endif

status_t GppLegacyProducer::setPresentMode(int32_t mode) {
    return mBase->setPresentMode(mode);
}

status_t GppLegacyProducer::requestBuffers(const std::vector<int32_t>& slots,
                                           std::vector<RequestBufferOutput>* outputs) {
    return mBase->requestBuffers(slots, outputs);
}

status_t GppLegacyProducer::dequeueBuffers(const std::vector<DequeueBufferInput>& inputs,
                                           std::vector<DequeueBufferOutput>* outputs) {
    return mBase->dequeueBuffers(inputs, outputs);
}

status_t GppLegacyProducer::detachBuffers(const std::vector<int32_t>& slots,
                                          std::vector<status_t>* results) {
    return mBase->detachBuffers(slots, results);
}

status_t GppLegacyProducer::attachBuffers(const std::vector<sp<GraphicBuffer>>& buffers,
                                          std::vector<AttachBufferOutput>* outputs) {
    return mBase->attachBuffers(buffers, outputs);
}

status_t GppLegacyProducer::queueBuffers(const std::vector<QueueBufferInput>& inputs,
                                         std::vector<QueueBufferOutput>* outputs) {
    return mBase->queueBuffers(inputs, outputs);
}

status_t GppLegacyProducer::cancelBuffers(const std::vector<CancelBufferInput>& inputs,
                                          std::vector<status_t>* results) {
    return mBase->cancelBuffers(inputs, results);
}

status_t GppLegacyProducer::query(const std::vector<int32_t>& inputs,
                                  std::vector<QueryOutput>* outputs) {
    return mBase->query(inputs, outputs);
}

} // namespace android::libguiextension
