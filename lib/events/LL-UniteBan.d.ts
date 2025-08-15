declare module "iListenAttentively" {
    export interface EventTypeMap {
        "_unite_ban::event::CheckBanBaseEvent": MergeMultiple<[EventTypeMap["_ll::event::Event"], {
            source: SerializeTypeData<"NetworkIdentifier", false, true>,
            request: SerializeTypeData<"ConnectionRequest", false, true>,
            requestData: {
                name: string,
                xuid: string,
                uuid: string,
                clientid: string,
                pfid: string,
                ip: string,
                language: string
            }
        }]>,
        "unite_ban::event::TryCheckBanEvent": MergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_unite_ban::event::CheckBanBaseEvent"], {

        }]>,
        "unite_ban::event::CheckBanPassEvent": MergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_unite_ban::event::CheckBanBaseEvent"], {

        }]>,
        "unite_ban::event::CheckBanFailEvent": MergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_unite_ban::event::CheckBanBaseEvent"], {
            reason: string
        }]>
        "unite_ban::event::CheckBanErrorEvent": MergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_unite_ban::event::CheckBanBaseEvent"], {
            error: string
        }]>
    };
}