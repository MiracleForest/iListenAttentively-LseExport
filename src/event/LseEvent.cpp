#pragma once
#include "LseEvent.h"
#include <ll/api/event/Emitter.h>
#include <mc/nbt/CompoundTag.h>

namespace ila::event {

void LseEvent::serialize(CompoundTag& nbt) const {
    Cancellable::serialize(nbt);
    if (!getData()) return;
    for (auto& [key, value] : getData()->mTags) {
        if (key == "eventId") continue;
        nbt[key] = value;
    }
}

void LseEvent::deserialize(CompoundTag const& nbt) {
    Cancellable::deserialize(nbt);
    if (!getData()) return;
    for (auto& [key, value] : nbt.mTags) {
        if (key == "eventId") continue;
        (*getData())[key] = value;
    }
}

ll::event::EventId LseEvent::getId() const { return ll::event::EventId(mEventName); }
CompoundTag*       LseEvent::getData() const { return mData; }

class LseEventEmitter : public ll::event::Emitter<[](auto&&...) { return nullptr; }, LseEvent> {};

} // namespace ila::event