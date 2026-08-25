const WINDOW_MS = 15 * 60 * 1000;
const MAX_ATTEMPTS = 10;

const attempts = new Map();

const cleanupExpiredEntries = (now) => {
  for (const [key, entry] of attempts.entries()) {
    if (entry.resetAt <= now) {
      attempts.delete(key);
    }
  }
};

const getClientKey = (req) => {
  const userId = req.user?._id?.toString() || "anonymous";
  const ip = req.ip || req.socket?.remoteAddress || "unknown";

  return `${userId}:${ip}`;
};

const pairingRateLimit = (req, res, next) => {
  const now = Date.now();
  cleanupExpiredEntries(now);

  const key = getClientKey(req);
  const current = attempts.get(key);

  if (!current || current.resetAt <= now) {
    attempts.set(key, {
      count: 1,
      resetAt: now + WINDOW_MS
    });
    return next();
  }

  if (current.count >= MAX_ATTEMPTS) {
    const retryAfter = Math.ceil((current.resetAt - now) / 1000);

    res.set("Retry-After", retryAfter.toString());

    return res.status(429).json({
      error: "Demasiados intentos de pairing. Intente nuevamente más tarde."
    });
  }

  current.count += 1;

  return next();
};

module.exports = pairingRateLimit;
