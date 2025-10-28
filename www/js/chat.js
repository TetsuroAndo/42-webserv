(() => {
  const chatWindow = document.getElementById('chat-window');
  const form = document.getElementById('chat-form');
  const input = document.getElementById('message-input');

  let isFetching = false;
  let timerId = null;
  let lastRenderKey = '';

  function render(messages) {
    const html = messages.map(m => (
      `<div class="chat-message">` +
        `<span class="meta">[${m.ts}]</span> ` +
        `<span class="user">${escapeHtml(m.user)}:</span>` +
        `<span class="msg">${escapeHtml(m.msg)}</span>` +
      `</div>`
    )).join('');
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

  async function fetchMessages() {
    if (isFetching) return;
    isFetching = true;
    try {
      const res = await fetch('/cgi-bin/messages.py?limit=100', { cache: 'no-store' });
      if (!res.ok) throw new Error('fetch failed');
      const data = await res.json();
      render(data.messages || []);
    } catch (e) {
      // noop
    } finally {
      isFetching = false;
    }
  }

  async function sendMessage(text) {
    const body = new URLSearchParams();
    body.set('message', text);
    const res = await fetch('/cgi-bin/send_message.py', {
      method: 'POST',
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
      body: body.toString(),
    });
    if (!res.ok) throw new Error('send failed');
  }

  form.addEventListener('submit', async (e) => {
    e.preventDefault();
    const text = (input.value || '').trim();
    if (!text) return;
    input.value = '';
    try {
      await sendMessage(text);
      await fetchMessages();
    } catch (e) {
      // noop
    }
  });

  // 初回ロード + ポーリング
  fetchMessages();
  timerId = setInterval(fetchMessages, 2000);
})();
