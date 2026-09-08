import { afterEach, describe, expect, it, vi } from 'vitest';
import { formatLastConnection, isFeederOnline } from './feederStatus';

describe('feeder connectivity status', () => {
  afterEach(() => {
    vi.useRealTimers();
  });

  it('treats an ISO UTC timestamp as an absolute instant', () => {
    vi.useFakeTimers();
    vi.setSystemTime(new Date('2026-09-08T15:47:42.273Z'));

    expect(isFeederOnline('2026-09-08T15:46:42.273Z')).toBe(true);
  });

  it('marks the feeder offline after the 90 second threshold', () => {
    vi.useFakeTimers();
    vi.setSystemTime(new Date('2026-09-08T15:49:13.000Z'));

    expect(isFeederOnline('2026-09-08T15:46:42.273Z')).toBe(false);
  });

  it('returns false when there is no last connection', () => {
    expect(isFeederOnline(null)).toBe(false);
    expect(isFeederOnline(undefined)).toBe(false);
  });

  it('formats the elapsed time without converting the stored UTC instant', () => {
    vi.useFakeTimers();
    vi.setSystemTime(new Date('2026-09-08T15:47:42.273Z'));

    expect(formatLastConnection('2026-09-08T15:46:42.273Z')).toBe('hace 1 minuto');
  });
});
