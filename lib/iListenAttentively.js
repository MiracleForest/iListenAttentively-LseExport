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
                callback(new Proxy(event, {
                    get(target, key) {
                        if (key === "get") return () => target;
                        if (target[key] !== undefined) return typeof target[key] === "function" ? target[key].bind(target) : target[key];
                        if (target?.getType() === NBT.List && /^[0-9]+$/.test(key)) key = parseInt(key);
                        const result = target.getTag(key);
                        if (result === undefined) return undefined;
                        if (result?.get) return result.get();
                        return new Proxy(result, this);
                    },
                    set(target, key, value) {
                        if (!this.isNbt(value)) value = this.toNbt(value);
                        if (target.getType() === NBT.List && /^[0-9]+$/.test(key)) key = parseInt(key);
                        if (target.getType() === NBT.List && target.getSize() <= key) {
                            for (let i = target.getSize(); i <= key; i++) target.addTag(new NbtByte(0));
                        }
                        target.setTag(key, value);
                        return this.get(target, key);
                    },
                    deleteProperty(target, key) {
                        target.removeTag(key);
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
                }));
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
    getPlayer(info) {
        const result = ll.imports("iListenAttentively", "getPlayer")(info);
        return result?.uuid ? result : undefined;
    },
    getEntity(info) {
        const result = ll.imports("iListenAttentively", "getEntity")(info);
        return result?.uniqueId ? result : undefined;
    }
};

module.exports = iListenAttentively;