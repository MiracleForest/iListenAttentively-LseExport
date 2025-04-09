const iListenAttentively = {
    getPluginName() {
        // quickjs
        try {
            throw new Error("getPluginName");
        } catch (error) {
            const /** @type {string} */ line = error.stack.trim().split("\n").pop().trim();
            if (line.includes("<anonymous>")) {
                return line.slice(
                    line.indexOf("(") + 1,
                    line.indexOf("\\")
                );
            }
            if (line.includes("<eval>")) {
                return line.slice(
                    line.indexOf("/", line.indexOf("/") + 1) + 1,
                    line.indexOf("\\")
                );
            }
        }

        // nodejs
        try {
            const path = require('path');
            const selfFileName = path.basename(__filename);
            const pluginDirectory = Object.entries(
                require('module')._pathCache
            ).find(
                ([key, _]) =>
                    key.includes(selfFileName)
            )[0].split("\u0000")[1];
            const directories = pluginDirectory.split("\\");
            const pluginName = directories[directories.findIndex(value => value === "plugins") + 1].trim();
            if (pluginName) return pluginName;
        } catch { }
        try {
            throw new Error("getPluginName");
        } catch (error) {
            const /** @type {string} */ line = error.stack.trim().split("\n").pop().trim();
            if (line.includes(".js") && /:\d+:\d+$/.test(line)) {
                const directories = line.split("\\");
                const pluginName = directories[directories.findIndex(value => value === "plugins") + 1].trim();
                if (pluginName) return pluginName;
            }
        }
        return "Unknown";
    },
    getAllEvent(modName) {
        return typeof modName === "string"
            ? ll.imports("iListenAttentively", "getAllEventFromModName")(modName)
            : ll.imports("iListenAttentively", "getAllEvent")();
    },
    getAllEventAlias() {
        return ll.imports("iListenAttentively", "getAllEventAlias")();
    },
    getEventAlias(eventName) {
        return ll.imports("iListenAttentively", "getEventAlias")(eventName);
    },
    getEventName(eventAlias) {
        return ll.imports("iListenAttentively", "getEventName")(eventAlias) || undefined;
    },
    getDimensionIdFromName(name) {
        const result = ll.imports("iListenAttentively", "getDimensionIdFromName")(name);
        return result === -1 ? undefined : result;
    },
    getDimensionNameFromId(name) {
        const result = ll.imports("iListenAttentively", "getDimensionNameFromId")(name);
        return result ? undefined : result;
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
        if (listenerId > 0xFFFFFFFFFFFFFFFFn || listenerId < 0) return false;
        return ll.imports("iListenAttentively", "hasListener")(listenerId);
    },
    RegisterEvent(eventName, pluginName = this.getPluginName()) {
        return ll.imports("iListenAttentively", "RegisterEvent")(pluginName, eventName);
    },
    publish(eventName, data, modName) {
        if (data instanceof NbtCompound) {
            return typeof modName === "string"
            ? ll.imports("iListenAttentively", "publishToMod")(modName, eventName, data)
            : ll.imports("iListenAttentively", "publish")(eventName, data);
        }
        if (data instanceof Object) {
            const eventData = this.nbtProxyHandler.toNbt(data);
            typeof modName === "string"
                ? ll.imports("iListenAttentively", "publishToMod")(modName, eventName, eventData)
                : ll.imports("iListenAttentively", "publish")(eventName, eventData);
            Object.keys(data).forEach(key => delete data[key]);
            Object.entries(this.nbtProxyHandler.toObject(eventData)).forEach(([key, value]) => (data[key] = value));
            return;
        }
        throw new Error("Unsupported data type");
    },
    emplaceListener(eventName, callback, priority = this.EventPriority.Normal, pluginName = this.getPluginName()) {
        if (!this.hasEvent(eventName) && this.getEventName(eventName)) eventName = this.getEventName(eventName);
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
        }, pluginName, eventName + "#" + listenerId.toString());
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
                        for (let index = 0; index < target.getSize(); index++) yield [index, iListenAttentively.nbtProxyHandler.get(target, index)];
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
        },
        toObject(data) {
            if (data instanceof NbtCompound) {
                const obj = {};
                for (const key of data.getKeys()) {
                    obj[key] = this.toObject(data.getData(key));
                }
                return obj;
            } else if (data instanceof NbtList) {
                const arr = [];
                for (let index = 0; index < data.getSize(); index++) {
                    arr.push(this.toObject(data.getTag(index)));
                }
                return arr;
            } else {
                return data?.get?.() ??  data;
            }
        }
    },
    getAddress(target) {
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

    { type: "LongLong" },
    { type: "UnsignedLongLong" },
    { type: "Int" },
    { type: "UnsignedInt" },
    { type: "Short" },
    { type: "UnsignedShort" },
    { type: "Char" },
    { type: "UnsignedChar" },

    { type: "Float" },
    { type: "Double" },
    { type: "LongDouble" },

    { type: "Boolean" },
    { type: "String" },
    { type: "RawAddress", checkFunc: result => result !== 0 }
].forEach(({ type, checkFunc }) => {
    iListenAttentively[`get${type}`] = function (address) {
        if (address?.get) address = address.get();
        if (address instanceof NbtCompound) address = this.nbtProxyHandler.get(address, "_pointer_");
        const result = ll.imports("iListenAttentively", `get${type}`)(address);
        return !checkFunc || checkFunc(result) ? result : undefined;
    };
});

const callCallback = (callback, self, ...args) =>
    callback.call ? callback.call(self, ...args) : callback(...args);

NbtList.prototype.toSNBT = function (index = 4) {
    let result = new NbtCompound({
        data: this
    }).toSNBT(index);
    return result.slice(
        result.indexOf(":") + 1,
        -1
    ).replace(
        new RegExp(
            `\n${" ".repeat(index < 0 ? 0 : index)}`,
            "g"
        ),
        "\n"
    ).trim();
};
NbtList.prototype.forEach = function (callback, thisArg) {
    if (this == null) throw new TypeError("this is null or undefined");
    if (typeof callback !== "function") throw new TypeError(callback + " is not a function");

    for (let length = this.getSize(), index = 0; index < length; index++) {
        if (index < this.getSize()) {
            callCallback(callback, thisArg ?? this, this.getTag(index), index, this);
        }
    }
};
NbtList.prototype.filter = function (callback, thisArg) {
    if (this == null) throw new TypeError("this is null or undefined");
    if (typeof callback !== "function") throw new TypeError(callback + " is not a function");

    const result = new NbtList();

    for (let length = this.getSize(), index = 0; index < length; index++) {
        if (index < this.getSize()) {
            if (callCallback(callback, thisArg ?? this, this.getTag(index), index, this)) {
                result.addTag(this.getTag(index));
            }
        }
    }

    return result;
};
NbtList.prototype.map = function (callback, thisArg) {
    if (this == null) throw new TypeError("this is null or undefined");
    if (typeof callback !== "function") throw new TypeError(callback + " is not a function");

    const result = new NbtList();

    for (let length = this.getSize(), index = 0; index < length; index++) {
        if (index < this.getSize()) {
            result.addTag(callCallback(callback, thisArg ?? this, this.getTag(index), index, this));
        }
    }

    return result;
};
NbtList.prototype.find = function (callback, thisArg) {
    if (this == null) throw new TypeError("this is null or undefined");
    if (typeof callback !== "function") throw new TypeError(callback + " is not a function");

    for (let length = this.getSize(), index = 0; index < length; index++) {
        if (index < this.getSize()) {
            if (callCallback(callback, thisArg ?? this, this.getTag(index), index, this)) return this.getTag(index);
        }
    }

    return undefined;
};
NbtList.prototype.some = function (callback, thisArg) {
    if (this == null) throw new TypeError("this is null or undefined");
    if (typeof callback !== "function") throw new TypeError(callback + " is not a function");

    for (let length = this.getSize(), index = 0; index < length; index++) {
        if (index < this.getSize()) {
            if (callCallback(callback, thisArg ?? this, this.getTag(index), index, this)) return true;
        }
    }

    return false;
};
NbtList.prototype.every = function (callback, thisArg) {
    if (this == null) throw new TypeError("this is null or undefined");
    if (typeof callback !== "function") throw new TypeError(callback + " is not a function");

    for (let length = this.getSize(), index = 0; index < length; index++) {
        if (index < this.getSize()) {
            if (!callCallback(callback, thisArg ?? this, this.getTag(index), index, this)) return false;
        }
    }

    return true;
};
NbtList.prototype.reduce = function (callback, initialValue) {
    if (this == null) throw new TypeError("this is null or undefined");
    if (typeof callback !== "function") throw new TypeError(callback + " is not a function");

    const length = this.getSize();
    let accumulator = initialValue;
    let index = 0;

    if (initialValue === undefined) {
        if (this.getSize() === 0) throw new TypeError("Reduce of empty array with no initial value");
        accumulator = this.getTag(index++);
    }

    while (index < length) {
        accumulator = callCallback(callback, thisArg ?? this, accumulator, this.getTag(index), index, this);
        index++;
    }

    return undefined;
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