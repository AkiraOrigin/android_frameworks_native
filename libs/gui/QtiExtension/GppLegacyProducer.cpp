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
    explicit LegacyQueueBufferOutput(IGraphicBufferProducer::QueueBufferOutput& output)
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

    status_t unflatten(void const*& buffer, size_t& size, int const*& fds, size_t& count) {
        if (size < getFlattenedSize() - mOutput.frameTimestamps.getFlattenedSize()) {
            return NO_MEMORY;
        }

        FlattenableUtils::read(buffer, size, mOutput.width);
        FlattenableUtils::read(buffer, size, mOutput.height);
        FlattenableUtils::read(buffer, size, mOutput.transformHint);
        FlattenableUtils::read(buffer, size, mOutput.numPendingBuffers);
        FlattenableUtils::read(buffer, size, mOutput.nextFrameNumber);
        FlattenableUtils::read(buffer, size, mOutput.bufferReplaced);
        FlattenableUtils::read(buffer, size, mOutput.maxBufferCount);
        FlattenableUtils::read(buffer, size, mOutput.isSlotExpansionAllowed);
        mOutput.bufferReplacedFrameId = 0;

        status_t result = mOutput.frameTimestamps.unflatten(buffer, size, fds, count);
        if (result != NO_ERROR) {
            return result;
        }
        FlattenableUtils::read(buffer, size, mOutput.result);
        return NO_ERROR;
    }

private:
    IGraphicBufferProducer::QueueBufferOutput& mOutput;
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

status_t GppLegacySessionProducer::connect(const sp<IProducerListener>& listener, int api,
                                           bool producerControlledByApp,
                                           QueueBufferOutput* output) {
    Parcel data, reply;
    data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
    if (listener != nullptr) {
        data.writeInt32(1);
        data.writeStrongBinder(IInterface::asBinder(listener));
    } else {
        data.writeInt32(0);
    }
    data.writeInt32(api);
    data.writeInt32(producerControlledByApp);
    status_t result = IInterface::asBinder(mBase)->transact(kConnect, data, &reply);
    if (result != NO_ERROR) {
        return result;
    }
    LegacyQueueBufferOutput legacyOutput(*output);
    reply.read(legacyOutput);
    return reply.readInt32();
}

status_t GppLegacySessionProducer::queueBuffer(int slot, const QueueBufferInput& input,
                                               QueueBufferOutput* output) {
    Parcel data, reply;
    data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
    data.writeInt32(slot);
    data.write(input);
    status_t result = IInterface::asBinder(mBase)->transact(kQueueBuffer, data, &reply);
    if (result != NO_ERROR) {
        return result;
    }
    LegacyQueueBufferOutput legacyOutput(*output);
    result = reply.read(legacyOutput);
    if (result != NO_ERROR) {
        return result;
    }
    return reply.readInt32();
}

} // namespace android::libguiextension
