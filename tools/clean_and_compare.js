const fs = require('fs');

function loadAndClean(path) {
  const buf = fs.readFileSync(path);
  // detect BOMs
  if (buf.length >= 3 && buf[0] === 0xEF && buf[1] === 0xBB && buf[2] === 0xBF) {
    return buf.slice(3).toString('utf8');
  }
  if (buf.length >= 2 && buf[0] === 0xFF && buf[1] === 0xFE) {
    return buf.slice(2).toString('utf16le');
  }
  return buf.toString('utf8');
}

try {
  const jsRaw = loadAndClean('tests/js_calendar_output_clean.json');
  const goRaw = loadAndClean('tests/go_calendar_output.json');
  const js = JSON.parse(jsRaw);
  const go = JSON.parse(goRaw);
  console.log('len(js)=', js.length, 'len(go)=', go.length);
  const min = Math.min(js.length, go.length);
  let diffs = 0;
  const maxDiffs = 20;
  for (let i = 0; i < min; i++) {
    const a = JSON.stringify(js[i]);
    const b = JSON.stringify(go[i]);
    if (a !== b) {
      console.log('diff at', i);
      console.log('js=', a);
      console.log('go=', b);
      diffs++;
      if (diffs >= maxDiffs) break;
    }
  }
  if (diffs === 0) console.log('no diffs'); else console.log(diffs, 'diffs found (showing up to', maxDiffs + ')');
} catch (err) {
  console.error('error:', err && err.message ? err.message : err);
  process.exit(1);
}
