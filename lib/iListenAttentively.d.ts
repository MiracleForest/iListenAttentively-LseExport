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

/** 获取所有事件别名 */
export function getAllEventAlias(): Record<EventName, EventName>;

/** 获取事件别名 */
export function getEventAlias(
    /** 事件名 */
    eventName: EventName
): EventName[];

/** 通过事件别名获取事件名 */
export function getEventName(
    /** 事件别名 */
    eventAlias: EventName
): EventName;

/** 通过维度名字获取ID */
export function getDimensionNameFromId(
    /** 维度ID */
    dimensionId: number
): string | undefined;

/** 通过维度ID获取名字 */
export function getDimensionIdFromName(
    /** 维度名字 */
    dimensionName: string
): number | undefined;

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
): Boolean;

/** 是否存在事件 */
export function hasEvent(
    /** 事件名 */
    eventName: EventName
): Boolean;

/** 是否存在监听 */
export function hasListener(
    /** 监听ID */
    listenerId: ListenerId
): Boolean;

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
): Boolean;

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

    toSNBT(index = -1): string;
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

/** 根据内存地址/序列化信息获取玩家 */
export function getPlayer(
    /** 地址/序列化信息 */
    info: number | NbtCompound | EventListenerData
): Player | undefined;

/** 根据内存地址/序列化信息获取实体 */
export function getActor(
    /** 地址/序列化信息 */
    info: number | NbtCompound | EventListenerData
): Entity | undefined;

/** 根据内存地址/序列化信息获取物品 */
export function getItemStack(
    /** 地址/序列化信息 */
    info: number | NbtCompound | EventListenerData
): Item | undefined;

/** 根据内存地址/序列化信息获取方块 */
export function getBlock(
    /** 地址/序列化信息 */
    info: number | NbtCompound | EventListenerData
): Block | undefined;

/** 根据内存地址/序列化信息获取方块实体 */
export function getBlockActor(
    /** 地址/序列化信息 */
    info: number | NbtCompound | EventListenerData
): BlockEntity | undefined;

/** 根据内存地址/序列化信息获取容器 */
export function getContainer(
    /** 地址/序列化信息 */
    info: number | NbtCompound | EventListenerData
): Container | undefined;

/** 根据内存地址/序列化信息获取NBT */
export function getCompoundTag(
    /** 地址/序列化信息 */
    info: number | NbtCompound | EventListenerData
): NbtCompound | undefined;

/** 根据内存地址/序列化信息获取数字 */
export function getLongLong(
    /** 地址/序列化信息 */
    info: number | NbtCompound | EventListenerData
): number;

/** 根据内存地址/序列化信息获取数字 */
export function getUnsignedLongLong(
    /** 地址/序列化信息 */
    info: number | NbtCompound | EventListenerData
): number;

/** 根据内存地址/序列化信息获取数字 */
export function getInt(
    /** 地址/序列化信息 */
    info: number | NbtCompound | EventListenerData
): number;

/** 根据内存地址/序列化信息获取数字 */
export function getUnsignedInt(
    /** 地址/序列化信息 */
    info: number | NbtCompound | EventListenerData
): number;

/** 根据内存地址/序列化信息获取数字 */
export function getShort(
    /** 地址/序列化信息 */
    info: number | NbtCompound | EventListenerData
): number;

/** 根据内存地址/序列化信息获取数字 */
export function getUnsignedShort(
    /** 地址/序列化信息 */
    info: number | NbtCompound | EventListenerData
): number;

/** 根据内存地址/序列化信息获取数字 */
export function getChar(
    /** 地址/序列化信息 */
    info: number | NbtCompound | EventListenerData
): number;

/** 根据内存地址/序列化信息获取数字 */
export function getUnsignedChar(
    /** 地址/序列化信息 */
    info: number | NbtCompound | EventListenerData
): number;

/** 根据内存地址/序列化信息获取浮点数 */
export function getFloat(
    /** 地址/序列化信息 */
    info: number | NbtCompound | EventListenerData
): number;

/** 根据内存地址/序列化信息获取浮点数 */
export function getDouble(
    /** 地址/序列化信息 */
    info: number | NbtCompound | EventListenerData
): number;

/** 根据内存地址/序列化信息获取浮点数 */
export function getLongDouble(
    /** 地址/序列化信息 */
    info: number | NbtCompound | EventListenerData
): number;

/** 根据内存地址/序列化信息获取布尔值 */
export function getBoolean(
    /** 地址/序列化信息 */
    info: number | NbtCompound | EventListenerData
): Boolean;

/** 根据内存地址/序列化信息获取字符串 */
export function getString(
    /** 地址/序列化信息 */
    info: number | NbtCompound | EventListenerData
): string;

/** 根据内存地址/序列化信息获取指针原始地址 */
export function getRawAddress(
    /** 地址/序列化信息 */
    info: number | NbtCompound | EventListenerData
): number | undefined;

/** 获取实体/玩家/物品/方块/方块实体/容器/NBT地址 */
export function getAddress(
    target: Entity | Player | Item | Block | BlockEntity | Container | CompoundTag
): number | undefined;

declare let nbtProxyHandler: ProxyHandler<NbtCompound>;

interface NbtList {
    toSNBT(index = -1): string;
    forEach(callback: (value: any, index: number, array: NbtList) => void): void;
    filter(callback: (value: any, index: number, array: NbtList) => Boolean): NbtList;
    map(callback: (value: any, index: number, array: NbtList) => any): NbtList;
    find(callback: (value: any, index: number, array: NbtList) => Boolean): any;
    some(callback: (value: any, index: number, array: NbtList) => Boolean): Boolean;
    every(callback: (value: any, index: number, array: NbtList) => Boolean): Boolean;
    reduce<T = any>(callback: (accumulator: T, value: any, index: number, array: NbtList) => any, initialValue?: T): T;
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