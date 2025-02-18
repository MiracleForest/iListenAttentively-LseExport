#pragma once
#include <ll/api/event/Cancellable.h>

namespace ila::event {

class LseEvent final : public ll::event::Cancellable<ll::event::Event> {
protected:
    CompoundTag* mData;

public:
    constexpr explicit LseEvent(CompoundTag* data) : Cancellable(), mData(data) {}

    void serialize(CompoundTag& nbt) const override;
    void deserialize(CompoundTag const& nbt) override;

    CompoundTag* getData() const;
}; // class MoneyChangEvent

} // namespace ila::lse