#pragma once
#include <ll/api/event/Cancellable.h>

namespace ila::event {

class LseEvent final : public ll::event::Cancellable<ll::event::Event> {
protected:
    std::string const& mEventName;
    CompoundTag*       mData;

public:
    constexpr explicit LseEvent(std::string const& eventName, CompoundTag* data)
    : Cancellable(),
      mEventName(eventName),
      mData(data) {}

    void serialize(CompoundTag& nbt) const override;
    void deserialize(CompoundTag const& nbt) override;

    ll::event::EventId getId() const override;
    CompoundTag*       getData() const;
};

} // namespace ila::event