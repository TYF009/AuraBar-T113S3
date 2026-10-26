# Security reporting

The current software is a development preview for a matching board/system. There is no long-term support branch or security-response SLA yet.

Use GitHub's **Security → Report a vulnerability** if private vulnerability reporting is enabled for this repository. If that entry is unavailable, open an issue requesting a private contact channel without exploit details, credentials, tokens or private project content. No unverified email address is advertised here.

Include affected version/commit, environment, impact and minimal reproduction through the private channel. Keep access tokens, raw task content and sensitive command output out of public reports.

The companion binds to loopback by default and authenticates task updates. It does not accept remote shell commands. USB mode keeps the pairing token off the board. Exposing the development service beyond loopback requires a deployment-specific authentication/TLS design.

Dependencies and media retain their own licenses and update requirements. See [notices](THIRD_PARTY_NOTICES.md) and [architecture](docs/architecture/README.md).
