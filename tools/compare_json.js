const fs = require('fs');

function load(path) {
  let s = fs.readFileSync(path, 'utf8');
  if (s.charCodeAt(0) === 0xFEFF) s = s.slice(1);
  return JSON.parse(s);
}

const js = load('tests/js_calendar_output_clean.json');
const go = load('tests/go_calendar_output.json');
console.log('len(js)=', js.length, 'len(go)=', go.length);
const min = Math.min(js.length, go.length);
let diffs = 0;
for (let i = 0; i < min && i < 5000; i++) {
  const a = JSON.stringify(js[i]);
  const b = JSON.stringify(go[i]);
  if (a !== b) {
    console.log('diff at', i);
    console.log('js=', a);
    console.log('go=', b);
    diffs++;
    break;
  }
}
if (diffs === 0) console.log('no diffs in inspected range');
