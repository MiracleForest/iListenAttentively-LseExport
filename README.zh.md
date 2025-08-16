# iListenAttentively

![iListenAttentively](https://socialify.git.ci/MiracleForest/iListenAttentively-LseExport/image?description=1&descriptionEditable=A%20rich%20and%20modern%20LeviLamina%20Minecraft%20event%20library&font=Raleway&forks=1&issues=1&language=1&name=1&owner=1&pattern=Circuit%20Board&pulls=1&stargazers=1&theme=Auto)

[![English](https://img.shields.io/badge/English-informational?style=for-the-badge)](README.md)
![中文](https://img.shields.io/badge/简体中文-inactive?style=for-the-badge)

## 安装

## 用法

```javascript
/** @type {import("iListenAttentively")} */
const iListenAttentively = require("./iListenAttentively-LseExport/lib/iListenAttentively.js");

iListenAttentively.emplaceListener(
    "ll::event::PlayerJoinEvent",
    event => {
        event.self.toPlayer().tell("Hello, world!");
    }
)
```

## 贡献者

这个项目的存在要感谢所有做出贡献的人

![Contributors](https://contrib.rocks/image?repo=MiracleForest/iListenAttentively-LseExport)

## 许可证

版权所有 © 2024 MiracleForest，保留所有权利。