#!/usr/bin/env python3
# ruff: noqa: TID251
import json
import os
import socket
import ssl
import urllib.error
import urllib.request
from copy import deepcopy
from dataclasses import dataclass
from pathlib import Path
from threading import Lock
from time import monotonic


class HostedAppsError(Exception):
    pass


class HostedAppsToolError(HostedAppsError):
    def __init__(self, message: str, result: dict[str, object]) -> None:
        super().__init__(message)
        self.result = result


_MAX_TOOL_LIST_PAGES = 100


def _url_error_category(reason: object) -> str:
    if isinstance(reason, TimeoutError):
        return "timeout"
    if isinstance(reason, socket.gaierror):
        return "DNS"
    if isinstance(reason, ssl.SSLError):
        return "TLS"
    if isinstance(reason, ConnectionError):
        return "connection"
    return "network"


def _require_object(value: object, label: str) -> dict[str, object]:
    if not isinstance(value, dict):
        raise HostedAppsError(f"{label} must be a JSON object")
    if not all(isinstance(key, str) for key in value):
        raise HostedAppsError(f"{label} keys must be strings")
    return value


def _require_string(value: object, label: str) -> str:
    if not isinstance(value, str) or not value:
        raise HostedAppsError(f"{label} must be a non-empty string")
    return value


@dataclass(frozen=True)
class HostedAppTool:
    name: str
    meta: dict[str, object]


class HostedAppsClient:
    """Call authenticated hosted-app tools from the active Codex session."""

    def __init__(self) -> None:
        self.endpoint = os.environ.get(
            "CODEX_APPS_MCP_URL",
            "https://chatgpt.com/backend-api/ps/mcp",
        )
        for origin in ("https://chatgpt.com", "https://chatgpt-staging.com"):
            if self.endpoint in (
                f"{origin}/backend-api/wham/apps",
                f"{origin}/backend-api/codex/apps",
            ):
                # The legacy catalog ignores the connector selection required by egress.
                self.endpoint = f"{origin}/backend-api/ps/mcp"
                break
        self._request_id = 0
        self._request_id_lock = Lock()
        self._tools: list[HostedAppTool] = []
        self._tool_lock = Lock()

    def fork_for_tool(self, connector_id: str, tool_name: str) -> "HostedAppsClient":
        client = HostedAppsClient()
        client.endpoint = self.endpoint
        if (
            connector_id == "connector_openai_library"
            and tool_name == "report_materialize_completion"
        ):
            for origin in ("https://chatgpt.com", "https://chatgpt-staging.com"):
                if self.endpoint in (
                    f"{origin}/backend-api/wham/apps",
                    f"{origin}/backend-api/codex/apps",
                    f"{origin}/backend-api/ps/mcp",
                ):
                    # This app-only callback is absent from public tool discovery.
                    # Its explicit binding also avoids discovery within the reporting budget.
                    client.endpoint = f"{origin}/backend-api/ps/mcp"
                    client._tools = [
                        HostedAppTool(
                            "library.report_materialize_completion",
                            {
                                "connector_id": connector_id,
                                "_codex_apps": {
                                    "resource_uri": (
                                        "/connector_openai_library/"
                                        "implicit_link::connector_openai_library/"
                                        "report_materialize_completion"
                                    ),
                                    "contains_mcp_source": False,
                                },
                            },
                        )
                    ]
                    return client
        if not self._tool_lock.acquire(blocking=False):
            return client
        try:
            tool = self._find_tool(connector_id, tool_name)
            if tool is not None:
                client._tools = [HostedAppTool(tool.name, deepcopy(tool.meta))]
        finally:
            self._tool_lock.release()
        return client

    def call_tool(
        self,
        connector_id: str,
        tool_name: str,
        arguments: dict[str, object],
        *,
        timeout_seconds: float | None = None,
    ) -> dict[str, object]:
        deadline = None if timeout_seconds is None else monotonic() + timeout_seconds
        tool = self._tool(connector_id, tool_name, deadline=deadline)
        params: dict[str, object] = {
            "name": tool.name,
            "arguments": arguments,
        }
        request_meta = self._request_meta(tool.meta)
        if request_meta:
            params["_meta"] = request_meta
        result = self._rpc("tools/call", params, deadline=deadline)
        if result.get("isError") is True:
            raise HostedAppsToolError(f"{tool_name} failed", result)
        return result

    def _tool(self, connector_id: str, tool_name: str, *, deadline: float | None) -> HostedAppTool:
        if deadline is None:
            self._tool_lock.acquire()
        elif not self._tool_lock.acquire(timeout=self._request_timeout(deadline)):
            raise HostedAppsError("hosted apps request deadline expired")
        try:
            tool = self._find_tool(connector_id, tool_name)
            return tool or self._load_tool(connector_id, tool_name, deadline=deadline)
        finally:
            self._tool_lock.release()

    def _find_tool(
        self,
        connector_id: str,
        tool_name: str,
    ) -> HostedAppTool | None:
        return next(
            (tool for tool in self._tools if self._is_tool(tool, connector_id, tool_name)),
            None,
        )

    def _load_tool(
        self, connector_id: str, tool_name: str, *, deadline: float | None
    ) -> HostedAppTool:
        cursor: str | None = None
        seen_cursors: set[str] = set()
        for _ in range(_MAX_TOOL_LIST_PAGES):
            params: dict[str, object] = {
                "_meta": {
                    "x-codex-turn-metadata": {
                        "mcp_request_meta": {"selected_connector_ids": [connector_id]}
                    }
                }
            }
            if cursor is not None:
                params["cursor"] = cursor
            result = self._rpc("tools/list", params, deadline=deadline)
            tools = result.get("tools")
            if not isinstance(tools, list):
                raise HostedAppsError("hosted apps tools/list did not return a tools array")
            discovered: list[HostedAppTool] = []
            for index, tool_value in enumerate(tools):
                tool = _require_object(tool_value, f"tools[{index}]")
                name = tool.get("name")
                meta = tool.get("_meta")
                if not isinstance(name, str) or not name or not isinstance(meta, dict):
                    continue
                discovered.append(
                    HostedAppTool(
                        name=name,
                        meta=meta,
                    )
                )
            self._tools.extend(discovered)
            matched_tool = self._find_tool(connector_id, tool_name)
            if matched_tool is not None:
                return matched_tool
            next_cursor = result.get("nextCursor", result.get("next_cursor"))
            if not isinstance(next_cursor, str) or not next_cursor:
                break
            if next_cursor in seen_cursors:
                raise HostedAppsError("hosted apps tools/list cursor repeated")
            seen_cursors.add(next_cursor)
            cursor = next_cursor
        raise HostedAppsError(f"Library {tool_name} is not available")

    @staticmethod
    def _is_tool(
        tool: HostedAppTool,
        connector_id: str,
        tool_name: str,
    ) -> bool:
        discovered_connector_id = next(
            (
                tool.meta.get(key)
                for key in ("connector_id", "connectorId", "app_id", "appId")
                if tool.meta.get(key) is not None
            ),
            None,
        )
        return discovered_connector_id == connector_id and (
            tool.name == tool_name
            or tool.name.endswith(f"_{tool_name}")
            or tool.name.endswith(f".{tool_name}")
        )

    def _rpc(
        self,
        method: str,
        params: dict[str, object],
        *,
        deadline: float | None = None,
    ) -> dict[str, object]:
        with self._request_id_lock:
            self._request_id += 1
            request_id = self._request_id
        body = json.dumps(
            {
                "jsonrpc": "2.0",
                "id": request_id,
                "method": method,
                "params": params,
            },
            separators=(",", ":"),
        ).encode()
        request = urllib.request.Request(
            self.endpoint,
            data=body,
            headers={
                "Accept": "application/json, text/event-stream",
                "Content-Type": "application/json",
                **self._auth_headers(),
            },
            method="POST",
        )
        try:
            with urllib.request.urlopen(
                request,
                timeout=self._request_timeout(deadline),
                context=self._ssl_context(),
            ) as response:
                response_body = response.read().decode()
                content_type = response.headers.get("content-type", "")
        except urllib.error.HTTPError as exc:
            raise HostedAppsError(
                f"hosted apps {method} request failed with HTTP status {exc.code}"
            ) from exc
        except urllib.error.URLError as exc:
            raise HostedAppsError(
                f"hosted apps {method} request failed: {_url_error_category(exc.reason)}"
            ) from exc
        payload = self._decode_response(response_body, content_type)
        if payload.get("error") is not None:
            raise HostedAppsError(f"hosted apps {method} RPC failed")
        return _require_object(payload.get("result"), "hosted apps RPC result")

    @staticmethod
    def _request_timeout(deadline: float | None) -> float:
        if deadline is None:
            return 90
        remaining = deadline - monotonic()
        if remaining <= 0:
            raise HostedAppsError("hosted apps request deadline expired")
        return min(90, remaining)

    @staticmethod
    def _request_meta(tool_meta: dict[str, object]) -> dict[str, object]:
        request_meta = dict(tool_meta)
        thread_id = os.environ.get("CODEX_THREAD_ID", "").strip()
        if thread_id:
            request_meta["threadId"] = thread_id
        session_id = os.environ.get("CODEX_SESSION_ID", "").strip()
        if thread_id and session_id and session_id != thread_id:
            existing_turn_metadata = request_meta.get("x-codex-turn-metadata")
            turn_metadata = (
                dict(existing_turn_metadata) if isinstance(existing_turn_metadata, dict) else {}
            )
            turn_metadata["session_id"] = session_id
            turn_metadata["thread_id"] = thread_id
            # The gateway reads the outer IDs for request correlation. It only
            # forwards declared first-party action metadata from mcp_request_meta.
            existing_action_meta = turn_metadata.get("mcp_request_meta")
            action_meta = (
                dict(existing_action_meta) if isinstance(existing_action_meta, dict) else {}
            )
            action_meta["x-codex-turn-metadata"] = {
                "session_id": session_id,
                "thread_id": thread_id,
            }
            turn_metadata["mcp_request_meta"] = action_meta
            request_meta["x-codex-turn-metadata"] = turn_metadata
        return request_meta

    def _auth_headers(self) -> dict[str, str]:
        account_id = os.environ.get("CODEX_APPS_CHATGPT_ACCOUNT_ID", "").strip() or None
        authorization = os.environ.get("CODEX_APPS_AUTHORIZATION", "").strip()
        if authorization:
            return self._with_account_id(authorization, account_id)
        connector_token = os.environ.get("CODEX_CONNECTORS_TOKEN", "").strip()
        if connector_token:
            return self._with_account_id(f"Bearer {connector_token}", account_id)
        token, auth_account_id = self._auth_file_tokens()
        return self._with_account_id(
            f"Bearer {token}",
            account_id or auth_account_id,
        )

    @staticmethod
    def _with_account_id(
        authorization: str,
        account_id: str | None,
    ) -> dict[str, str]:
        headers = {"Authorization": authorization}
        if account_id is not None:
            headers["ChatGPT-Account-ID"] = account_id
        return headers

    @staticmethod
    def _auth_file_tokens() -> tuple[str, str | None]:
        configured_path = os.environ.get("CODEX_AUTH_PATH")
        if configured_path:
            auth_path = Path(configured_path).expanduser()
        elif os.environ.get("CODEX_HOME"):
            auth_path = Path(os.environ["CODEX_HOME"]).expanduser() / "auth.json"
        else:
            auth_path = Path.home() / ".codex" / "auth.json"
        try:
            payload: object = json.loads(auth_path.read_text(encoding="utf-8"))
            auth = _require_object(payload, "Codex auth")
            tokens = _require_object(auth.get("tokens"), "Codex auth tokens")
            token = _require_string(tokens.get("access_token"), "Codex access token")
            account_value = tokens.get("account_id")
            account_id = (
                _require_string(account_value, "Codex account id")
                if account_value is not None
                else None
            )
        except (OSError, json.JSONDecodeError, HostedAppsError) as exc:
            raise HostedAppsError(f"could not read Codex auth from {auth_path}") from exc
        return token, account_id

    @staticmethod
    def _ssl_context() -> ssl.SSLContext:
        for environment_name in (
            "SSL_CERT_FILE",
            "REQUESTS_CA_BUNDLE",
            "CURL_CA_BUNDLE",
        ):
            ca_bundle = os.environ.get(environment_name)
            if ca_bundle:
                return ssl.create_default_context(cafile=ca_bundle)
        default_ca_bundle = Path("/etc/ssl/certs/ca-certificates.crt")
        if default_ca_bundle.exists():
            return ssl.create_default_context(cafile=default_ca_bundle)
        return ssl.create_default_context()

    @classmethod
    def _decode_response(
        cls,
        body: str,
        content_type: str,
    ) -> dict[str, object]:
        if "text/event-stream" not in content_type.lower():
            try:
                value: object = json.loads(body)
            except json.JSONDecodeError as exc:
                raise HostedAppsError("hosted apps returned invalid JSON") from exc
            return _require_object(value, "hosted apps response")

        last_event: dict[str, object] | None = None
        data_lines: list[str] = []
        for line in (*body.splitlines(), ""):
            if not line:
                event = cls._decode_event(data_lines)
                if event is not None:
                    last_event = event
                data_lines = []
            elif line.startswith("data:"):
                data_lines.append(line.removeprefix("data:").lstrip())
        if last_event is None:
            raise HostedAppsError("hosted apps returned an empty event stream")
        return last_event

    @staticmethod
    def _decode_event(data_lines: list[str]) -> dict[str, object] | None:
        if not data_lines:
            return None
        candidate = "\n".join(data_lines)
        if candidate.strip() == "[DONE]":
            return None
        try:
            value: object = json.loads(candidate)
        except json.JSONDecodeError:
            return None
        return value if isinstance(value, dict) else None

