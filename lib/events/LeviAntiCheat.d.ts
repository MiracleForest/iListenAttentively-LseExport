declare module "iListenAttentively" {
    export interface EventTypeMap {
        "lac::punish::PlayerBanWaveEvent": MeMergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_ll::event::PlayerEvent"], {

        }]>,
        "lac::punish::PlayerCheatEvent": MeMergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_ll::event::PlayerEvent"], {

        }]>,
        "lac::punish::SusClientEvent": MeMergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_ll::event::PlayerEvent"], {

        }]>,
    };
}