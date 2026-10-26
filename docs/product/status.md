# Capability and readiness matrix

Updated 2026-10-26. “Implemented” means code exists; “automated” means an exercised test; “board confirmed” means a recorded physical check. These are separate claims. This is an application preview, not production certification.

| Capability | Implementation | Evidence / remaining work |
|---|---|---|
| Visible UI and Chinese text | Native LVGL | Actual framebuffer and owner confirmation |
| Local WAV/MP3 output | Implemented | Automated playback/control tests; real speaker tones confirmed |
| Brown noise and synthesized textures | Implemented | Ambient playback/pause confirmed; rain/ocean are synthesized, not recordings |
| Seek, repeat, favorites, sleep timer | Implemented | Automated coverage; not every physical control path has separate acceptance |
| Direct MP3 HTTP/HTTPS streams | Implemented | Local HTTP success/error tests; physical Wi-Fi/real HTTPS chain acceptance pending |
| Generic task state and elapsed time | Implemented | Real successful ARM build and nonzero-exit task on board |
| USB disconnect/reconnect | Implemented | Board screenshots/probe and host integration coverage |
| Codex events and result card | Implemented | Real model static review of supplied code; events/parser regressions |
| Summary/Evidence/Next and text scroll | Implemented | Owner confirmed tabs, scrolling and improved alpha.2 responsiveness |
| Notification/audio coordination | Implemented | Automated tests passed; final physical recovery listening check open |
| Entire Codex desktop chat synchronization | Not implemented | Only companion-launched tasks are tracked |
| Full project-file/command workflow on development PC | Not fully accepted | Codex sandbox-helper access issue blocked a file-reading attempt |
| Screen approvals or task launch/cancel | Not implemented | Actions and review remain on computer |
| Wi-Fi task transport | HTTP client implemented | Host integration tested; physical transport acceptance uses USB |
| Bluetooth audio / BLE Mesh | Hardware connections present | No accepted software feature |
| Weather, alarms, To-Do, Pomodoro | Existing interfaces retained | Do not treat every path as a fully accepted current product feature |
| Appliance or vehicle integration | Concept | No real fridge temperatures, vehicle bus, navigation or charging integration |
| Autonomous startup / durable job recovery | Not implemented | Preview startup is manual; companion history is in memory |
| Full flashable BSP image | Not supplied | Matching existing Tina system required |

## Interpreting AI results

An AI summary is not independent verification. Command exit 0 does not prove test coverage. A model-reported blocked outcome becomes a blocked/failed task; it is not a complete validator of whether a request was fulfilled. Read [protocol and evidence semantics](../architecture/task-protocol.md).

## Concepts and hardware

The polished renders show redesigned enclosures and UI. Example temperatures, charging progress and file counts are illustrative. Source schematics, actual board captures and release acceptance records remain the engineering references.

[Validation records](../testing/README.md) · [Known issues](../guides/troubleshooting.md) · [Release compatibility](../releases/README.md)
