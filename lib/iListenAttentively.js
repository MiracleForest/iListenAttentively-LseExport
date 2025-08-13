module.exports = Object.assign(
    {
        NamespaceName: "iListenAttentively",
        EventPriority: Object.freeze({
            Highest: 0,
            High: 100,
            Normal: 200,
            Low: 300,
            Lowest: 400,
        }),
        NativeType: Object.freeze({
            Void: 0,
            Bool: 1,
            Char: 2,
            UnsignedChar: 3,
            Short: 4,
            UnsignedShort: 5,
            Int: 6,
            UnsignedInt: 7,
            Long: 8,
            UnsignedLong: 9,
            LongLong: 10,
            UnsignedLongLong: 11,
            Float: 12,
            Double: 13,
            LongDouble: 14,
            Pointer: 15
        }),
        SnbtFormat: Object.freeze({
            Minimize: 0,
            CompoundLineFeed: 1 << 0,
            ArrayLineFeed: 1 << 1,
            Colored: 1 << 2,
            Console: 1 << 3,
            ForceAscii: 1 << 4,
            ForceQuote: 1 << 5,
            CommentMarks: 1 << 6,
            Jsonify: 1 << 5 | 1 << 6,
            PartialLineFeed: 1 << 0,
            AlwaysLineFeed: 1 << 0 | 1 << 1,
            PrettyFilePrint: 1 << 0,
            PrettyChatPrint: 1 << 0 | 1 << 2,
            PrettyConsolePrint: 1 << 0 | 1 << 2 | 1 << 3
        }),
        NbtType: Object.freeze({
            End: 0,
            Byte: 1,
            Short: 2,
            Int: 3,
            Int64: 4,
            Float: 5,
            Double: 6,
            ByteArray: 7,
            String: 8,
            List: 9,
            Compound: 10,
            IntArray: 11,
        }),
        /** @type {ProxyHandler} */
        NbtProxy: {
            FunctionMap: {
                get(target) {
                    switch (target.getType()) {
                        case NBT.Compound: {
                            return (key) => {
                                let nbt = target.getTag(key);
                                if (nbt === undefined) return undefined;
                                if (nbt?.get) nbt = nbt.get();
                                return typeof nbt === "object" ? new Proxy(nbt, module.exports.NbtProxy) : nbt;
                            };
                        }
                        case NBT.List: {
                            return (index) => {
                                if (typeof index === "string" && /^[0-9]+$/.test(key)) index = parseInt(index);
                                if (typeof index !== "number" || index < 0 || index >= target.getSize()) return undefined;
                                let nbt = target.getTag(index);
                                if (nbt?.get) nbt = nbt.get();
                                return typeof nbt === "object" ? new Proxy(nbt, module.exports.NbtProxy) : nbt;
                            };
                        }
                        default: {
                            return target?.get?.();
                        }
                    }
                },
                set(target) {
                    switch (target.getType()) {
                        case NBT.Compound: {
                            return (key, value) => {
                                if (typeof key !== "string") return false;
                                target.setTag(key, module.exports.toNbt(value));
                                return true;
                            };
                        }
                        case NBT.List: {
                            return (index, value) => {
                                if (typeof index === "string" && /^[0-9]+$/.test(index)) index = parseInt(index);
                                if (typeof index !== "number" || index < 0 || index >= target.getSize()) return false;
                                target.setTag(index, module.exports.toNbt(value));
                                return true;
                            };
                        }
                        default: {
                            return target?.set;
                        }
                    }
                },
                remove(target) {
                    switch (target.getType()) {
                        case NBT.Compound: {
                            return (key) => {
                                if (typeof key !== "string") return false;
                                target.removeTag(key);
                                return true;
                            };
                        }
                        case NBT.List: {
                            return (index) => {
                                if (typeof index === "string" && /^[0-9]+$/.test(index)) index = parseInt(index);
                                if (typeof index !== "number" || index < 0 || index >= target.getSize()) return false;
                                target.removeTag(index);
                                return true;
                            };
                        }
                        default: {
                            return () => false;
                        }
                    }
                },
                add(target) {
                    return (value) => {
                        if (!this.isArray(target)()) throw new Error(`Unsupported type ${target.getType()}`);
                        target.addTag(module.exports.toNbt(value));
                        return target.getSize() - 1;
                    };
                },
                toSnbt(target) {
                    return (...args) => module.exports.toSnbt(target, ...args);
                },
                hold(target) {
                    return (type) => target.getType() === type;
                },
                isArray(target) {
                    return () => this.hold(target)(NBT.List);
                },
                isObject(target) {
                    return () => this.hold(target)(NBT.Compound);
                },
                isString(target) {
                    return () => this.hold(target)(NBT.String);
                },
                isBoolean(target) {
                    return () => this.hold(target)(NBT.Byte);
                },
                isNumberFloat(target) {
                    return () => this.hold(target)(NBT.Float) || this.hold(target)(NBT.Double);
                },
                isNumberInteger(target) {
                    return () => this.hold(target)(NBT.Byte) || this.hold(target)(NBT.Short) || this.hold(target)(NBT.Int) || this.hold(target)(NBT.Int64);
                },
                isNumber(target) {
                    return () => this.isNumberFloat(target)() || this.isNumberInteger(target)();
                },
                isPrimitive(target) {
                    return () => this.isNumber(target)() || this.isString(target)();
                },
                contains(target) {
                    return (key, type) => {
                        if (!this.isObject(target)() || typeof key !== "string") return false;
                        if (typeof type === "number") {
                            return target.getTag(key)?.getType() === type;
                        } else {
                            return target.getTag(key) !== undefined;
                        }
                    };
                },
                size(target) {
                    return () => {
                        switch (target.getType()) {
                            case NBT.Byte:
                            case NBT.Short:
                            case NBT.Int:
                            case NBT.Long:
                            case NBT.Float:
                            case NBT.Double:
                            case NBT.String:
                                return 1;
                            case NBT.List:
                                return target.getSize();
                            case NBT.Compound:
                                return target.getKeys().length;
                            case NBT.ByteArray:
                                return target.get().byteLength;
                            default:
                                throw new Error(`Unsupported type ${target.getType()}`);
                        }
                    };
                },
                [Symbol.iterator](target) {
                    return function* () {
                        const proxy = module.exports.NbtProxy.FunctionMap;
                        switch (target.getType()) {
                            case NBT.List: {
                                for (let index = 0; index < target.getSize(); index++) {
                                    yield [index, proxy.get(target)(index)];
                                }
                                break;
                            }
                            case NBT.Compound: {
                                for (let key of target.getKeys()) {
                                    yield [key, proxy.get(target)(key)];
                                }
                                break;
                            }
                            default: {
                                throw new Error(`Unsupported type ${target.getType()}`);
                            }
                        }
                    };
                }
            },
            get(target, key) {
                if (key in this.FunctionMap) return this.FunctionMap[key](target);
                if (target[key]) return target[key].bind(target);
                return this.FunctionMap.get(target)(key);
            },
            set(target, key, value) {
                return this.FunctionMap.set(target)(key, value);
            },
            deleteProperty(target, key) {
                return this.FunctionMap.remove(target)(key);
            },
            has(target, key) {
                return FunctionMap.contains(target)(key);
            },
            ownKeys(target) {
                switch (target.getType()) {
                    case NBT.List: return Array.from({ length: target.getSize() }, (_, i) => i);
                    case NBT.Compound: return target.getKeys();
                    default: throw new Error(`Unsupported type ${target.getType()}`);
                }
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
            if (module.exports.isNbt(data)) return data;
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
                    const list = new NbtList();
                    for (const item of data) {
                        list.addTag(module.exports.toNbt(item));
                    }
                    return list;
                } else {
                    const compound = new NbtCompound();
                    for (const key in data) {
                        compound.setTag(key, module.exports.toNbt(data[key]));
                    }
                    return compound;
                }
            } else {
                throw new Error(`Unsupported type ${typeof data}`);
            }
        },
        fromNbt(data) {
            if (!module.exports.isNbt(data)) return data;
            if (data instanceof NbtCompound) {
                const obj = {};
                for (const key of data.getKeys()) {
                    obj[key] = module.exports.fromNbt(data.getData(key));
                }
                return obj;
            } else if (data instanceof NbtList) {
                const arr = [];
                for (let index = 0; index < data.getSize(); index++) {
                    arr.push(module.exports.fromNbt(data.getTag(index)));
                }
                return arr;
            } else {
                return data?.get?.() ?? data;
            }
        },
        toSnbt(data, snbtFormat = module.exports.SnbtFormat.PrettyFilePrint, indent = 4) {
            if (!module.exports.isNbt(data)) throw new Error(`Unsupported type ${typeof data}`);
            return module.exports.callExportedFunction(
                "toSnbt",
                new NbtCompound({
                    "value": data
                }),
                snbtFormat,
                indent
            );
        },
        fromSnbt(snbt) {
            if (typeof snbt !== "string") throw new Error(`Unsupported type ${typeof snbt}`);
            return module.exports.callExportedFunction(
                "fromSnbt",
                snbt
            ).getTag("value");
        },
        callExportedFunction(functionName, ...args) {
            const result = (module.exports[`${functionName}#Origin`] ??= ll.imports(module.exports.NamespaceName, functionName))(...args);
            if (!result.success) throw new Error(`Failed to execute function ${module.exports.NamespaceName}.${functionName} in ${module.exports.getPluginName()}\n` + result.error);
            return result["value"];
        },
        getPluginName() {
            if (ll.getCurrentPluginInfo) return ll.getCurrentPluginInfo().name;

            const error = new Error("getPluginName");

            if (error.stack) {
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

            if (error.stack) {
                const /** @type {string} */ line = error.stack.trim().split("\n").pop().trim();
                if (line.includes(".js") && /:\d+:\d+$/.test(line)) {
                    const directories = line.split("\\");
                    const pluginName = directories[directories.findIndex(value => value === "plugins") + 1].trim();
                    if (pluginName) return pluginName;
                }
            }
            return "Unknown";
        },
        registerEvent(eventName, pluginName = module.exports.getPluginName()) {
            return module.exports.callExportedFunction("registerEvent", pluginName, eventName);
        },
        RegisterEvent(...args) {
            logger.warn(`function iListenAttentively.RegisterEvent is deprecated, use iListenAttentively.registerEvent instead`);
            return module.exports.registerEvent(...args);
        },
        emplaceListener(eventName, callback, priority = module.exports.EventPriority.Normal, pluginName = module.exports.getPluginName()) {
            if (!module.exports.hasEvent(eventName) && module.exports.getEventName(eventName)) eventName = module.exports.getEventName(eventName);
            const listenerId = module.exports.callExportedFunction("emplaceListener", pluginName, eventName, priority);
            ll.exports(event => {
                try {
                    callback(new Proxy(event, module.exports.NbtProxy));
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
        publish(...args) {
            if (args.length < 2) throw new Error("Invalid arguments");
            if (!module.exports.isNbt(args[1])){
                const eventData = module.exports.toNbt(args[1]);
                Object.keys(args[1]).forEach(key => delete args[1][key]);
                module.exports.callExportedFunction("publish", args[0], eventData, ...args.slice(2));
                Object.entries(module.exports.fromNbt(eventData)).forEach(([key, value]) => (args[1][key] = value));
            } else {
                module.exports.callExportedFunction("publish", ...args);
            }
        }
    },
    ...(() => {
        return [
            "getPlayer",
            "getActor",
            "getItemStack",
            "getBlock",
            "getBlockActor",
            "getContainer",
            "getCompoundTag",
        ].map(
            name => ({
                [name](data) {
                    try {
                        return module.exports.callExportedFunction(name, data?.getData?.("_pointer_") ?? data);
                    } catch {
                        return undefined;
                    }
                }
            })
        );
    })(),
    ...(() => {
        return [
            "getAllEventAlias",
            "getEventAlias",
            "getEventName",
            "getDimensionIdFromName",
            "getDimensionNameFromId",
            "removeListener",
            "hasListener",
            "getAllEvent",
            "hasEvent",
            "getListenerCount",

            "getPlayerAddress",
            "getActorAddress",
            "getItemStackAddress",
            "getBlockAddress",
            "getBlockActorAddress",
            "getContainerAddress",
            "getCompoundTagAddress",

            "getRawAddress",
            "getLongLong",
            "getUnsignedLongLong",
            "getInt",
            "getUnsignedInt",
            "getShort",
            "getUnsignedShort",
            "getChar",
            "getUnsignedChar",
            "getFloat",
            "getLongDouble",
            "getDouble",
            "getBool",
            "getString",

            "setRawAddress",
            "setLongLong",
            "setUnsignedLongLong",
            "setInt",
            "setUnsignedInt",
            "setShort",
            "setUnsignedShort",
            "setChar",
            "setUnsignedChar",
            "setFloat",
            "setLongDouble",
            "setDouble",
            "setBool",
            "setString",

            "getAddressFromSymbol",
            "getAddressFromSignature",
            "getImageRange",
            "dynamicCall",
            "mallocMemory",
            "freeMemory",
            "alignedMallocMemory",
            "alignedFreeMemory",
            "getUsableMemorySize",
            "memcpyMemory",
            "memsetMemory",
            "memcmpMemory",
            "memmoveMemory",
        ].map(
            name => ({
                [name](...args) {
                    return module.exports.callExportedFunction(name, ...args);
                }
            })
        );
    })()
);