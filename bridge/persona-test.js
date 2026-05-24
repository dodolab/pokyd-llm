#!/usr/bin/env node
'use strict';

/**
 * Verify POKYD.CFG parsing and persona prompt assembly (no OpenAI).
 *
 *   node persona-test.js
 *   node persona-test.js ../POKYD.CFG
 */

const fs = require('fs');
const path = require('path');

process.env.OPENAI_API_KEY = process.env.OPENAI_API_KEY || 'test-key-for-load-only';

const {
  parsePokydCfg,
  buildSessionSystemPrompt,
  MOOD_LABELS,
  CHARAKTER_LABELS,
} = require('./server.js');

const cfgPath = path.resolve(process.argv[2] || path.join(__dirname, '..', 'POKYD.CFG'));
const raw = fs.readFileSync(cfgPath, 'utf8');
const parsed = parsePokydCfg(raw);

console.log('Parsed POKYD.CFG:');
console.log('  userGender:', parsed.userGender);
console.log('  computerGender:', parsed.computerGender);
console.log('  moodLabel:', parsed.moodLabel, `(level ${parsed.moodLevel})`);
console.log('  characterLabel:', parsed.characterLabel, `(level ${parsed.characterLevel})`);
console.log('  machineMode:', parsed.machineMode);
console.log('  activeName:', parsed.computerNameActive);

const prompt = buildSessionSystemPrompt(parsed);
const head = prompt.slice(0, 220).replace(/\n/g, ' ');
console.log('\nSystem prompt preview:', head + '...');
console.log('\nMood labels:', MOOD_LABELS.join(', '));
console.log('Character labels:', CHARAKTER_LABELS.join(', '));

const stroj = parsePokydCfg(
  raw.replace(
    /Charakter pocitace[^\n]*\n\s*\d+/,
    'Charakter pocitace (0..stroj):\n0'
  )
);
if (!stroj.machineMode) {
  console.error('FAIL: expected machineMode for charakter stroj');
  process.exit(1);
}
const tPrompt = buildSessionSystemPrompt(stroj);
if (!tPrompt.includes('MACHINE') && !tPrompt.includes('Machine')) {
  console.error('FAIL: Machine prompt missing for stroj');
  process.exit(1);
}

console.log('\nOK: persona-test passed.');
process.exit(0);
