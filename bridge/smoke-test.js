#!/usr/bin/env node
'use strict';

/**
 * Quick check that the bridge accepts USER lines and returns one REPLY/ERROR line.
 * Run on the HOST (same machine as the bridge), not inside DOSBox.
 *
 *   node smoke-test.js
 *   node smoke-test.js 127.0.0.1 8765
 */

const net = require('net');

const host = process.argv[2] || '127.0.0.1';
const port = parseInt(process.argv[3] || process.env.BRIDGE_PORT || '8765', 10);
const msg = process.argv[4] || 'hello from smoke-test';

console.log(`Connecting to ${host}:${port} ...`);

const sock = net.connect({ host, port }, () => {
  const line = `USER ${msg}\n`;
  console.log(`Sending: ${JSON.stringify(line)}`);
  sock.write(line, 'latin1');
});

let buf = '';
sock.setTimeout(180000);

sock.on('data', (chunk) => {
  buf += chunk.toString('latin1');
  console.log(`RX chunk ${chunk.length} bytes (buffer ${buf.length} chars)`);
  const nl = buf.indexOf('\n');
  if (nl >= 0) {
    const first = buf.slice(0, nl);
    buf = buf.slice(nl + 1);
    console.log('First line from server:', JSON.stringify(first));
    sock.end();
  }
});

sock.on('close', () => {
  if (buf.trim()) console.log('Trailing buffer:', JSON.stringify(buf));
  console.log('Done.');
  process.exit(0);
});

sock.on('error', (err) => {
  console.error('Socket error:', err.message);
  process.exit(1);
});

sock.on('timeout', () => {
  console.error('Timeout waiting for first line of response.');
  process.exit(2);
});
