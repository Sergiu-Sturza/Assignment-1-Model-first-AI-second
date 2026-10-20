// The line protocol between the website and the device. Pure functions, no I/O.
//
// Device -> website (impl/src/message_formatter.cpp):
//   T,<celsius>,<OK|LOW|HIGH|ERROR>,<low>,<high>   one measurement; celsius empty on ERROR
//   TH,<low>,<high>                                the threshold now
//   ERR,INVALID                                    the last command was not accepted
// Website -> device (impl/src/command_parser.cpp):
//   SET,<low>,<high>                               change the threshold
//   GET                                            ask for the threshold

/** The range the device accepts, in °C (impl/src/threshold.cpp). */
export const LIMITS = Object.freeze({ min: -40, max: 125 });

export const STATUSES = Object.freeze(["OK", "LOW", "HIGH", "ERROR"]);

const DECIMAL = /^-?\d+(\.\d+)?$/;

function decimal(text) {
  return DECIMAL.test(text) ? Number(text) : NaN;
}

/**
 * One line from the device -> a message:
 *   {type: "reading", celsius (null on ERROR), status, low, high}
 *   {type: "threshold", low, high}
 *   {type: "rejected", reason}
 *   {type: "unknown", text}   anything else, e.g. boot noise on the serial line
 */
export function parseDeviceLine(line) {
  const text = String(line).trim();
  const fields = text.split(",");
  const unknown = { type: "unknown", text };

  if (fields[0] === "T" && fields.length === 5) {
    const status = fields[2];
    const low = decimal(fields[3]);
    const high = decimal(fields[4]);
    if (!STATUSES.includes(status) || !Number.isFinite(low) || !Number.isFinite(high)) return unknown;
    if (status === "ERROR") {
      return fields[1] === "" ? { type: "reading", celsius: null, status, low, high } : unknown;
    }
    const celsius = decimal(fields[1]);
    return Number.isFinite(celsius) ? { type: "reading", celsius, status, low, high } : unknown;
  }
  if (fields[0] === "TH" && fields.length === 3) {
    const low = decimal(fields[1]);
    const high = decimal(fields[2]);
    return Number.isFinite(low) && Number.isFinite(high) ? { type: "threshold", low, high } : unknown;
  }
  if (fields[0] === "ERR" && fields.length === 2 && fields[1] !== "") {
    return { type: "rejected", reason: fields[1] };
  }
  return unknown;
}

/**
 * Why the device would refuse this threshold, or null if it accepts it. The same rule as
 * Threshold::set on the device, so the form can say so before anything is sent.
 */
export function thresholdProblem(low, high) {
  if (!Number.isFinite(low) || !Number.isFinite(high)) return "Enter a number for both limits.";
  if (low < LIMITS.min || high > LIMITS.max) {
    return `The limits must lie between ${LIMITS.min} °C and ${LIMITS.max} °C.`;
  }
  if (!(low < high)) return "The low limit must be below the high limit.";
  return null;
}

/** The SET command for a threshold, one decimal per limit. Throws if the device would refuse it. */
export function formatSetThreshold(low, high) {
  const problem = thresholdProblem(low, high);
  if (problem) throw new RangeError(problem);
  return `SET,${low.toFixed(1)},${high.toFixed(1)}\n`;
}

export function formatGetThreshold() {
  return "GET\n";
}
