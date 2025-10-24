<?php
// CgiEnvBuilder から渡される環境変数を取得
$server_name = $_SERVER['SERVER_NAME'] ?? 'webserv/42'; 

// app.js の Date.parse() が解釈できるISO 8601形式で現在時刻を生成
$last_access_time = date('c'); 

/*
 * 1. HTTPヘッダーの送信 (CGIのルール)
 * --------------------------------------
 * Content-TypeヘッダーとSet-Cookieヘッダーを送信します。
 * PHPの header() 関数は、CGIの仕様に従いヘッダー行を出力します。
 */
header("Content-Type: text/html; charset=utf-8");

// app.js が読み取る 'serverName' Cookie をセット
// rawurlencodeはJSのdecodeURIComponent()と互換性があります
header("Set-Cookie: serverName=" . rawurlencode($server_name) . "; Path=/");

// app.js が読み取る 'lastAccessTime' Cookie をセット
header("Set-Cookie: lastAccessTime=" . rawurlencode($last_access_time) . "; Path=/");

/*
 * 2. スタイルシート (CSS) の定義
 * --------------------------------------
 * tmp/www/styles.css の内容をインライン化します。
 */
$styles = <<<CSS
:root {
  --bg: #f9fafb;
  --panel: #ffffff;
  --text: #111827;
  --muted: #6b7280;
  --border: #e5e7eb;
  --accent: #2563eb;
}
@media (prefers-color-scheme: dark) {
  :root {
    --bg: #0b1220;
    --panel: #0f172a;
    --text: #e5e7eb;
    --muted: #94a3b8;
    --border: #1f2937;
    --accent: #60a5fa;
  }
}
html, body {
  height: 100%;
}
body {
  margin: 0;
  padding: 24px;
  background: var(--bg);
  color: var(--text);
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  font-family: ui-sans-serif, system-ui, -apple-system, Segoe UI, Roboto, Helvetica, Arial, "Apple Color Emoji", "Segoe UI Emoji";
  font-size: 16px;
  line-height: 1.7;
  letter-spacing: 0.02cm;
}
body > * {
  max-width: 760px;
  margin-left: auto;
  margin-right: auto;
}
h1, h2, h3 { line-height: 1.25; margin: 0.8em 0 0.4em; }
p { margin: 0.6em 0; }
a { color: var(--accent); text-decoration: none; }
a:hover { text-decoration: underline; }
.card {
  background: var(--panel);
  border: 1px solid var(--border);
  border-radius: 10px;
  padding: 12px 14px;
  box-shadow: 0 1px 2px rgba(0,0,0,0.04);
  margin-top: 12px;
}
.card .title {
  font-weight: 600;
  margin-bottom: 4px;
}
.card .value {
  color: var(--text);
}
.card .hint, #last-access span { color: var(--muted); }
* { transition: background-color .2s ease, color .2s ease, border-color .2s ease; }
CSS;

/*
 * 3. JavaScript (app.js) の定義
 * --------------------------------------
 * tmp/www/app.js の内容をインライン化します。
 * このJSは、上記でセットされたCookieをブラウザ側で読み取ります。
 */
$javascript = <<<JS
function getCookie(name) {
    if (!document.cookie) return null;
    const pairs = document.cookie.split(';');
    for (let p of pairs) {
      const idx = p.indexOf('=');
      const key = (idx >= 0 ? p.slice(0, idx) : p).trim();
      // PHPの rawurlencode() に合わせて decodeURIComponent() を使用
      const val = idx >= 0 ? decodeURIComponent(p.slice(idx + 1).trim()) : '';
      if (key === name) return val;
    }
    return null;
  }

(() => {
  function parseToMs(ts) {
    if (ts == null) return null;
    const s = String(ts).trim();
    if (/^\d+$/.test(s)) {
      const n = parseInt(s, 10);
      if (s.length >= 13) return n;
      if (s.length === 10) return n * 1000;
      return n;
    }
    const mLocal = s.match(/^(\d{4})-(\d{2})-(\d{2})[ T](\d{2}):(\d{2})(?::(\d{2}))?(\.\d+)?([Z+-].*)?$/);
    if (mLocal) {
        // PHPの date('c') 形式 (ISO 8601) をパースするために正規表現を少し調整
        const t = Date.parse(s);
        if (!isNaN(t)) return t;
    }
    const t = Date.parse(s);
    if (!isNaN(t)) return t;
    return null;
  }
  function formatTimestamp(ms) {
    if (ms == null) return null;
    const d = new Date(ms);
    if (isNaN(d.getTime())) return null;
    return d.toLocaleString();
  }
  function formatDuration(ms) {
    if (ms < 0) ms = 0;
    const sec = Math.floor(ms / 1000);
    const s = sec % 60;
    const m = Math.floor(sec / 60) % 60;
    const h = Math.floor(sec / 3600) % 24;
    const d = Math.floor(sec / 86400);
    const parts = [];
    if (d) parts.push(d + 'd');
    if (h || d) parts.push(h.toString().padStart(2, '0') + 'h');
    parts.push(m.toString().padStart(2, '0') + 'm');
    parts.push(s.toString().padStart(2, '0') + 's');
    return parts.join(' ');
  }
  function animateCount({ fromMs, toMs, durationMs, onUpdate, onComplete }) {
    const start = performance.now();
    const delta = toMs - fromMs;
    function easeOutCubic(t) { return 1 - Math.pow(1 - t, 3); }
    function frame(now) {
      const t = Math.min(1, (now - start) / durationMs);
      const eased = easeOutCubic(t);
      const current = fromMs + delta * eased;
      onUpdate(current);
      if (t < 1) requestAnimationFrame(frame); else onComplete && onComplete();
    }
    requestAnimationFrame(frame);
  }
  const el = document.getElementById('last-access');
  if (!el) return;
  const lastStr = getCookie('lastAccessTime');
  if (!lastStr) {
    el.textContent = 'lastAccessTime: not set (refresh page)';
    return;
  }
  const lastMs = parseToMs(lastStr);
  const formatted = formatTimestamp(lastMs);
  const base = document.createElement('div');
  base.textContent = formatted
    ? `lastAccessTime: ${lastStr} ( ${formatted} )`
    : `lastAccessTime: ${lastStr}`;
  el.textContent = '';
  el.appendChild(base);
  const diffRow = document.createElement('div');
  diffRow.style.marginTop = '0.25em';
  const label = document.createElement('span');
  label.textContent = 'since update: ';
  const value = document.createElement('span');
  value.id = 'last-access-diff';
  value.style.fontWeight = 'bold';
  diffRow.appendChild(label);
  diffRow.appendChild(value);
  el.appendChild(diffRow);
  if (lastMs == null) {
    value.textContent = '(invalid timestamp)';
    return;
  }
  const nowMs = Date.now();
  const target = Math.max(0, nowMs - lastMs);
  animateCount({
    fromMs: 0,
    toMs: target,
    durationMs: Math.min(1200, Math.max(400, target / 10)),
    onUpdate: (ms) => {
      value.textContent = formatDuration(ms);
    },
    onComplete: () => {
      let startBase = Date.now();
      function tick() {
        const live = Date.now() - lastMs;
        value.textContent = formatDuration(live);
      }
      let lastSec = -1;
      function loop() {
        const sec = Math.floor((Date.now() - startBase) / 1000);
        if (sec !== lastSec) {
          lastSec = sec;
          tick();
        }
        requestAnimationFrame(loop);
      }
      tick();
      requestAnimationFrame(loop);
    }
  });
})();

(() => {
  const nameEl = document.getElementById('server-name');
  if (!nameEl) return;
  const name = getCookie('serverName');
  nameEl.textContent = name || 'not set (refresh page)';
})();
JS;

/*
 * 4. HTTPボディ (HTML) の送信
 * --------------------------------------
 * tmp/www/index.html の構造をベースに、
 * $styles と $javascript 変数をインライン展開して出力します。
 */
echo <<<HTML
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Webserv CGI Demo</title>
    <style>
    {$styles}
    </style>
</head>
<body>

<div class="card">
  <div class="title">CGI Demo Page</div>
  <div class="hint">This page is rendered by a PHP-CGI script.</div>
</div>

<div id="server-info" class="card">
  <div class="title">Server (from CGI Environment)</div>
  <div id="server-name" class="value">Loading...</div>
</div>

<div id="last-access" class="card">Loading lastAccessTime...</div>

<script>
{$javascript}
</script>

</body>
</html>
HTML;
?>
