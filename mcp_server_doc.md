# MCP Servers (Model Context Protocol) in Claude Code

## What Are MCP Servers?

MCP is an **open standard** that lets Claude connect to external tools and data sources — databases, APIs, filesystems, GitHub, Slack, and more. Think of it as a plugin system: each MCP server advertises a set of tools, and Claude can call those tools during your conversation.

---

## How They Work

1. You **configure** an MCP server (tell Claude how to start/connect to it)
2. The server **starts** and advertises its tools
3. Claude **discovers** the tools automatically
4. You **grant permissions** for which tools Claude can actually use
5. Claude **calls the tools** as needed during conversations

---

## Transport Types

| Type | Use Case | Example |
|------|----------|---------|
| **stdio** | Local processes (most common) | `npx @modelcontextprotocol/server-github` |
| **http** | Remote cloud APIs | `https://api.example.com/mcp` |
| **sse** | Streaming/real-time connections | `https://api.example.com/mcp/sse` |

---

## Configuration

MCP servers are configured in these files (pick based on scope):

| File | Scope | Committed to git? |
|------|-------|--------------------|
| `~/.claude/settings.json` | User (all projects) | No |
| `.claude/settings.json` | Project (team-shared) | Yes |
| `.claude/settings.local.json` | Project (private) | No |
| `.mcp.json` | Project root | Yes |

### Example `.mcp.json`:

```json
{
  "mcpServers": {
    "github": {
      "command": "npx",
      "args": ["-y", "@modelcontextprotocol/server-github"],
      "env": {
        "GITHUB_TOKEN": "${GITHUB_TOKEN}"
      }
    },
    "postgres": {
      "command": "npx",
      "args": ["-y", "@modelcontextprotocol/server-postgres", "${DATABASE_URL}"]
    }
  }
}
```

### Example in `settings.json`:

```json
{
  "mcpServers": {
    "github": {
      "command": "npx",
      "args": ["-y", "@modelcontextprotocol/server-github"],
      "env": {
        "GITHUB_TOKEN": "${GITHUB_TOKEN}"
      }
    },
    "filesystem": {
      "command": "npx",
      "args": ["-y", "@modelcontextprotocol/server-filesystem", "/Users/me/projects"]
    },
    "remote-docs": {
      "type": "http",
      "url": "https://code.claude.com/docs/mcp"
    }
  }
}
```

---

## Permissions

Tools follow the naming pattern `mcp__<server>__<tool>`. You must explicitly allow them:

```json
{
  "permissions": {
    "allow": [
      "mcp__github__list_issues",
      "mcp__postgres__query",
      "mcp__filesystem__*"
    ]
  }
}
```

Wildcards (`*`) grant all tools from a server — use with caution.

---

## Common MCP Servers

- **GitHub** — issues, PRs, repos (`@modelcontextprotocol/server-github`)
- **Filesystem** — read/write files (`@modelcontextprotocol/server-filesystem`)
- **PostgreSQL** — database queries (`@modelcontextprotocol/server-postgres`)
- **Slack** — messages and channels (`@modelcontextprotocol/server-slack`)
- **Context7** — library documentation (fetch up-to-date docs for any library)

Full registry: https://github.com/modelcontextprotocol/servers

---

## Security Best Practices

1. **Never hardcode secrets** — use `${ENV_VAR}` syntax in config files
2. **Least privilege** — grant specific tools, not `mcp__*__*`
3. **User-scope for secrets** — put tokens in `~/.claude/settings.json`, not project files
4. **Verify sources** — only use servers from trusted repositories
5. **Credential isolation** — store credentials in environment variables

```json
// Good
"env": { "API_KEY": "${SECRET_API_KEY}" }

// Bad - never do this
"env": { "API_KEY": "sk_live_xxxxxxxxxxxx" }
```

---

## Managing MCP Servers

- Use `/config` in the REPL to view server status and settings
- Connection status appears in the system init message when Claude starts
- Failed connections show errors you can debug (missing env vars, bad URLs, etc.)

---

## Troubleshooting

| Issue | Cause | Solution |
|-------|-------|----------|
| "failed" connection status | Missing env vars or server not installed | Verify env variables are set, check server is installed |
| Tools not being called | Missing `allowedTools` permission | Add tools to `permissions.allow` array |
| Environment variable not expanding | Syntax error | Use `${VAR_NAME}` syntax, not `$VAR_NAME` |
| Connection timeout | Server takes too long to start | Use a lighter-weight server or pre-warm it |
| Network error for HTTP/SSE | Server unreachable | Check URL, firewall, and server status |
