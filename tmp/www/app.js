(() => {
  function getCookie(name) {
    if (!document.cookie) return null;
    const pairs = document.cookie.split(';');
    for (let p of pairs) {
      const idx = p.indexOf('=');
      const key = (idx >= 0 ? p.slice(0, idx) : p).trim();
      const val = idx >= 0 ? p.slice(idx + 1).trim() : '';
      if (key === name) return decodeURIComponent(val);
    }
    return null;
  }

  function parseToMs(ts) {
    if (ts == null) return null;
    const s = String(ts).trim();

    // 1) Pure digits: epoch seconds or milliseconds
    if (/^\d+$/.test(s)) {
      const n = parseInt(s, 10);
      if (s.length >= 13) return n;          // assume ms
      if (s.length === 10) return n * 1000;  // assume seconds
      return n;                               // fallback assume ms
    }

    // 2) ISO8601 without timezone: e.g. 2025-10-13T22:23:21 or with space
    const mLocal = s.match(/^(\d{4})-(\d{2})-(\d{2})[ T](\d{2}):(\d{2})(?::(\d{2}))?$/);
    if (mLocal) {
      const [_, yy, mm, dd, hh, mi, ss] = mLocal;
      const d = new Date(
        Number(yy),
        Number(mm) - 1,
        Number(dd),
        Number(hh),
        Number(mi),
        ss ? Number(ss) : 0,
        0
      );
      const t = d.getTime();
      if (!isNaN(t)) return t; // treat as local time
    }

    // 3) General Date.parse for ISO with timezone (e.g., ...Z or ...+09:00)
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
    el.textContent = 'lastAccessTime: not set';
    return;
  }

  const lastMs = parseToMs(lastStr);
  const formatted = formatTimestamp(lastMs);

  // Base label
  const base = document.createElement('div');
  base.textContent = formatted
    ? `lastAccessTime: ${lastStr} ( ${formatted} )`
    : `lastAccessTime: ${lastStr}`;
  el.textContent = '';
  el.appendChild(base);

  // Diff row
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

  // Initial animated count up to current delta
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
      // After animation, keep ticking in real time
      let startBase = Date.now();
      function tick() {
        const live = Date.now() - lastMs;
        value.textContent = formatDuration(live);
      }
      // Use rAF for smoothness, but throttle updates roughly each second
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
