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
 * 注册事件(需在自身插件加载完后注册)
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

interface EventListenerData extends NbtCompound extends NbtList {
    [key: string | number]: EventListenerData | undefined;
    [Symbol.iterator]: () => Iterator<[string | number, EventListenerData]>;
}

/** 安装事件监听(需事件注册后安装) */
export function emplaceListener(
    /** 事件名 */
    eventName: EventName,
    /** 回调函数 */
    callback: (
        /** 事件数据 */
        event: EventListenerData
    ) => void,
    /** 事件优先级 */
    priority: EventPriority.Normal = EventPriority.Normal,
    /** 模组名 */
    modName: string | undefined = getPluginName()
): ListenerId;

/** 根据内存地址获取玩家 */
export function getPlayer(
    /** 地址 */
    info: number
): Player | undefined;

/** 根据内存地址获取实体 */
export function getEntity(
    /** 地址 */
    info: number
): Entity | undefined;

/** 根据内存地址获取物品 */
export function getItem(
    /** 地址 */
    info: number
): Item | undefined;

/** 根据内存地址获取方块 */
export function getBlock(
    /** 地址 */
    info: number
): Block | undefined;

/** 根据内存地址获取方块实体 */
export function getBlockEntity(
    /** 地址 */
    info: number
): BlockEntity | undefined;

/** 根据内存地址获取容器 */
export function getContainer(
    /** 地址 */
    info: number
): Container | undefined;

/** 根据根据序列化信息获取玩家 */
export function getPlayer(
    /** 序列化信息 */
    info: NbtCompound
): Player | undefined;

/** 根据序列化信息获取实体 */
export function getEntity(
    /** 序列化信息 */
    info: NbtCompound
): Entity | undefined;

/** 根据序列化信息获取物品 */
export function getItem(
    /** 序列化信息 */
    info: NbtCompound
): Item | undefined;

/** 根据序列化信息获取方块 */
export function getBlock(
    /** 序列化信息 */
    info: NbtCompound
): Block | undefined;

/** 根据序列化信息获取方块实体 */
export function getBlockEntity(
    /** 序列化信息 */
    info: NbtCompound
): BlockEntity | undefined;

/** 根据序列化信息获取容器 */
export function getContainer(
    /** 序列化信息 */
    info: NbtCompound
): Container | undefined;

declare let nbtProxyHandler: ProxyHandler<any>;

interface NbtList {
    toSNBT(index = -1): string;
}
interface NbtByte {
    toSNBT(): string;
}
interface NbtDouble {
    toSNBT(): string;
}
interface NbtFloat {
    toSNBT(): string;
}
interface NbtLong {
    toSNBT(): string;
}
interface NbtInt {
    toSNBT(): string;
}
interface NbtShort {
    toSNBT(): string;
}
interface NbtString {
    toSNBT(): string;
}