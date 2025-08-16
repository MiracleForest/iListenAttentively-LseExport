# iListenAttentively-LseExport

![iListenAttentively-LseExport](https://socialify.git.ci/MiracleForest/iListenAttentively-LseExport/image?description=1&descriptionEditable=A%20rich%20and%20modern%20LeviLamina%20Minecraft%20event%20library&font=Raleway&forks=1&issues=1&language=1&name=1&owner=1&pattern=Circuit%20Board&pulls=1&stargazers=1&theme=Auto)

![English](https://img.shields.io/badge/English-inactive?style=for-the-badge)
[![中文](https://img.shields.io/badge/简体中文-informational?style=for-the-badge)](README.zh.md)

## Install

## Usage

```javascript
/** @type {import("iListenAttentively")} */
const iListenAttentively = require("./iListenAttentively-LseExport/lib/iListenAttentively.js");

iListenAttentively.emplaceListener(
    "ll::event::PlayerJoinEvent",
    event => {
        event.self.toPlayer().tell("Hello, world!");
    }
);
```

## Contributors

This project exists thanks to all the people who contribute.

![Contributors](https://contrib.rocks/image?repo=MiracleForest/iListenAttentively-LseExport)

## License

Copyright © 2024 MiracleForest, All rights reserved.
