# Contributing

Start with the [development environment](docs/development/README.md) and [current scope](docs/product/status.md). Small changes should state the user-visible behavior and why it is needed.

## Workflow

1. Branch from main and keep unrelated changes separate.
2. Run `python scripts/check_repository.py` and the tests relevant to your change. Native regressions need the Linux dependencies described in the development guide.
3. For device changes, record whether ARM compilation and physical acceptance were actually performed.
4. Update user-facing setup, compatibility and known issues when behavior changes.
5. Open a PR with a concrete before/after description, validation and limitations.

Do not commit private tokens, credentials, user media, local SDK binaries, build outputs or workstation-specific build helpers. Keep third-party notices and source provenance intact. Avoid combining vendor upgrades, directory migration and behavior changes in one PR.

## Reports

Use [issue templates](https://github.com/TYF009/AuraBar-T113S3/issues/new/choose). Include the version/commit, board/system, reproduction and relevant redacted logs. AI concepts and hardware feature presence are not evidence of completed software integration.

Sensitive issues follow [security reporting](SECURITY.md). Maintenance is currently a project-maintainer workflow; no commercial response SLA is promised.
