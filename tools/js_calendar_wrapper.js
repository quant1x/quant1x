// Wrapper to run tests/calendar_decode.js and emit clean JSON
// It overrides console.log to capture the final decoded value (the JS script prints debug logs and then console.log(dates)).

(function() {
  const path = require('path');
  const captured = [];
  const origLog = console.log;
  // override console.log to capture arguments but don't print them
  console.log = function() {
    captured.push(Array.from(arguments));
  };

  // execute the target script (it will call console.log which we capture)
  require(path.join(__dirname, '..', 'tests', 'calendar_decode.js'));

  // restore console.log for our output
  console.log = origLog;

  // find the last captured item whose first arg is an Array (the final dates list)
  let finalArg = null;
  for (let i = captured.length - 1; i >= 0; i--) {
    const args = captured[i];
    if (args && args.length > 0 && Array.isArray(args[0])) {
      finalArg = args[0];
      break;
    }
  }

  if (!finalArg) {
    // fallback: pick last captured entry and stringify it
    const last = captured[captured.length - 1] || [];
    const out = JSON.stringify(last, null, 2);
    process.stdout.write(out);
    process.exit(0);
  }

  // normalize elements: Dates -> {date: "YYYY-MM-DD"}, objects -> convert Date fields and keep others
  function normalizeElement(el) {
    if (el instanceof Date) {
      return { date: el.toISOString().slice(0, 10) };
    }
    if (el && typeof el === 'object') {
      const out = {};
      for (const k of Object.keys(el)) {
        const v = el[k];
        if (v instanceof Date) {
          out[k === 'day' ? 'date' : k] = v.toISOString().slice(0, 10);
        } else if (typeof v === 'number' || typeof v === 'string' || typeof v === 'boolean') {
          out[k === 'day' && typeof v === 'string' ? 'date' : k] = v;
        } else if (v && typeof v === 'object' && v.toString && v.toString().indexOf('Date') >= 0) {
          // generic fallback
          try {
            out[k] = (new Date(v)).toISOString();
          } catch (e) {
            out[k] = v;
          }
        } else {
          out[k] = v;
        }
      }
      // if the object only has a 'day' field that is Date or string, normalize to {date:...}
      if (Object.keys(out).length === 1 && out.day) {
        return { date: (out.day instanceof Date) ? out.day.toISOString().slice(0,10) : out.day };
      }
      // if it has 'day' key, rename to 'date'
      if (out.day) {
        out.date = out.day instanceof Date ? out.day.toISOString().slice(0,10) : out.day;
        delete out.day;
      }
      return out;
    }
    // fallback: return as-is
    return el;
  }

  const normalized = finalArg.map(normalizeElement);
  process.stdout.write(JSON.stringify(normalized, null, 2));
})();
