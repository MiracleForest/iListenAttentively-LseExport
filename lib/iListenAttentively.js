const iListenAttentively = {
    getPluginName() {
        try {
            throw new Error("getPluginName");
        } catch (error) {
            return error.stack.trim().match(/\((.*)\\.*\.js:[0-9]+:[0-9]+\)$/)?.[1] || "Unknown";
        }
    },
    getAllEvent(modName) {
        return typeof modName === "string"
            ? ll.imports("iListenAttentively", "getAllEventFromModName")(modName)
            : ll.imports("iListenAttentively", "getAllEvent")();
    },
    getListenerCount(eventName) {
        return ll.imports("iListenAttentively", "getListenerCount")(eventName);
    },
    removeListener(listenerId) {
        return ll.imports("iListenAttentively", "removeListener")(listenerId);
    },
    hasEvent(eventName) {
        return ll.imports("iListenAttentively", "hasEvent")(eventName);
    },
    hasListener(listenerId) {
        return ll.imports("iListenAttentively", "hasListener")(listenerId);
    },
    RegisterEvent(eventName, pluginName = this.getPluginName()) {
        return ll.imports("iListenAttentively", "RegisterEvent")(pluginName, eventName);
    },
    publish(eventName, data, modName) {
        if (!(data instanceof NbtCompound)) throw new Error("data must be a NbtCompound");
        typeof modName === "string"
            ? ll.imports("iListenAttentively", "publishToMod")(modName, eventName, data)
            : ll.imports("iListenAttentively", "publish")(eventName, data);
    },
    emplaceListener(eventName, callback, priority = this.EventPriority.Normal, pluginName = this.getPluginName()) {
        const listenerId = ll.imports("iListenAttentively", "emplaceListener")(pluginName, eventName, priority);
        if (!this.hasListener(listenerId)) throw new Error(`Failed to emplace listener for ${eventName} in ${pluginName}`);
        ll.exports(event => {
            try {
                callback(new Proxy(event, this.nbtProxyHandler));
            } catch (error) {
                `Failed to execute event callback for ${eventName} in ${pluginName} plugin\nException: ${error.message}\nStack:\n${error.stack}`
                    .trim()
                    .split("\n")
                    .forEach(line =>
                        logger.error(line)
                    );
            }
            return event;
        }, pluginName, eventName + "_" + listenerId.toString());
        return listenerId;
    },
    EventPriority: Object.freeze({
        Highest: 0,
        High: 100,
        Normal: 200,
        Low: 300,
        Lowest: 400,
    }),
    nbtProxyHandler: {
        get(target, key) {
            if (key === "get") return () => target;
            if (key === Symbol.iterator && (target.getType() === NBT.List || target.getType() === NBT.Compound)) {
                return function* () {
                    if (target.getType() === NBT.List) {
                        for (let index = 0; index < target.getSize(); index++) yield [index, iListenAttentively.nbtProxyHandler.get(target, i)];
                    } else {
                        for (const key of target.getKeys()) yield [key, iListenAttentively.nbtProxyHandler.get(target, key)];
                    }
                };
            }
            if (target[key] !== undefined) return typeof target[key] === "function" ? target[key].bind(target) : target[key];
            if (target?.getType() === NBT.List && /^[0-9]+$/.test(key)) key = parseInt(key);
            if (target?.getType() === NBT.List && typeof key !== "number") return undefined;
            const result = target.getTag(key);
            if (result === undefined) return undefined;
            if (result?.get) return result.get();
            return new Proxy(result, this);
        },
        set(target, key, value) {
            if (!this.isNbt(value)) value = this.toNbt(value);
            if (target.getType() === NBT.List && /^[0-9]+$/.test(key)) key = parseInt(key);
            if (target.getType() === NBT.List && target.getSize() <= key) {
                for (let i = target.getSize(); i <= key; i++) target.addTag(new NbtEnd());
            }
            target.setTag(key, value);
            return this.get(target, key);
        },
        deleteProperty(target, key) {
            if (target.getType() === NBT.List && /^[0-9]+$/.test(key)) key = parseInt(key);
            target.removeTag(key);
        },
        has(target, key) {
            switch (target.getType()) {
                case NBT.Compound: return target.getKeys().includes(key);
                case NBT.List: return target.toArray().includes(key);
                default: return false;
            }
        },
        ownKeys(target) {
            switch (target.getType()) {
                case NBT.List: return Array.from({ length: target.getSize() }, (_, i) => i);
                case NBT.Compound: return target.getKeys();
                default: return [];
            }
        },
        isNbt(data) {
            return data instanceof NbtByte
                || data instanceof NbtShort
                || data instanceof NbtInt
                || data instanceof NbtLong
                || data instanceof NbtFloat
                || data instanceof NbtDouble
                || data instanceof NbtString
                || data instanceof NbtByteArray
                || data instanceof NbtList
                || data instanceof NbtCompound;
        },
        toNbt(data) {
            if (this.isNbt(data)) return data;
            if (typeof data === "string") {
                return new NbtString(data);
            } else if (typeof data === "bigint") {
                return new NbtLong(data);
            } else if (typeof data === "number" && !Number.isInteger(data)) {
                return new NbtFloat(data);
            } else if (typeof data === "number") {
                return new NbtInt(data);
            } else if (typeof data === "boolean") {
                return new NbtByte(data ? 1 : 0);
            } else if (typeof data === "object") {
                if (Array.isArray(data)) {
                    return new NbtList(data.map(this.toNbt));
                } else {
                    const compound = new NbtCompound();
                    for (const key in data) {
                        compound.setTag(key, this.toNbt(data[key]));
                    }
                    return compound;
                }
            } else {
                throw new Error(`Unsupported type ${typeof data}`);
            }
        }
    },
    getAddress(target){
        if (target instanceof LLSE_Player) return ll.imports("iListenAttentively", "getPlayerAddress")(target);
        if (target instanceof LLSE_Entity) return ll.imports("iListenAttentively", "getActorAddress")(target);
        if (target instanceof LLSE_Item) return ll.imports("iListenAttentively", "getItemStackAddress")(target);
        if (target instanceof LLSE_Block) return ll.imports("iListenAttentively", "getBlockAddress")(target);
        if (target instanceof LLSE_BlockEntity) return ll.imports("iListenAttentively", "getBlockActorAddress")(target);
        if (target instanceof LLSE_Container) return ll.imports("iListenAttentively", "getContainerAddress")(target);
        if (target instanceof NbtCompound) return ll.imports("iListenAttentively", "getCompoundTagAddress")(target);
        return undefined;
    }
};

[
    { type: "Player", checkFunc: result => result?.getNbt },
    { type: "Actor", checkFunc: result => result?.getNbt },
    { type: "ItemStack", checkFunc: result => result?.getNbt },
    { type: "Block", checkFunc: result => result?.getNbt },
    { type: "BlockActor", checkFunc: result => result?.getNbt },
    { type: "Container", checkFunc: result => result?.isEmpty },
    { type: "CompoundTag", checkFunc: result => result?.getType },
    { type: "Number", checkFunc: _ => true },
    { type: "Float", checkFunc: _ => true },
    { type: "Boolean", checkFunc: _ => true },
    { type: "String", checkFunc: _ => true },
    { type: "RawAddress", checkFunc: result => result !== 0 }
].forEach(({ type, checkFunc }) => {
    iListenAttentively[`get${type}`] = function (address) {
        if (address?.get) address = address.get();
        if (address instanceof NbtCompound) address = this.nbtProxyHandler.get(address, "_pointer_");
        const result = ll.imports("iListenAttentively", `get${type}`)(address);
        return checkFunc(result) ? result : undefined;
    };
});

NbtList.prototype.toSNBT = function (index = -1) {
    let result = new NbtCompound({
        data: this
    }).toSNBT(index);
    return result.slice(
        result.indexOf(":") + 1,
        -1
    ).replace(
        new RegExp(
            `\n${" ".repeat(index)}`,
            "g"
        ),
        "\n"
    ).trim();
};
NbtByte.prototype.toSNBT = function () {
    return `${this.get()}b`;
};
NbtDouble.prototype.toSNBT = function () {
    return `${this.get()}D`;
};
NbtFloat.prototype.toSNBT = function () {
    return `${this.get()}f`;
};
NbtLong.prototype.toSNBT = function () {
    return `${this.get()}l`;
};
NbtInt.prototype.toSNBT = function () {
    return `${this.get()}`;
};
NbtShort.prototype.toSNBT = function () {
    return `${this.get()}s`;
};
NbtString.prototype.toSNBT = function () {
    return JSON.stringify(this.get());
};

module.exports = iListenAttentively;