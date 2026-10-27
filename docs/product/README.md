# Product scope

AuraBar is a compact Linux display platform with two application directions: an AI workspace companion and embedded interfaces for connected products.

The current product loop is specific: start a task on the computer, glance at its state on AuraBar, read the result card, then review or act on the computer. Music and ambient sound support everyday use between tasks.

## Product decisions

- Put concise state and next actions on the small screen; keep code, detailed logs and approvals on the computer.
- Keep the device focused on UI/audio and bounded transport. The 128 MB board does not host a bundled language model.
- Start with USB10 because it is the physically verified transport and does not require first-use Wi-Fi setup.
- Share display, storage, audio and task infrastructure across successive software previews.
- Publish actual implementation and evidence separately from future industrial-design scenes.

## Application directions

| Direction | User need | Current scope |
|---|---|---|
| AI workspace | Know what the agent is doing and what needs review | Companion-launched Codex events and cards |
| Music/focus | A persistent audio and information companion | Local audio, synthesized textures and controls |
| Connected appliances/furniture | A compact reusable interaction surface | Hardware/UI foundation; device-specific adapters planned |
| Auxiliary vehicle display | Information integrated into interior trim | Product concept; vehicle data and qualification not implemented |

Start with the [product film](demo.md), [integration brief](integration-brief.md) and [reproduction guide](../guides/reproduce.md).

The [product delivery review](repository-review.md) assesses the current repository and prioritizes the remaining release and deployment work.

Read the [capability matrix](status.md), [roadmap](roadmap.md) and [architecture](../architecture/README.md).
