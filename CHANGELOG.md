# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added


### Fixed


### Changed



## [0.1.5] - 2024-04-03

### Added

- Added forEach, filter, map, find, some, every, reduce for NbtList @zimuya4153
- Added getEventName and getEventAlias function @zimuya4153

### Fixed

- Fixed iterator index for NbtList @zimuya4153
- Fixed d.ts type @zimuya4153
- Fixed getBoolean, getString, getRawAddress function @zimuya4153

### Changed

- Changed the event alias to dynamically generated @zimuya4153
- Optimized code @zimuya4153


## [0.1.4] - 2024-03-08

### Added

- Added listenerId overflow check @zimuya4153
- Added event alias @zimuya4153
- Added getAllEventAlias function @zimuya4153
- Added getDimensionIdFromName and getDimensionNameFromId function @zimuya4153
- Added more types of getNumber @zimuya4153

### Clanged

- Use macro to simplify function definition @zimuya4153

## [0.1.3] - 2024-02-27

### Added

- Added some get function @zimuya4153

### Change

- Optimized some get functions @zimuya4153

### Fixed

- Fixed the NbtList toSNBT format error @zimuya4153


## [0.1.2] - 2024-02-24

### Added

- Adapted to LeviLamina 1.1.0-rc.1 @zimuya4153
- Added some get function @zimuya4153
- Added toSNBT method to NBT @zimuya4153


## [0.1.1] - 2024-2-19

### Added

- Added default values to emplaceListener parameters @zimuya4153
- Added passive to manifest @zimuya4153

### Fixed

- Fixed proxy get method for iListenAttentively @zimuya4153
- Fixed NBT.List deleteProperty method to handle numeric keys correctly @zimuya4153
- Fixed getPlayer and getEntity to handle info.get() @zimuya4153
- Fixed serialize/deserialize for custom event @zimuya4153

### Change

- Deleted the type detection of the getPlayer and getEntity interfaces @zimuya4153
- Enhanced listening callback completion @zimuya4153


## [0.1.0] - 2024-2-18

### Added

- Initialize the module @zimuya4153

[Unreleased]: https://github.com/MiracleForest/iListenAttentively/compare/v0.1.2...HEAD
[0.1.5]: https://github.com/MiracleForest/iListenAttentively-LseExport/compare/v0.1.4...v0.1.5
[0.1.4]: https://github.com/MiracleForest/iListenAttentively-LseExport/compare/v0.1.3...v0.1.4
[0.1.3]: https://github.com/MiracleForest/iListenAttentively-LseExport/compare/v0.1.2...v0.1.3
[0.1.2]: https://github.com/MiracleForest/iListenAttentively-LseExport/compare/v0.1.1...v0.1.2
[0.1.1]: https://github.com/MiracleForest/iListenAttentively-LseExport/compare/v0.1.0...v0.1.1
[0.1.0]: https://github.com/MiracleForest/iListenAttentively-LseExport/releases/tag/v0.1.0