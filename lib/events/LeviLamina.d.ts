declare module "iListenAttentively" {
    export interface EventTypeMap {
        // command
        "_ll::event::ExecuteCommandEvent": MergeMultiple<[EventTypeMap["_ll::event::Event"], {
            minecraftCommands: SerializeTypeData<"MinecraftCommands", false, false>,
            commandContext: SerializeTypeData<"CommandContext", false, true>,
            suppressOutput: boolean
        }]>,
        "ll::event::ExecutingCommandEvent": MergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_ll::event::ExecuteCommandEvent"], {

        }]>,
        "ll::event::ExecutedCommandEvent": MergeMultiple<[EventTypeMap["ll::event::ExecuteCommandEvent"], {
            result: {
                success: boolean,
                code: number
            }
        }]>,

        // entity
        "_ll::event::ActorEvent": MergeMultiple<[EventTypeMap["_ll::event::Event"], {
            self: SerializeTypeData<"Actor", false, false>
        }]>,
        "_ll::event::MobEvent": MergeMultiple<[EventTypeMap["_ll::event::ActorEvent"], {
            self: SerializeTypeData<"Mob", false, false>
        }]>,
        "ll::event::ActorHurtEvent": MergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_ll::event::ActorEvent"], {
            source: SerializeTypeData<"ActorDamageSource", false, true>,
            damage: number
            knock: boolean,
            ignite: boolean
        }]>,
        "ll::event::MobDieEvent": MergeMultiple<[EventTypeMap["_ll::event::MobEvent"], {
            source: SerializeTypeData<"ActorDamageSource", false, true>,
        }]>,

        // player
        "_ll::event::PlayerEvent": MergeMultiple<[EventTypeMap["_ll::event::MobEvent"], {
            self: SerializeTypeData<"Player", false, false>
        }]>,
        "_ll::event::ServerPlayerEvent": MergeMultiple<[EventTypeMap["_ll::event::PlayerEvent"], {
            self: SerializeTypeData<"ServerPlayer", false, false>
        }]>,
        "_ll::event::PlayerClickEvent": MergeMultiple<[EventTypeMap["_ll::event::PlayerEvent"], {

        }]>,
        "_ll::event::PlayerRightClickEvent": MergeMultiple<[EventTypeMap["_ll::event::PlayerEvent"], {

        }]>,
        "_ll::event::PlayerLeftClickEvent": MergeMultiple<[EventTypeMap["_ll::event::PlayerEvent"], {

        }]>,
        "ll::event::PlayerAddExperienceEvent": MergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_ll::event::PlayerEvent"], {
            experience: number
        }]>,
        "ll::event::PlayerAttackEvent": MergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_ll::event::PlayerEvent"], {
            target: SerializeTypeData<"Actor", false, false>,
            case: string
        }]>,
        "ll::event::PlayerDieEvent": MergeMultiple<[EventTypeMap["_ll::event::PlayerEvent"], {
            source: SerializeTypeData<"ActorDamageSource", false, true>
        }]>,
        "ll::event::PlayerInteractBlockEvent": MergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_ll::event::PlayerEvent"], {
            item: SerializeTypeData<"ItemStack", false, false>,
            blockPos: [number, number, number],
            face: string,
            clickPos: [number, number, number],
            block: SerializeTypeData<"Block", true, true>
        }]>,
        "ll::event::PlayerJumpEvent": MergeMultiple<[EventTypeMap["_ll::event::PlayerEvent"], {

        }]>,
        "ll::event::PlayerPickUpItemEvent": MergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_ll::event::PlayerEvent"], {
            item: SerializeTypeData<"ItemActor", false, false>,
            orgCount: number,
            favoredSlot: number
        }]>,
        "_ll::event::PlayerPlaceBlockEvent": MergeMultiple<[EventTypeMap["_ll::event::PlayerEvent"], {
            pos: [number, number, number],
        }]>,
        "ll::event::PlayerPlacingBlockEvent": MergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_ll::event::PlayerPlaceBlockEvent"], {
            face: number
        }]>,
        "ll::event::PlayerPlacedBlockEvent": MergeMultiple<[EventTypeMap["_ll::event::PlayerPlaceBlockEvent"], {
            placedBlock: SerializeTypeData<"Block", false, true>
        }]>,
        "ll::event::PlayerRespawnEvent": MergeMultiple<[EventTypeMap["_ll::event::PlayerEvent"], {

        }]>,
        "ll::event::PlayerUseItemEvent": MergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_ll::event::PlayerEvent"], {
            item: SerializeTypeData<"ItemStack", false, false>
        }]>,
        "ll::event::PlayerChangePermEvent": MergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_ll::event::PlayerEvent"], {
            permission: string
        }]>,
        "ll::event::PlayerChatEvent": MergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_ll::event::PlayerEvent"], {
            message: string
        }]>,
        "ll::event::PlayerConnectEvent": MergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_ll::event::ServerPlayerEvent"], {
            networkIdentifier: SerializeTypeData<"NetworkIdentifier", false, true>,
            connectionRequest: SerializeTypeData<"ConnectionRequest", false, true>
        }]>,
        "ll::event::PlayerDestroyBlockEvent": MergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_ll::event::PlayerLeftClickEvent"], {
            pos: [number, number, number],
        }]>,
        "ll::event::PlayerDisconnectEvent": MergeMultiple<[EventTypeMap["_ll::event::ServerPlayerEvent"], {

        }]>,
        "ll::event::PlayerJoinEvent": MergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_ll::event::ServerPlayerEvent"], {

        }]>,
        "_ll::event::PlayerSneakEvent": MergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_ll::event::ServerPlayerEvent"], {

        }]>,
        "ll::event::PlayerSneakingEvent": MergeMultiple<[EventTypeMap["_ll::event::PlayerSneakEvent"], {

        }]>,
        "ll::event::PlayerSneakedEvent": MergeMultiple<[EventTypeMap["_ll::event::PlayerSneakEvent"], {

        }]>,
        "_ll::event::PlayerSprintEvent": MergeMultiple<[EventTypeMap["_ll::event::ServerPlayerEvent"], {

        }]>,
        "ll::event::PlayerSprintingEvent": MergeMultiple<[EventTypeMap["_ll::event::PlayerSprintEvent"], {

        }]>,
        "ll::event::PlayerSprintedEvent": MergeMultiple<[EventTypeMap["_ll::event::PlayerSprintEvent"], {

        }]>,
        "ll::event::PlayerSwingEvent": MergeMultiple<[EventTypeMap["_ll::event::PlayerLeftClickEvent"], {

        }]>,

        // service
        "_ll::event::ServiceEvent": MergeMultiple<[EventTypeMap["_ll::event::Event"], {

        }]>,
        "ll::event::ServiceRegisterEvent": MergeMultiple<[EventTypeMap["_ll::event::ServiceEvent"], {

        }]>,
        "ll::event::ServiceUnregisterEvent": MergeMultiple<[EventTypeMap["_ll::event::ServiceEvent"], {

        }]>,

        // server
        "ll::event::ServerStartedEvent": MergeMultiple<[EventTypeMap, {
            server: SerializeTypeData<"ServerInstance", false, false>
        }]>,
        "ll::event::ServerStoppingEvent": MergeMultiple<[EventTypeMap, {
            server: SerializeTypeData<"ServerInstance", false, false>
        }]>,

        // world
        "_ll::event::WorldEvent": MergeMultiple<[EventTypeMap["_ll::event::Event"], {
            blockSource: SerializeTypeData<"BlockSource", false, false>
        }]>,
        "_ll::event::LevelEvent": MergeMultiple<[EventTypeMap["_ll::event::Event"], {
            level: SerializeTypeData<"Level", false, false>
        }]>,
        "ll::event::BlockChangedEvent": MergeMultiple<[EventTypeMap["_ll::event::WorldEvent"], {
            layer: number,
            previousBlock: SerializeTypeData<"Block", false, true>,
            newBlock: SerializeTypeData<"Block", false, true>,
            pos: [number, number, number]
        }]>,
        "ll::event::FireSpreadEvent": MergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_ll::event::WorldEvent"], {
            pos: [number, number, number],
        }]>,
        "ll::event::LevelTickEvent": MergeMultiple<[EventTypeMap["_ll::event::LevelEvent"], {

        }]>,
        "_ll::event::SpawnMobEvent": MergeMultiple<[EventTypeMap["_ll::event::WorldEvent"], {
            identifier: SerializeTypeData<"ActorDefinitionIdentifier", false, true>,
            spawner?: number,
            pos: [number, number, number],
            naturalSpawn: boolean,
            surface: boolean,
            fromSpawner: boolean
        }]>,
        "ll::event::SpawningMobEvent": MergeMultiple<[EventTypeMap["_ll::event::Cancellable"], EventTypeMap["_ll::event::SpawnMobEvent"], {

        }]>,
        "ll::event::SpawnedMobEvent": MergeMultiple<[EventTypeMap["_ll::event::SpawnMobEvent"], {

        }]>
    };
}