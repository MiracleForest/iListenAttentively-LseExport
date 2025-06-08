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
        getPluginName() {
            if (ll.getCurrentPluginInfo) return ll.getCurrentPluginInfo().name;

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
        RegisterEvent(eventName, pluginName = this.getPluginName()) {
            const result = (this[`RegisterEvent#Origin`] ??= ll.imports(this.NamespaceName, "RegisterEvent"))(eventName, pluginName);
            if (!result.success) throw new Error(`Failed to execute function ${this.NamespaceName}.RegisterEvent in ${this.getPluginName()}\n` + result.error);
            return result["value"];
        },
        emplaceListener(eventName, callback, priority = this.EventPriority.Normal, pluginName = this.getPluginName()) {
            if (!this.hasEvent(eventName) && this.getEventName(eventName)) eventName = this.getEventName(eventName);
            // const listenerId = ll.imports(this.NamespaceName, "emplaceListener")(pluginName, eventName, priority);
            // if (!this.hasListener(listenerId)) throw new Error(`Failed to emplace listener for ${eventName} in ${pluginName}`);
            // ll.exports(event => {
            //     try {
            //         callback(new Proxy(event, this.nbtProxyHandler));
            //     } catch (error) {
            //         `Failed to execute event callback for ${eventName} in ${pluginName} plugin\nException: ${error.message}\nStack:\n${error.stack}`
            //             .trim()
            //             .split("\n")
            //             .forEach(line =>
            //                 logger.error(line)
            //             );
            //     }
            //     return event;
            // }, pluginName, eventName + "#" + listenerId.toString());
            // return listenerId;
        },
    },
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
            "publish",

            "getPlayer",
            "getActor",
            "getItemStack",
            "getBlock",
            "getBlockActor",
            "getContainer",
            "getCompoundTag",

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
            "setString"
        ].map(
            name => ({
                [name]: function(...args) {
                    const result = (this[`${name}#Origin`] ??= ll.imports(this.NamespaceName, name))(...args);
                    if (!result.success) throw new Error(`Failed to execute function ${this.NamespaceName}.${name} in ${this.getPluginName()}\n` + result.error);
                    return result["value"];
                }
            })
        );
    })()
);