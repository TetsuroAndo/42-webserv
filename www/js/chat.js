(() => {
  const chatWindow = document.getElementById('chat-window');
  const form = document.getElementById('chat-form');
  const input = document.getElementById('message-input');
  const fileInput = document.getElementById('file-input');
  const uploadBtn = document.getElementById('upload-btn');
  let currentUser = null;

  let isFetching = false;
  let timerId = null;
  let lastRenderKey = '';

  function render(messages) {
    let lastDate = '';
    const parts = [];
    messages.forEach(m => {
      const ts = String(m.ts || '');
      const date = ts.slice(0, 10);
      if (date && date !== lastDate) {
        lastDate = date;
        parts.push(
          `<div class="my-3 flex items-center gap-3">` +
            `<div class="h-px bg-gray-200 dark:bg-gray-800 flex-1"></div>` +
            `<div class="text-xs text-gray-500 dark:text-gray-400 whitespace-nowrap">${date}</div>` +
            `<div class="h-px bg-gray-200 dark:bg-gray-800 flex-1"></div>` +
          `</div>`
        );
      }

      const isMine = currentUser && m.user === currentUser;
      const alignClass = isMine ? 'justify-end' : 'justify-start';
      const bubbleClass = isMine
        ? 'bg-blue-600 text-white rounded-2xl rounded-br-sm'
        : 'bg-gray-200 dark:bg-gray-800 text-gray-900 dark:text-gray-100 rounded-2xl rounded-bl-sm';
      const nameHtml = isMine ? '' : `<div class="text-xs font-semibold text-gray-600 dark:text-gray-300 mb-1">${escapeHtml(m.user)}</div>`;
      const fileHtml = (m.file && m.file.url)
        ? renderAttachment(m.file)
        : '';
      parts.push(
        `<div class="chat-row flex ${alignClass} my-1">` +
          `<div class="max-w-[80%]">` +
            `${nameHtml}` +
            `<div class="chat-bubble ${bubbleClass} px-3 py-2 shadow-sm">` +
              `<div class="text-sm leading-relaxed break-words">${escapeHtml(m.msg || '')}</div>` +
              `${fileHtml}` +
              `<div class="text-[10px] opacity-70 text-right mt-1 select-none">${ts}</div>` +
            `</div>` +
          `</div>` +
        `</div>`
      );
    });
    const html = parts.join('');
    const newKey = messages.length ? `${messages[0].ts}-${messages[messages.length - 1].ts}-${messages.length}` : 'empty';
    if (newKey !== lastRenderKey) {
      chatWindow.innerHTML = html || '<p>まだメッセージはありません。</p>';
      chatWindow.scrollTop = chatWindow.scrollHeight;
      lastRenderKey = newKey;
    }
  }

  function escapeHtml(str) {
    return String(str)
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;')
      .replace(/"/g, '&quot;')
      .replace(/'/g, '&#39;');
  }

  function renderAttachment(file) {
    const name = escapeHtml(file.name || 'attachment');
    const url = String(file.url || '');
    const size = file.size || 0;
    const mime = String(file.mime || '');
    const looksImage = /^image\//.test(mime) || /\.(png|jpe?g|gif|webp|svg)$/i.test(String(file.name || ''));

    if (looksImage && url) {
      return (
        `<div class="mt-2">` +
          `<a href="${url}" target="_blank" rel="noopener">` +
            `<img src="${url}" alt="${name}" class="max-w-[240px] max-h-[240px] rounded-md border border-gray-200 dark:border-gray-800" />` +
          `</a>` +
          `<div class="text-xs text-gray-500 mt-1">${name} (${size}B)</div>` +
        `</div>`
      );
    }

    return `<div class="mt-1 text-sm"><a class="underline text-blue-600 dark:text-blue-400 break-all" href="${url}" target="_blank" rel="noopener">${name}</a> <span class="text-xs text-gray-500">(${size}B)</span></div>`;
  }

  async function fetchMessages() {
    if (isFetching) return;
    isFetching = true;
    try {
      const res = await fetch('/api/messages.py?limit=100', { cache: 'no-store' });
      if (!res.ok) throw new Error('fetch failed');
      const data = await res.json();
      render(data.messages || []);
    } catch (e) {
      // noop
    } finally {
      isFetching = false;
    }
  }

  async function fetchWhoAmI() {
    try {
      const res = await fetch('/api/whoami.py', { cache: 'no-store' });
      if (!res.ok) return;
      const data = await res.json();
      currentUser = data.user || null;
    } catch (e) {
      // noop
    }
  }

  async function sendMessage(text) {
    const body = new URLSearchParams();
    body.set('message', text);
    const res = await fetch('/api/send_message.py', {
      method: 'POST',
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
      body: body.toString(),
    });
    if (!res.ok) throw new Error('send failed');
    const data = await res.json();
    if (!data.ok) throw new Error('send failed');
  }

  async function uploadFile(file, caption) {
    const fd = new FormData();
    if (caption) fd.set('message', caption);
    fd.set('file', file);
    const res = await fetch('/api/upload.py', { method: 'POST', body: fd });
    let payload = null;
    try { payload = await res.json(); } catch (e) {
      // JSONでない場合、テキストを拾っておく
      try {
        const txt = await res.text();
        throw new Error(`upload failed: non-json response ${res.status} ${String(txt).slice(0,200)}`);
      } catch (_) {
        throw new Error(`upload failed: HTTP ${res.status}`);
      }
    }
    if (!res.ok || !payload || payload.ok === false) {
      const msg = (payload && payload.error) ? String(payload.error) : `HTTP ${res.status}`;
      throw new Error(`upload failed: ${msg}`);
    }
  }

  form.addEventListener('submit', async (e) => {
    e.preventDefault();
    const file = fileInput && fileInput.files && fileInput.files[0];
    const text = (input.value || '').trim();
    if (file) {
      input.value = '';
      if (fileInput) fileInput.value = '';
      try {
        await uploadFile(file, text);
        await fetchMessages();
      } catch (e) {
        // noop
      }
      return;
    }
    if (!text) return;
    input.value = '';
    try {
      await sendMessage(text);
      await fetchMessages();
    } catch (e) {
      // noop
    }
  });

  if (uploadBtn && fileInput) {
    // クリックでファイル選択を開く
    uploadBtn.addEventListener('click', () => {
      fileInput.click();
    });

    // 選択後に自動アップロード
    fileInput.addEventListener('change', async () => {
      const file = fileInput.files && fileInput.files[0];
      if (!file) return;
      const caption = (input.value || '').trim();
      try {
        await uploadFile(file, caption);
        input.value = '';
        fileInput.value = '';
        await fetchMessages();
      } catch (e) {
        alert(`アップロードに失敗しました: ${e && e.message ? e.message : ''}`);
      }
    });
  }

  // 初回ロード + ポーリング
  fetchWhoAmI().then(fetchMessages);
  timerId = setInterval(fetchMessages, 2000);
})();
