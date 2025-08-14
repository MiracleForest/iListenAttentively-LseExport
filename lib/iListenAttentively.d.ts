declare module "iListenAttentively" {
    type NbtTypes =
        | NbtByte
        | NbtShort
        | NbtInt
        | NbtLong
        | NbtFloat
        | NbtDouble
        | NbtString
        | NbtByteArray
        | NbtList
        | NbtCompound;

    type ProxiedNbt = {
        /** 读取/设置某个下标位置的NBT对象 */
        [key: string | number]: ProxiedNbt
        /** 读取某个下标位置的NBT对象 */
        get(key: string | number): ProxiedNbt | undefined;
        /** 设置某个下标位置的NBT对象 */
        set(key: string | number, value: any): boolean;
        /** 删除某个下标位置的NBT对象 */
        remove(key: string | number): boolean;
        /** 添加一个NBT对象(仅对NbtList有效) */
        add(value: any): number;
        /** 将NBT对象转换为SNBT格式字符串 */
        toSnbt(snbtFormat?: SnbtFormat, indent?: number): string;
        /** 判断NBT对象类型 */
        hold(type: NbtType): boolean;
        /** 判断NBT对象是否为列表 */
        isArray(): boolean;
        /** 判断NBT对象是否为复合 */
        isObject(): boolean;
        /** 判断NBT对象是否为字符串 */
        isString(): boolean;
        /** 判断NBT对象是否为字节 */
        isBoolean(): boolean;
        /** 判断NBT对象是否为浮点数 */
        isNumberFloat(): boolean;
        /** 判断NBT对象是否为整数 */
        isNumberInteger(): boolean;
        /** 判断NBT对象是否为数字 */
        isNumber(): boolean;
        /** 判断NBT对象是否基础类型 */
        isPrimitive(): boolean;
        /** 判断是否包含某个下标的NBT对象(仅对NbtCompound有效) */
        contains(key: string | number, type?: NbtType): boolean;
        /** 获取NBT对象的大小 */
        size(): number;
        [Symbol.iterator](): IterableIterator<[string | number, ProxiedNbt | number | string]>;

        /** 转成玩家对象 */
        toPlayer(): Player | undefined;
        /** 转成实体对象 */
        toActor(): Entity | undefined;
        /** 转成物品对象 */
        toItemStack(): Item | undefined;
        /** 转成方块对象 */
        toBlock(): Block | undefined;
        /** 转成方块实体对象 */
        toBlockEntity(): BlockEntity | undefined;
        /** 转成容器对象 */
        toContainer(): Container | undefined;
        /** 转成NBT对象对象 */
        toCompoundTag(): NbtCompound | undefined;
    } & NbtTypes;

    /** 全局导出命名空间 */
    const NamespaceName: string;

    /** 事件优先级 */
    enum EventPriority {
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

    /** 原生类型 */
    enum NativeType {
        Void = 0,
        Bool = 1,
        Char = 2,
        UnsignedChar = 3,
        Short = 4,
        UnsignedShort = 5,
        Int = 6,
        UnsignedInt = 7,
        Long = 8,
        UnsignedLong = 9,
        LongLong = 10,
        UnsignedLongLong = 11,
        Float = 12,
        Double = 13,
        LongDouble = 14,
        Pointer = 15,
    };

    /** SNBT格式 */
    enum SnbtFormat {
        Minimize = 0,
        CompoundLineFeed = 1 << 0,
        ArrayLineFeed = 1 << 1,
        Colored = 1 << 2,
        Console = 1 << 3,
        ForceAscii = 1 << 4,
        ForceQuote = 1 << 5,
        CommentMarks = 1 << 6,
        Jsonify = ForceQuote | CommentMarks,
        PartialLineFeed = CompoundLineFeed,
        AlwaysLineFeed = CompoundLineFeed | ArrayLineFeed,
        PrettyFilePrint = PartialLineFeed,
        PrettyChatPrint = PrettyFilePrint | Colored,
        PrettyConsolePrint = PrettyFilePrint | Colored | Console
    };

    /** NBT类型 */
    enum NbtType {
        End = 0,
        Byte = 1,
        Short = 2,
        Int = 3,
        Int64 = 4,
        Float = 5,
        Double = 6,
        ByteArray = 7,
        String = 8,
        List = 9,
        Compound = 10,
        IntArray = 11,
    };

    const NbtProxy: {
        FunctionMap: {
            get(target: any): (key: any) => any;
            set(target: any): (key: any, value: any) => boolean;
            remove(target: any): (key: any) => boolean;
            add(target: any): (value: any) => number;
            toSnbt(target: any): (snbtFormat?: SnbtFormat, indent?: number) => string;
            hold(target: any): (type: number) => boolean;
            isArray(target: any): () => boolean;
            isObject(target: any): () => boolean;
            isString(target: any): () => boolean;
            isBoolean(target: any): () => boolean;
            isNumberFloat(target: any): () => boolean;
            isNumberInteger(target: any): () => boolean;
            isNumber(target: any): () => boolean;
            isPrimitive(target: any): () => boolean;
            contains(target: any): (key: any, type?: number) => boolean;
            size(target: any): () => number;
            [Symbol.iterator](target: any): () => Generator<[string | number, ProxiedNbt | number | string], void, unknown>;
            toPlayer(target: any): () => Player | undefined;
            toEntity(target: any): () => Entity | undefined;
            toBlock(target: any): () => Block | undefined;
            toBlockActor(target: any): () => BlockEntity | undefined;
            toItemStack(target: any): () => Item | undefined;
            toContainer(target: any): () => Container | undefined;
            toCompoundTag(target: any): () => NbtCompound | undefined;
        };
        get(target: any, key: any): any;
        set(target: any, key: any, value: any): boolean;
        deleteProperty(target: any, key: any): boolean;
        has(target: any, key: any): boolean;
        ownKeys(target: any): string[] | number[];
    };

    /** 判断一个对象是否为NBT */
    function isNbt(data: any): data is NbtTypes;
    /** 将一个对象转换为NBT */
    function toNbt(data: any): NbtTypes;
    /** 将一个NBT转换为对象 */
    function fromNbt(data: NbtTypes): any;
    /** 将一个NBT转换为SNBT字符串 */
    function toSnbt(data: NbtTypes, snbtFormat?: SnbtFormat, indent?: number): string;
    /** 将一个SNBT字符串转换为NBT */
    function fromSnbt(snbt: string): NbtTypes;
    /** 调用插件导出的函数 */
    function callExportedFunction(functionName: string, ...args: any[]): any;
    /** 获取自身插件名称 */
    function getPluginName(): string;
    /** 
     * 注册事件(需在自身插件加载完后注册,建议使用setTimeout)
     * @description 请务必在事件名前面加一些信息用于区分   
     * 例如：`"pluginName::eventName"`   
     * 如需要分类，可以使用`"pluginName::category::eventName"`   
     * 或需要更深层分类可以如`"pluginName::category1::category2::category3::eventName"`
     * 至于为什么需要加前缀，是因为事件名是全局的，如果两个插件注册了相同的事件名，就会导致冲突
     */
    function registerEvent(eventName: string, pluginName?: string): boolean;
    /** @deprecated */
    function RegisterEvent(eventName: string, pluginName?: string): any;
    /** 监听事件 */
    function emplaceListener(
        eventName: string,
        callback: (event: ProxiedNbt) => void,
        priority?: EventPriority,
        pluginName?: string
    ): number;
    /** 推送事件 */
    function publish(eventName: string, eventData: Record<string, any> | NbtCompound, modName?: string): void;

    /** 获取所有事件别名 */
    function getAllEventAlias(): Record<string, string>;
    /** 获取事件别名 */
    function getEventAlias(eventName: string): string[];
    /** 通过事件别名获取事件名 */
    function getEventName(eventAlias: string): string;
    /** 通过维度名字获得维度ID */
    function getDimensionIdFromName(dimensionName: string): number;
    /** 通过维度ID获得维度名字 */
    function getDimensionNameFromId(dimensionId: number): string;
    /** 取消监听事件 */
    function removeListener(listenerId: number): boolean;
    /** 是否存在监听 */
    function hasListener(listenerId: number): boolean;
    /** 获取所有事件 */
    function getAllEvent(): { eventName: string, modName: string }[];
    /** 通过插件名获取事件 */
    function getAllEvent(modName: string): string[];
    /** 判断是否存在事件 */
    function hasEvent(eventName: string): boolean;
    /** 获取事件监听数量 */
    function getListenerCount(eventName: string): number;

    /** 获取玩家 */
    function getPlayer(data: NbtCompound | number): Player | undefined;
    /** 获取实体 */
    function getActor(data: NbtCompound | number): Entity | undefined;
    /** 获取物品 */
    function getItemStack(data: NbtCompound | number): Item | undefined;
    /** 获取方块 */
    function getBlock(data: NbtCompound | number): Block | undefined;
    /** 获取方块实体 */
    function getBlockActor(data: NbtCompound | number): BlockEntity | undefined;
    /** 获取容器 */
    function getContainer(data: NbtCompound | number): Container | undefined;
    /** 获取NBT */
    function getCompoundTag(data: NbtCompound | number): NbtCompound | undefined;

    /** 获取玩家地址 */
    function getPlayerAddress(data: Player): number;
    /** 获取实体地址 */
    function getActorAddress(data: Entity): number;
    /** 获取物品地址 */
    function getItemStackAddress(data: Item): number;
    /** 获取方块地址 */
    function getBlockAddress(data: Block): number;
    /** 获取方块实体地址 */
    function getBlockActorAddress(data: BlockEntity): number;
    /** 获取容器地址 */
    function getContainerAddress(data: Container): number;
    /** 获取NBT地址 */
    function getCompoundTagAddress(data: NbtCompound): number;

    function getRawAddress(address: number): any;
    function getLongLong(address: number): any;
    function getUnsignedLongLong(address: number): any;
    function getInt(address: number): any;
    function getUnsignedInt(address: number): any;
    function getShort(address: number): any;
    function getUnsignedShort(address: number): any;
    function getChar(address: number): any;
    function getUnsignedChar(address: number): any;
    function getFloat(address: number): any;
    function getLongDouble(address: number): any;
    function getDouble(address: number): any;
    function getBool(address: number): any;
    function getString(address: number): any;

    function setRawAddress(address: number, data: number): any;
    function setLongLong(address: number, data: number): any;
    function setUnsignedLongLong(address: number, data: number): any;
    function setInt(address: number, data: number): any;
    function setUnsignedInt(address: number, data: number): any;
    function setShort(address: number, data: number): any;
    function setUnsignedShort(address: number, data: number): any;
    function setChar(address: number, data: number): any;
    function setUnsignedChar(address: number, data: number): any;
    function setFloat(address: number, data: number): any;
    function setLongDouble(address: number, data: number): any;
    function setDouble(address: number, data: number): any;
    function setBool(address: number, data: number): any;
    function setString(address: number, data: number): any;

    function getAddressFromSymbol(symbol: string): number;
    function getAddressFromSymbol(dllNmae: string, symbol: string): number;
    function getAddressFromSignature(signature: string): number;
    function getAddressFromSignature(dllName: string, signature: string): number;
    function getImageRange(dllName: string): { start: number, end: number, size: number };
    function dynamicCall(address: number, returnType: NativeType, paramsType: NativeType[], params: number[]): any;
    function mallocMemory(size: number): number;
    function freeMemory(address: number): void;
    function alignedMallocMemory(size: number, alignment: number): number;
    function alignedFreeMemory(address: number): void;
    function getUsableMemorySize(address: number): number;
    function memcpyMemory(dest: number, src: number, size: number): void;
    function memsetMemory(dest: number, value: number, size: number): void;
    function memcmpMemory(dest: number, src: number, size: number): number;
    function memmoveMemory(dest: number, src: number, size: number): number;

    export = {
        NamespaceName,
        EventPriority,
        NativeType,
        SnbtFormat,
        NbtProxy,
        NbtType,
        isNbt,
        toNbt,
        fromNbt,
        toSnbt,
        fromSnbt,
        callExportedFunction,
        getPluginName,
        registerEvent,
        RegisterEvent,
        emplaceListener,
        publish,
        getAllEventAlias,
        getEventAlias,
        getEventName,
        getDimensionIdFromName,
        getDimensionNameFromId,
        removeListener,
        hasListener,
        getAllEvent,
        hasEvent,
        getListenerCount,
        getPlayer,
        getActor,
        getItemStack,
        getBlock,
        getBlockActor,
        getContainer,
        getCompoundTag,
        getPlayerAddress,
        getActorAddress,
        getItemStackAddress,
        getBlockAddress,
        getBlockActorAddress,
        getContainerAddress,
        getCompoundTagAddress,
        getRawAddress,
        getLongLong,
        getUnsignedLongLong,
        getInt,
        getUnsignedInt,
        getShort,
        getUnsignedShort,
        getChar,
        getUnsignedChar,
        getFloat,
        getLongDouble,
        getDouble,
        getBool,
        getString,
        setRawAddress,
        setLongLong,
        setUnsignedLongLong,
        setInt,
        setUnsignedInt,
        setShort,
        setUnsignedShort,
        setChar,
        setUnsignedChar,
        setFloat,
        setLongDouble,
        setDouble,
        setBool,
        setString,
        getAddressFromSymbol,
        getAddressFromSignature,
        getImageRange,
        dynamicCall,
        mallocMemory,
        freeMemory,
        alignedMallocMemory,
        alignedFreeMemory,
        getUsableMemorySize,
        memcpyMemory,
        memsetMemory,
        memcmpMemory,
        memmoveMemory
    };
}