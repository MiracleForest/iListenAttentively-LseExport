/** 监听ID */
type ListenerId = number;
/** 事件名 */
type EventName = string;

/** 事件优先级 */
export enum EventPriority {
    /** 最高 */
    Highest = 0,
    /** 高 */
    High = 100,
    /** 默认 */
    Normal = 200,
    /** 低 */
    Low = 300,
    /** 最低 */
    Lowest = 400,
};

/** 获取自身插件名 */
export function getPluginName(): string;

/** 获取所有已注册事件 */
export function getAllEvent(): {
    /** 事件名 */
    eventName: EventName,
    /** 模组名 */
    modName: string
}[];

/** 获取指定插件的已注册事件 */
export function getAllEvent(
    /** 模组名 */
    modName: string
): EventName[];

/** 获取事件监听数量 */
export function getListenerCount(
    /** 事件名 */
    eventName: EventName
): number;

/** 删除事件监听 */
export function removeListener(
    /** 监听ID */
    listenerId: ListenerId
): boolean;

/** 是否存在事件 */
export function hasEvent(
    /** 事件名 */
    eventName: EventName
): boolean;

/** 是否存在监听 */
export function hasListener(
    /** 监听ID */
    listenerId: ListenerId
): boolean;

/** 
 * 注册事件(需在插件加载完后注册)
 * @description 请务必在事件名前面加一些信息用于区分   
 * 例如：`"pluginName::eventName"`   
 * 如需要分类，可以使用`"pluginName::category::eventName"`   
 * 或需要更深层分类可以如`"pluginName::category1::category2::category3::eventName"`   
 * 至于为什么需要加前缀，是因为事件名是全局的，如果两个插件注册了相同的事件名，就会导致冲突
 */
export function RegisterEvent(
    /** 事件名 */
    eventName: EventName
): boolean;

/** 推送事件 */
export function publish(
    /** 事件名 */
    eventName: EventName,
    /** 事件数据 */
    eventData: NbtCompound
): void;

/** 推送事件到指定模组 */
export function publish(
    /** 事件名 */
    eventName: EventName,
    /** 事件数据 */
    eventData: NbtCompound,
    /** 模组名 */
    modName: string
): void;

/** 安装事件监听(需事件注册后安装) */
export function emplaceListener(
    /** 事件名 */
    eventName: EventName,
    /** 回调函数 */
    callback: (
        /** 事件数据 */
        event: NbtCompound
    ) => void,
    /** 事件优先级 */
    priority: EventPriority.Normal,
    /** 模组名 */
    modName: string | undefined
): ListenerId;

/** 根据玩家信息获取玩家(用于事件回调) */
export function getPlayer(
    /** 玩家信息 */
    info: NbtCompound
): Player | undefined;

/** 根据实体信息获取实体(用于事件回调) */
export function getEntity(
    /** 实体信息 */
    info: NbtCompound
): Entity | undefined;