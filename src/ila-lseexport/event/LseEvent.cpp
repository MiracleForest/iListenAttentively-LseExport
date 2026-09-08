#include "ila-lseexport/event/LseEvent.h"
#include <ll/api/event/Emitter.h>

#if __has_include(<mc/nbt/CompoundTagVariant.h>)
#  include <mc/nbt/CompoundTagVariant.h>
#elif __has_include(<mc/deps/nbt/CompoundTagVariant.h>)
#  include <mc/deps/nbt/CompoundTagVariant.h>
#else
#  error "CompoundTagVariant not found"
#endif

namespace mif::ila_lseexport::event {

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

} // namespace mif::ila_lseexport::event